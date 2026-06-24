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
#include <exception>
#include <cctype>

static std::unordered_map<std::string, int> progressMap;
static std::mutex progressMutex;
static std::unordered_map<std::string, std::shared_ptr<std::mutex>> fileMutexes;
static std::mutex fileMutexesGuard;
namespace {
std::string storageMetadataDir(const std::string& storageRoot) {
    return (std::filesystem::path(storageRoot) / "metadata").string();
}

std::string storageMetadataPath(const std::string& storageRoot, const std::string& fileId) {
    return (std::filesystem::path(storageMetadataDir(storageRoot)) / (fileId + ".meta")).string();
}

std::unordered_set<std::string> collectReferencedChunkIds(const std::string& storageRoot,
                                                          const std::string& excludedMetadataFileId = "",
                                                          const std::string& excludedWalFileId = "") {
    std::unordered_set<std::string> referencedChunkIds;
    MetadataManager metadataManager(storageRoot);
    WriteAheadLog wal(storageRoot);
    referencedChunkIds = metadataManager.collectReferencedChunkIds(excludedMetadataFileId);

    for (const auto& entry : wal.listEntries()) {
        if (!excludedWalFileId.empty() && entry.fileId == excludedWalFileId) {
            continue;
        }

        for (const auto& chunk : entry.newChunks) {
            referencedChunkIds.insert(chunk.id);
        }
    }

    return referencedChunkIds;
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
    liveChunkIds = collectReferencedChunkIds(storageRoot);

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

std::string normalizeExtension(const std::string& extension) {
    if (extension.empty()) {
        return "";
    }
    if (extension.front() == '.') {
        return extension.substr(1);
    }
    return extension;
}

std::string lowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string inferContentType(const std::string& filename, const std::string& extension) {
    static const std::unordered_map<std::string, std::string> contentTypes = {
        {"txt", "text/plain"},
        {"json", "application/json"},
        {"pdf", "application/pdf"},
        {"png", "image/png"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"gif", "image/gif"},
        {"webp", "image/webp"},
        {"svg", "image/svg+xml"},
        {"csv", "text/csv"},
        {"xml", "application/xml"},
        {"html", "text/html"},
        {"htm", "text/html"},
        {"mp4", "video/mp4"},
        {"mov", "video/quicktime"},
        {"mp3", "audio/mpeg"},
        {"wav", "audio/wav"},
        {"zip", "application/zip"},
        {"gz", "application/gzip"},
    };

    std::string candidateExtension = normalizeExtension(extension);
    if (candidateExtension.empty()) {
        const std::filesystem::path path(filename);
        if (path.has_extension()) {
            candidateExtension = normalizeExtension(path.extension().string());
        }
    }

    const auto it = contentTypes.find(lowerCopy(candidateExtension));
    if (it != contentTypes.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

FileMetadata buildStoredMetadata(const std::string& filePath,
                                 const std::string& fileId,
                                 const StoreFileOptions& options,
                                 size_t fileSize,
                                 const std::string& checksum,
                                 const std::vector<ChunkInfo>& chunks) {
    FileMetadata metadata;
    metadata.fileId = fileId;
    metadata.storageKey = fileId;
    metadata.originalFilename = options.originalFilename.empty()
        ? std::filesystem::path(filePath).filename().string()
        : options.originalFilename;
    if (metadata.originalFilename.empty()) {
        metadata.originalFilename = fileId;
    }

    metadata.extension = normalizeExtension(options.extension);
    if (metadata.extension.empty()) {
        const std::filesystem::path filename(metadata.originalFilename);
        if (filename.has_extension()) {
            metadata.extension = normalizeExtension(filename.extension().string());
        }
    }

    metadata.contentType = options.contentType.empty()
        ? inferContentType(metadata.originalFilename, metadata.extension)
        : options.contentType;
    metadata.checksum = checksum;
    metadata.uploadedAtEpochMs = options.uploadedAtEpochMs > 0
        ? options.uploadedAtEpochMs
        : std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch()).count();
    metadata.fileSize = fileSize;
    metadata.chunks = chunks;
    return metadata;
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
    startBackgroundMaintenance();
}

StorageEngine::~StorageEngine() {
    waitForBackgroundTasks();
}

void StorageEngine::waitForBackgroundTasks() {
    if (maintenanceThread_.joinable()) {
        maintenanceThread_.join();
    }
}

void StorageEngine::startBackgroundMaintenance() {
    maintenanceThread_ = std::thread(&StorageEngine::runBackgroundMaintenance, this);
}

void StorageEngine::runBackgroundMaintenance() {
    try {
        std::lock_guard<std::mutex> maintenanceLock(maintenanceMutex_);
        MetadataManager metadataManager(storageRoot_);
        metadataManager.cleanupTempFiles();
        collectGarbageChunks(storageRoot_);
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Background maintenance failed: ") + e.what());
    } catch (...) {
        LOG_ERROR("Background maintenance failed with unknown error");
    }
}

std::string StorageEngine::metadataPath(const std::string& fileId) const {
    return storageMetadataPath(storageRoot_, fileId);
}

std::string StorageEngine::metadataDir() const {
    return storageMetadataDir(storageRoot_);
}

// ======================= WRITE =======================
bool StorageEngine::storeFile(const std::string &filePath, const std::string &fileId, ProgressCallback progressCallback){
    return storeFile(filePath, fileId, StoreFileOptions{}, progressCallback);
}

bool StorageEngine::storeFile(const std::string &filePath,
                              const std::string &fileId,
                              const StoreFileOptions& options,
                              ProgressCallback progressCallback){
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

    ChecksumState fullFileChecksum;

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
        const std::string checksum = fullFileChecksum.finalize();
        if (!options.checksum.empty() && options.checksum != checksum) {
            LOG_ERROR("Provided checksum does not match file contents for empty file: " + fileId);
            return false;
        }

        FileMetadata metadata = buildStoredMetadata(filePath, fileId, options, 0, checksum, emptyChunks);
        if (!metadataManager.saveMetadata(metadata)) {
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

    size_t processed = 0;
    std::vector<ChunkInfo> chunkInfos;
    std::vector<std::string> oldChunkIds;
    std::vector<std::string> attemptedCreatedChunkIds;

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

        std::string checksum = computeChecksum(buffer);
        fullFileChecksum.update(buffer);
        std::string chunkId = checksum;

        ChunkInfo info;
        info.id = chunkId;
        info.checksum = checksum;

        if (!wal.appendChunk(fileId, info)) {
            const auto referencedChunkIds = collectReferencedChunkIds(storageRoot_, "", fileId);
            for (const auto& attemptedChunkId : attemptedCreatedChunkIds) {
                if (referencedChunkIds.find(attemptedChunkId) == referencedChunkIds.end()) {
                    chunkManager.deleteChunk(attemptedChunkId);
                }
            }
            wal.remove(fileId);
            LOG_ERROR("Failed to append chunk to WAL: " + chunkId);
            return false;
        }

        std::string key = "mysecretkey";
        std::vector<char> encryptedData = encryptData(buffer, key);

        bool createdChunk = false;
        if (!chunkManager.writeChunk(chunkId, encryptedData, &createdChunk)) {
            const auto referencedChunkIds = collectReferencedChunkIds(storageRoot_, "", fileId);
            for (const auto& attemptedChunkId : attemptedCreatedChunkIds) {
                if (referencedChunkIds.find(attemptedChunkId) == referencedChunkIds.end()) {
                    chunkManager.deleteChunk(attemptedChunkId);
                }
            }
            wal.remove(fileId);
            LOG_ERROR("Failed to write chunk: " + chunkId);
            return false;
        }

        if (createdChunk) {
            attemptedCreatedChunkIds.push_back(chunkId);
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

    const std::string fileChecksum = fullFileChecksum.finalize();
    if (!options.checksum.empty() && options.checksum != fileChecksum) {
        LOG_ERROR("Provided checksum does not match file contents for file: " + fileId);
        return false;
    }

    FileMetadata metadata = buildStoredMetadata(filePath, fileId, options, fileSize, fileChecksum, chunkInfos);
    if (!metadataManager.saveMetadata(metadata)) {
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

bool StorageEngine::getFileMetadata(const std::string& fileId, FileMetadata& metadata) {
    MetadataManager metadataManager(storageRoot_);
    return metadataManager.loadMetadata(fileId, metadata);
}

// ======================= DELETE =======================
bool StorageEngine::deleteFile(const std::string& fileId) {
    auto fileLock = lockFileOperation(storageRoot_, fileId);
    std::lock_guard<std::mutex> maintenanceLock(maintenanceMutex_);

    MetadataManager metadataManager(storageRoot_);
    ChunkManager chunkManager(storageRoot_);

    std::set<std::string> chunkIds;
    for (const auto& metadata : metadataManager.listMetadataVersions(fileId)) {
        for (const auto& chunk : metadata.chunks) {
            chunkIds.insert(chunk.id);
        }
    }
    const auto referencedByOtherFilesOrWal = collectReferencedChunkIds(storageRoot_, fileId, fileId);
    bool success = metadataManager.deleteMetadata(fileId);

    if (!success) {
        LOG_ERROR("Failed to delete metadata for file: " + fileId);
        LOG_ERROR("Delete request failed for file: " + fileId);
        return false;
    }

    for (const auto& chunkId : chunkIds) {
        if (referencedByOtherFilesOrWal.find(chunkId) != referencedByOtherFilesOrWal.end()) {
            continue;
        }

        if (!chunkManager.deleteChunk(chunkId)) {
            LOG_ERROR("Failed to delete chunk: " + chunkId);
            success = false;
        }
    }

    if (!success) {
        LOG_ERROR("Delete request failed for file: " + fileId);
        return false;
    }

    LOG_INFO("Deleted file: " + fileId);
    return true;
}

bool StorageEngine::deleteAllFiles() {
    const auto files = listFiles();
    bool success = true;

    for (const auto& fileId : files) {
        if (!deleteFile(fileId)) {
            LOG_ERROR("Failed to delete file during delete-all: " + fileId);
            success = false;
        }
    }

    if (success) {
        LOG_INFO("Deleted all files");
    } else {
        LOG_ERROR("Delete-all completed with failures");
    }

    return success;
}

// ======================= LIST =======================
std::vector<std::string> StorageEngine::listFiles() {
    MetadataManager metadataManager(storageRoot_);
    return metadataManager.listFileIds();
}
