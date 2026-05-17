#include <sstream>
#include "storage_engine.h"
#include "chunk_manager.h"
#include "metadata_manager.h"
#include "write_ahead_log.h"
#include "checksum.h"
#include "encryption.h"
#include "logger.h"
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <atomic>
#include <set>

static std::unordered_map<std::string, int> progressMap;
static std::mutex progressMutex;
static std::unordered_map<std::string, std::shared_ptr<std::mutex>> fileMutexes;
static std::mutex fileMutexesGuard;
static std::atomic<unsigned long long> transactionCounter{0};

namespace {
std::string createTransactionId() {
    const auto timestamp = static_cast<unsigned long long>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    const auto counter = transactionCounter.fetch_add(1);
    return std::to_string(timestamp) + "_" + std::to_string(counter);
}

std::string storageMetadataDir(const std::string& storageRoot) {
    return (std::filesystem::path(storageRoot) / "metadata").string();
}

std::string storageMetadataPath(const std::string& storageRoot, const std::string& fileId) {
    return (std::filesystem::path(storageMetadataDir(storageRoot)) / (fileId + ".meta")).string();
}

std::unique_lock<std::mutex> lockFileOperation(const std::string& storageRoot, const std::string& fileId) {
    const std::string key = storageRoot + "::" + fileId;
    std::shared_ptr<std::mutex> fileMutex;

    {
        std::lock_guard<std::mutex> guard(fileMutexesGuard);
        auto& slot = fileMutexes[key];
        if (!slot) {
            slot = std::make_shared<std::mutex>();
        }
        fileMutex = slot;
    }

    return std::unique_lock<std::mutex>(*fileMutex);
}

void collectGarbageChunks(const std::string& storageRoot) {
    MetadataManager metadataManager(storageRoot);
    ChunkManager chunkManager(storageRoot);

    const std::filesystem::path metadataDir = metadataManager.metadataDir();
    const std::filesystem::path chunksDir = chunkManager.chunksDir();

    if (!std::filesystem::exists(chunksDir)) {
        return;
    }

    std::unordered_set<std::string> liveChunkIds;

    if (std::filesystem::exists(metadataDir)) {
        for (const auto& entry : std::filesystem::directory_iterator(metadataDir)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".meta") {
                continue;
            }

            const std::string fileId = entry.path().stem().string();
            for (const auto& metadata : metadataManager.listMetadataVersions(fileId)) {
                for (const auto& chunk : metadata.chunks) {
                    liveChunkIds.insert(chunk.id);
                }
            }
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator(chunksDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const std::string chunkId = entry.path().filename().string();
        if (liveChunkIds.find(chunkId) != liveChunkIds.end()) {
            continue;
        }

        if (!chunkManager.deleteChunk(chunkId)) {
            LOG_ERROR("Failed to garbage-collect chunk: " + chunkId);
        }
    }
}
}

std::string formatTime(std::chrono::system_clock::time_point tp) {
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm *tm = std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// ======================= PROGRESS =======================
static void updateProgress(const std::string& fileId, int percent) {
    std::lock_guard<std::mutex> lock(progressMutex);
    progressMap[fileId] = percent;
}

// ======================= GET PROGRESS =======================
int StorageEngine::getProgress(const std::string& fileId) {
    std::lock_guard<std::mutex> lock(progressMutex);
    if (progressMap.find(fileId) == progressMap.end()) return 0;
    return progressMap[fileId];
}

StorageEngine::StorageEngine(std::string storageRoot)
    : storageRoot_(std::move(storageRoot)) {
    MetadataManager metadataManager(storageRoot_);
    ChunkManager chunkManager(storageRoot_);
    WriteAheadLog wal(storageRoot_);
    wal.recoverPending(metadataManager, chunkManager);
    metadataManager.cleanupTempFiles();
    collectGarbageChunks(storageRoot_);
}

std::string StorageEngine::metadataPath(const std::string& fileId) const {
    return storageMetadataPath(storageRoot_, fileId);
}

std::string StorageEngine::metadataDir() const {
    return storageMetadataDir(storageRoot_);
}

// ======================= WRITE =======================
bool StorageEngine::storeFile(const std::string &filePath, const std::string &fileId, ProgressCallback progressCallback){
    auto fileLock = lockFileOperation(storageRoot_, fileId);

    // ✅ START TIME
    auto startTime = std::chrono::system_clock::now();
    auto startHighRes = std::chrono::high_resolution_clock::now();

    ChunkManager chunkManager(storageRoot_);
    MetadataManager metadataManager(storageRoot_);
    WriteAheadLog wal(storageRoot_);

    std::ifstream in(filePath, std::ios::binary);

    if (!in.is_open()) {
        LOG_ERROR("Cannot open file: " + filePath);
        return false;
    }

    in.seekg(0, std::ios::end);
    size_t fileSize = in.tellg();
    in.seekg(0);

    if (fileSize == 0) {
        std::vector<std::string> oldChunkIds;
        for (const auto& chunk : metadataManager.loadChunks(fileId)) {
            oldChunkIds.push_back(chunk.id);
        }

        if (!wal.begin(fileId, 0, oldChunkIds)) {
            LOG_ERROR("Failed to create WAL for empty file: " + fileId);
            return false;
        }

        if (!wal.markApplying(fileId)) {
            LOG_ERROR("Failed to transition WAL to applying for empty file: " + fileId);
            wal.remove(fileId);
            return false;
        }

        std::vector<ChunkInfo> emptyChunks;

        if (!metadataManager.saveMetadata(fileId, emptyChunks, 0)) {
            LOG_ERROR("Failed to save metadata for empty file: " + fileId);
            return false;
        }

        if (!wal.markCommitted(fileId)) {
            LOG_ERROR("Failed to mark WAL committed for empty file: " + fileId);
            return false;
        }

        wal.remove(fileId);

        updateProgress(fileId, 100);
        if (progressCallback) progressCallback(100);

        // ✅ END TIME
        auto endTime = std::chrono::system_clock::now();
        auto endHighRes = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endHighRes - startHighRes);

        LOG_INFO("Stored empty file: " + fileId);
        LOG_INFO("Start Time: " + formatTime(startTime));
        LOG_INFO("End Time: " + formatTime(endTime));
        LOG_INFO("Time Taken: " + std::to_string(duration.count()) + " ms");

        return true;
    }

    const size_t CHUNK_SIZE = 4 * 1024 * 1024;
    const std::string transactionId = createTransactionId();

    size_t processed = 0;
    int chunkIndex = 0;
    std::vector<ChunkInfo> chunkInfos;
    std::vector<std::string> oldChunkIds;
    std::vector<std::string> attemptedChunkIds;

    for (const auto& chunk : metadataManager.loadChunks(fileId)) {
        oldChunkIds.push_back(chunk.id);
    }

    if (!wal.begin(fileId, fileSize, oldChunkIds)) {
        LOG_ERROR("Failed to create WAL for file: " + fileId);
        return false;
    }

    while (true) {
        std::vector<char> buffer(CHUNK_SIZE);

        in.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = in.gcount();

        if (bytesRead <= 0) break;

        buffer.resize(bytesRead);

        std::string chunkId = fileId + "_" + transactionId + "_chunk_" + std::to_string(chunkIndex++);

        std::string checksum = computeChecksum(buffer);

        ChunkInfo info;
        info.id = chunkId;
        info.checksum = checksum;

        if (!wal.appendChunk(fileId, info)) {
            for (const auto& attemptedChunkId : attemptedChunkIds) {
                chunkManager.deleteChunk(attemptedChunkId);
            }
            wal.remove(fileId);
            LOG_ERROR("Failed to append chunk to WAL: " + chunkId);
            return false;
        }

        attemptedChunkIds.push_back(chunkId);

        std::string key = "mysecretkey";
        std::vector<char> encryptedData = encryptData(buffer, key);

        if (!chunkManager.writeChunk(chunkId, encryptedData)) {
            for (const auto& attemptedChunkId : attemptedChunkIds) {
                chunkManager.deleteChunk(attemptedChunkId);
            }
            wal.remove(fileId);
            LOG_ERROR("Failed to write chunk: " + chunkId);
            return false;
        }

        chunkInfos.push_back(info);

        processed += bytesRead;
        int percent = static_cast<int>((processed * 100) / fileSize);

        updateProgress(fileId, percent);
        if (progressCallback) progressCallback(percent);
    }

    if (!wal.markApplying(fileId)) {
        LOG_ERROR("Failed to transition WAL to applying: " + fileId);
        return false;
    }

    if (!metadataManager.saveMetadata(fileId, chunkInfos, fileSize)) {
        LOG_ERROR("Failed to save metadata: " + fileId);
        return false;
    }

    if (!wal.markCommitted(fileId)) {
        LOG_ERROR("Failed to mark WAL committed: " + fileId);
        return false;
    }

    wal.remove(fileId);

    updateProgress(fileId, 100);

    // ✅ END TIME
    auto endTime = std::chrono::system_clock::now();
    auto endHighRes = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endHighRes - startHighRes);

    // ✅ LOGS
    LOG_INFO("Stored file - ID: " + fileId);
    LOG_INFO("Stored file - Path: " + filePath);
    LOG_INFO("Stored file - Size: " + std::to_string(fileSize) + " bytes");
    LOG_INFO("Stored file - Chunks: " + std::to_string(chunkInfos.size()));

    LOG_INFO("Start Time: " + formatTime(startTime));
    LOG_INFO("End Time: " + formatTime(endTime));
    LOG_INFO("Time Taken: " + std::to_string(duration.count()) + " ms");

    return true;
}
// ======================= READ =======================
bool StorageEngine::retrieveFile(const std::string &fileId,  const std::string &outputPath, ProgressCallback progressCallback)
{
    auto fileLock = lockFileOperation(storageRoot_, fileId);

    ChunkManager chunkManager(storageRoot_);
    MetadataManager metadataManager(storageRoot_);
    FileMetadata metadata;
    if (!metadataManager.loadMetadata(fileId, metadata)) {
        LOG_ERROR("File does not exist: " + fileId);
        return false;
    }

    const auto& chunks = metadata.chunks;

    // ===== EMPTY FILE =====
    if (chunks.empty()) {
        std::ofstream out(outputPath, std::ios::binary);
        if (!out.is_open()) {
            LOG_ERROR("Cannot open output file: " + outputPath);
            return false;
        }

        out.close();

        updateProgress(fileId, 100);

        if (progressCallback) {
            progressCallback(100);
        }

        LOG_INFO("Retrieved empty file: " + outputPath);
        return true;
    }

    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open()) {
        LOG_ERROR("Cannot open output file: " + outputPath);
        return false;
    }

    const auto failRetrieve = [&](const std::string& message) {
        LOG_ERROR(message);
        out.close();
        std::error_code removeError;
        std::filesystem::remove(outputPath, removeError);
        return false;
    };

    int totalChunks = chunks.size();
    int processedChunks = 0;

    for (const auto& chunk : chunks) {
        std::string key = "mysecretkey";
        std::ifstream chunkStream(
            (std::filesystem::path(chunkManager.chunksDir()) / chunk.id).string(),
            std::ios::binary);
        if (!chunkStream.is_open()) {
            return failRetrieve("Missing chunk: " + chunk.id);
        }

        ChecksumState checksum;
        if (!decryptStream(chunkStream, out, key, &checksum)) {
            return failRetrieve("Cannot stream chunk: " + chunk.id);
        }

        if (checksum.finalize() != chunk.checksum) {
            return failRetrieve("Data corruption in chunk: " + chunk.id);
        }

        processedChunks++;
        int percent = (processedChunks * 100) / totalChunks;

        updateProgress(fileId, percent);

        if (progressCallback) {
            progressCallback(percent);
        }

        LOG_DEBUG("Read chunk: " + chunk.id);
    }

    out.close();
    LOG_INFO("File reconstructed: " + outputPath);
    return true;
}

// ======================= DELETE =======================
bool StorageEngine::deleteFile(const std::string& fileId) {
    auto fileLock = lockFileOperation(storageRoot_, fileId);

    MetadataManager metadataManager(storageRoot_);
    ChunkManager chunkManager(storageRoot_);

    std::set<std::string> chunkIds;
    for (const auto& metadata : metadataManager.listMetadataVersions(fileId)) {
        for (const auto& chunk : metadata.chunks) {
            chunkIds.insert(chunk.id);
        }
    }
    bool success = true;

    if (!chunkIds.empty()) {
        for (const auto& chunkId : chunkIds) {
            if (!chunkManager.deleteChunk(chunkId)) {
                LOG_ERROR("Failed to delete chunk: " + chunkId);
                success = false;
            }
        }
    }

    if (!metadataManager.deleteMetadata(fileId)) {
        LOG_ERROR("Failed to delete metadata for file: " + fileId);
        success = false;
    }

    if (!success) {
        LOG_ERROR("Delete request failed for file: " + fileId);
        return false;
    }

    LOG_INFO("Deleted file: " + fileId);
    return true;
}

// ======================= LIST =======================
std::vector<std::string> StorageEngine::listFiles() {
    std::vector<std::string> files;
    const std::string path = metadataDir();

    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".meta") {
                continue;
            }

            std::string filename = entry.path().filename().string();

            if (filename.size() > 5) {
                files.push_back(filename.substr(0, filename.size() - 5));
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR(std::string("Cannot list files: ") + e.what());
    }

    return files;
}
