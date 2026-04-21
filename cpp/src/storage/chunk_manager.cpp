#include "chunk_manager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

namespace {
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

bool ChunkManager::writeChunk(const std::string& chunkId, const std::vector<char>& data) {
    const std::string dir = "data/chunks";
    std::filesystem::create_directories(dir);

    const std::string path = dir + "/" + chunkId;
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        std::cerr << "ERROR: Cannot write chunk: " << chunkId << std::endl;
        return false;
    }

    const char* buffer = data.empty() ? nullptr : data.data();
    size_t totalWritten = 0;
    while (totalWritten < data.size()) {
        const ssize_t written = ::write(fd, buffer + totalWritten, data.size() - totalWritten);
        if (written < 0) {
            ::close(fd);
            return false;
        }
        totalWritten += static_cast<size_t>(written);
    }

    const bool synced = (::fsync(fd) == 0);
    ::close(fd);

    return synced && fsyncDirectory(dir);
}

std::vector<char> ChunkManager::readChunk(const std::string& chunkId) {
    std::ifstream in("data/chunks/" + chunkId, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "ERROR: Cannot open chunk: " << chunkId << std::endl;
        return {};
    }

    return std::vector<char>((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
}

bool ChunkManager::deleteChunk(const std::string& chunkId) {
    const std::string path = "data/chunks/" + chunkId;
    const bool removed = std::filesystem::remove(path) || !std::filesystem::exists(path);
    if (!removed) {
        return false;
    }

    return fsyncDirectory("data/chunks");
}
