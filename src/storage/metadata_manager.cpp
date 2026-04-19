#include "metadata_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

// ================= SAVE METADATA =================
bool MetadataManager::saveMetadata(const std::string& fileId,
                                   const std::vector<ChunkInfo>& chunks,
                                   size_t fileSize)
{
    std::string dir = "data/metadata/";
    std::filesystem::create_directories(dir);

    std::string finalPath = dir + fileId + ".meta";
    std::string tempPath = finalPath + ".tmp";

    // 1. Write to temp file
    std::ofstream out(tempPath, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "ERROR: Cannot open temp metadata file\n";
        return false;
    }

    // Format:
    // first line → file size
    // next lines → chunkId checksum
    out << fileSize << "\n";
    for (const auto& chunk : chunks) {
        out << chunk.id << " " << chunk.checksum << "\n";
    }

    // 2. Flush and validate
    out.flush();
    if (!out.good()) {
        std::cerr << "ERROR: Failed writing metadata\n";
        out.close();
        std::remove(tempPath.c_str());
        return false;
    }

    out.close();

    // 3. Atomic rename
    try {
        std::filesystem::rename(tempPath, finalPath);
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Atomic rename failed: " << e.what() << "\n";
        std::remove(tempPath.c_str());
        return false;
    }

    return true;
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