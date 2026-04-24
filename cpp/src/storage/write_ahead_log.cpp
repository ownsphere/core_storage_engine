#include "write_ahead_log.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

#include "chunk_manager.h"
#include "metadata_manager.h"

namespace {
std::string stateToString(WalState state) {
    switch (state) {
        case WalState::Pending:
            return "PENDING";
        case WalState::Applying:
            return "APPLYING";
        case WalState::Committed:
            return "COMMITTED";
    }

    return "PENDING";
}

WalState stringToState(const std::string& value) {
    if (value == "APPLYING") {
        return WalState::Applying;
    }
    if (value == "COMMITTED") {
        return WalState::Committed;
    }
    return WalState::Pending;
}

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
}  // namespace

WriteAheadLog::WriteAheadLog(std::string storageRoot)
    : storageRoot_(std::move(storageRoot)) {}

std::string WriteAheadLog::walDir() const {
    return (std::filesystem::path(storageRoot_) / "wal").string();
}

std::string WriteAheadLog::walPath(const std::string& fileId) const {
    return (std::filesystem::path(walDir()) / (fileId + ".wal")).string();
}

bool WriteAheadLog::begin(const std::string& fileId,
                          size_t fileSize,
                          const std::vector<std::string>& oldChunkIds) {
    WalEntry entry;
    entry.fileId = fileId;
    entry.fileSize = fileSize;
    entry.state = WalState::Pending;
    entry.oldChunkIds = oldChunkIds;
    return writeEntry(entry);
}

bool WriteAheadLog::appendChunk(const std::string& fileId,
                                const ChunkInfo& chunk) {
    WalEntry entry;
    if (!loadEntry(walPath(fileId), entry)) {
        return false;
    }

    entry.newChunks.push_back(chunk);
    return writeEntry(entry);
}

bool WriteAheadLog::markApplying(const std::string& fileId) {
    WalEntry entry;
    if (!loadEntry(walPath(fileId), entry)) {
        return false;
    }

    entry.state = WalState::Applying;
    return writeEntry(entry);
}

bool WriteAheadLog::markCommitted(const std::string& fileId) {
    WalEntry entry;
    if (!loadEntry(walPath(fileId), entry)) {
        return false;
    }

    entry.state = WalState::Committed;
    return writeEntry(entry);
}

bool WriteAheadLog::remove(const std::string& fileId) {
    const bool removed =
        std::filesystem::remove(walPath(fileId)) || !std::filesystem::exists(walPath(fileId));
    if (!removed) {
        return false;
    }

    return fsyncDirectory(walDir());
}

bool WriteAheadLog::recoverPending(MetadataManager& metadataManager,
                                   ChunkManager& chunkManager) {
    const std::string dir = walDir();
    if (!std::filesystem::exists(dir)) {
        return true;
    }

    bool allRecovered = true;

    for (const auto& entryPath : std::filesystem::directory_iterator(dir)) {
        if (!entryPath.is_regular_file()) {
            continue;
        }

        WalEntry entry;
        if (!loadEntry(entryPath.path().string(), entry)) {
            allRecovered = false;
            continue;
        }

        if (entry.state == WalState::Pending) {
            for (const auto& chunk : entry.newChunks) {
                chunkManager.deleteChunk(chunk.id);
            }

            if (!remove(entry.fileId)) {
                allRecovered = false;
            }
            continue;
        }

        if (entry.state == WalState::Applying) {
            if (!metadataManager.saveMetadata(entry.fileId, entry.newChunks, entry.fileSize)) {
                allRecovered = false;
                continue;
            }
        }

        for (const auto& chunkId : entry.oldChunkIds) {
            chunkManager.deleteChunk(chunkId);
        }

        if (!remove(entry.fileId)) {
            allRecovered = false;
        }
    }

    return allRecovered;
}

bool WriteAheadLog::writeEntry(const WalEntry& entry) {
    std::filesystem::create_directories(walDir());

    const std::string path = walPath(entry.fileId);
    const std::string tempPath = path + ".tmp";
    std::ostringstream content;
    content << "STATE " << stateToString(entry.state) << "\n";
    content << "FILE " << entry.fileId << "\n";
    content << "SIZE " << entry.fileSize << "\n";

    for (const auto& chunkId : entry.oldChunkIds) {
        content << "OLD_CHUNK " << chunkId << "\n";
    }

    for (const auto& chunk : entry.newChunks) {
        content << "NEW_CHUNK " << chunk.id << " " << chunk.checksum << "\n";
    }

    if (!writeAndSyncFile(tempPath, content.str())) {
        std::filesystem::remove(tempPath);
        return false;
    }

    std::filesystem::rename(tempPath, path);
    return fsyncDirectory(walDir());
}

bool WriteAheadLog::loadEntry(const std::string& path, WalEntry& entry) const {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    entry = WalEntry{};

    std::string tag;
    while (in >> tag) {
        if (tag == "STATE") {
            std::string value;
            in >> value;
            entry.state = stringToState(value);
        } else if (tag == "FILE") {
            in >> entry.fileId;
        } else if (tag == "SIZE") {
            in >> entry.fileSize;
        } else if (tag == "OLD_CHUNK") {
            std::string chunkId;
            in >> chunkId;
            entry.oldChunkIds.push_back(chunkId);
        } else if (tag == "NEW_CHUNK") {
            ChunkInfo chunk;
            in >> chunk.id >> chunk.checksum;
            entry.newChunks.push_back(chunk);
        }
    }

    return !entry.fileId.empty();
}
