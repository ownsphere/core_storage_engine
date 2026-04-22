#include "metadata_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>

namespace {
bool writeAndSyncFile(const std::string& path, const std::string& content) {
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return false;
    }

    size_t totalWritten = 0;
    while (totalWritten < content.size()) {
        const ssize_t written = ::write(fd, content.data() + totalWritten, content.size() - totalWritten);
        if (written < 0) {
            ::close(fd);
            return false;
        }
        totalWritten += static_cast<size_t>(written);
    }

    const bool ok = (::fsync(fd) == 0);
    ::close(fd);
    return ok;
}

bool fsyncDirectory(const std::string& path) {
    const int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        return false;
    }

    const bool ok = (::fsync(fd) == 0);
    ::close(fd);
    return ok;
}
}

// ================= SAVE METADATA =================
bool MetadataManager::saveMetadata(const std::string& fileId,
                                   const std::vector<ChunkInfo>& chunks,
                                   size_t fileSize)
{
    std::string dir = "data/metadata/";
    std::filesystem::create_directories(dir);

    std::string finalPath = dir + fileId + ".meta";
    std::string tempPath = finalPath + ".tmp";

    std::ostringstream content;
    content << fileSize << "\n";
    for (const auto& chunk : chunks) {
        content << chunk.id << " " << chunk.checksum << "\n";
    }

    if (!writeAndSyncFile(tempPath, content.str())) {
        std::cerr << "ERROR: Failed writing metadata\n";
        std::filesystem::remove(tempPath);
        return false;
    }

    // 3. Atomic rename
    try {
        std::filesystem::rename(tempPath, finalPath);
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Atomic rename failed: " << e.what() << "\n";
        std::filesystem::remove(tempPath);
        return false;
    }

    return fsyncDirectory(dir);
}

// ================= LOAD METADATA =================
std::vector<ChunkInfo> MetadataManager::loadChunks(const std::string &fileId)
{
    std::vector<ChunkInfo> chunks;

    std::string path = "data/metadata/" + fileId + ".meta";
    std::ifstream in(path);

    if (!in.is_open()) {
        std::cerr << "ERROR: Cannot open metadata for file: " << fileId << std::endl;
        return chunks;
    }

    size_t fileSize;
    in >> fileSize; // first line

    std::string chunkId, checksum;

    while (in >> chunkId >> checksum) {
        ChunkInfo info;
        info.id = chunkId;
        info.checksum = checksum;
        chunks.push_back(info);
    }

    return chunks;
}

// ================= CLEANUP TEMP FILES =================
void MetadataManager::cleanupTempFiles()
{
    std::string dir = "data/metadata/";

    if (!std::filesystem::exists(dir)) return;

    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        std::string path = entry.path().string();

        // C++17 compatible ends_with
        if (path.size() >= 4 && path.substr(path.size() - 4) == ".tmp") {
            std::remove(path.c_str());
        }
    }
}
