#include "chunk_manager.h"
#include <cerrno>
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

ChunkManager::ChunkManager(std::string storageRoot)
    : storageRoot_(std::move(storageRoot)) {}

std::string ChunkManager::chunksDir() const {
    return (std::filesystem::path(storageRoot_) / "chunks").string();
}

bool ChunkManager::writeChunk(const std::string& chunkId,
                              const std::vector<char>& data,
                              bool* created) {
    const std::string dir = chunksDir();
    std::filesystem::create_directories(dir);

    const std::string path = (std::filesystem::path(dir) / chunkId).string();
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd < 0) {
        if (errno == EEXIST) {
            std::error_code ec;
            if (!std::filesystem::is_regular_file(path, ec) || ec) {
                std::cerr << "ERROR: Cannot write chunk: " << chunkId << std::endl;
                return false;
            }
            if (created != nullptr) {
                *created = false;
            }
            return true;
        }
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

    if (created != nullptr) {
        *created = true;
    }

    return synced && fsyncDirectory(dir);
}

std::vector<char> ChunkManager::readChunk(const std::string& chunkId) {
    std::ifstream in((std::filesystem::path(chunksDir()) / chunkId).string(), std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "ERROR: Cannot open chunk: " << chunkId << std::endl;
        return {};
    }

    return std::vector<char>((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
}

bool ChunkManager::deleteChunk(const std::string& chunkId) {
    const std::string dir = chunksDir();
    const std::string path = (std::filesystem::path(dir) / chunkId).string();

    std::error_code removeError;
    const bool removed = std::filesystem::remove(path, removeError);
    if (removeError) {
        return false;
    }

    std::error_code existsError;
    const bool stillExists = std::filesystem::exists(path, existsError);
    if (existsError) {
        return false;
    }

    if (!removed && !stillExists) {
        return true;
    }

    if (!removed) {
        return false;
    }

    return fsyncDirectory(dir);
}
