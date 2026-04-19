#include <sstream>
#include "storage_engine.h"
#include "chunk_manager.h"
#include "metadata_manager.h"
#include "checksum.h"
#include "encryption.h"
#include "logger.h"
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <mutex>

static std::unordered_map<std::string, int> progressMap;
static std::mutex progressMutex;

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

// ======================= WRITE =======================
bool StorageEngine::storeFile(const std::string &filePath, const std::string &fileId, ProgressCallback progressCallback){

    // ✅ START TIME
    auto startTime = std::chrono::system_clock::now();
    auto startHighRes = std::chrono::high_resolution_clock::now();

    ChunkManager chunkManager;
    MetadataManager metadataManager;

    std::ifstream in(filePath, std::ios::binary);

    if (!in.is_open()) {
        LOG_ERROR("Cannot open file: " + filePath);
        return false;
    }

    in.seekg(0, std::ios::end);
    size_t fileSize = in.tellg();
    in.seekg(0);

    if (fileSize == 0) {
        std::vector<ChunkInfo> emptyChunks;

        if (!metadataManager.saveMetadata(fileId, emptyChunks, 0)) {
            LOG_ERROR("Failed to save metadata for empty file: " + fileId);
            return false;
        }

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
    int chunkIndex = 0;
    std::vector<ChunkInfo> chunkInfos;

    while (true) {
        std::vector<char> buffer(CHUNK_SIZE);

        in.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = in.gcount();

        if (bytesRead <= 0) break;

        buffer.resize(bytesRead);

        std::string chunkId = fileId + "_chunk_" + std::to_string(chunkIndex++);

        std::string key = "mysecretkey";
        std::vector<char> encryptedData = encryptData(buffer, key);

        if (!chunkManager.writeChunk(chunkId, encryptedData)) {
            LOG_ERROR("Failed to write chunk: " + chunkId);
            return false;
        }

        std::string checksum = computeChecksum(buffer);

        ChunkInfo info;
        info.id = chunkId;
        info.checksum = checksum;

        chunkInfos.push_back(info);

        processed += bytesRead;
        int percent = static_cast<int>((processed * 100) / fileSize);

        updateProgress(fileId, percent);
        if (progressCallback) progressCallback(percent);
    }

    if (!metadataManager.saveMetadata(fileId, chunkInfos, fileSize)) {
        LOG_ERROR("Failed to save metadata: " + fileId);
        return false;
    }

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
    ChunkManager chunkManager;
    MetadataManager metadataManager;

    std::string metaPath = "data/metadata/" + fileId + ".meta";

    if (!std::filesystem::exists(metaPath)) {
        LOG_ERROR("File does not exist: " + fileId);
        return false;
    }

    auto chunks = metadataManager.loadChunks(fileId);

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

    int totalChunks = chunks.size();
    int processedChunks = 0;

    for (const auto& chunk : chunks) {
        std::string key = "mysecretkey";

        auto encryptedData = chunkManager.readChunk(chunk.id);
        if (encryptedData.empty()) {
            LOG_ERROR("Missing chunk: " + chunk.id);
            return false;
        }

        auto data = decryptData(encryptedData, key);

        if (computeChecksum(data) != chunk.checksum) {
            LOG_ERROR("Data corruption in chunk: " + chunk.id);
            return false;
        }

        out.write(data.data(), data.size());

        processedChunks++;
        int percent = (processedChunks * 100) / totalChunks;

        updateProgress(fileId, percent);

        if (progressCallback) {
            progressCallback(percent);
        }

        LOG_DEBUG("Read chunk: " + chunk.id);
    }

    LOG_INFO("File reconstructed: " + outputPath);
    return true;
}

// ======================= DELETE =======================
bool StorageEngine::deleteFile(const std::string& fileId) {
    MetadataManager metadataManager;
    ChunkManager chunkManager;

    std::vector<ChunkInfo> chunks = metadataManager.loadChunks(fileId);

    if (!chunks.empty()) {
        for (const auto& chunk : chunks) {
            std::string path = "data/chunks/" + chunk.id;
            std::remove(path.c_str());
        }
    }

    std::string metaPath = "data/metadata/" + fileId + ".meta";
    std::remove(metaPath.c_str());

    LOG_INFO("Deleted file: " + fileId);
    return true;
}

// ======================= LIST =======================
std::vector<std::string> StorageEngine::listFiles() {
    std::vector<std::string> files;
    std::string path = "data/metadata/";

    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
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