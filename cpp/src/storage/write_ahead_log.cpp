#include "write_ahead_log.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>
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

std::unordered_set<std::string> collectReferencedChunkIds(MetadataManager& metadataManager,
                                                          const std::vector<WalEntry>& walEntries,
                                                          const std::string& excludedWalFileId) {
    std::unordered_set<std::string> referencedChunkIds =
        metadataManager.collectReferencedChunkIds();

    for (const auto& walEntry : walEntries) {
        if (walEntry.fileId == excludedWalFileId) {
            continue;
        }

        for (const auto& chunk : walEntry.newChunks) {
            referencedChunkIds.insert(chunk.id);
        }
    }

    return referencedChunkIds;
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
    std::error_code removeError;
    const bool removed = std::filesystem::remove(walPath(fileId), removeError);
    if (removeError) {
        return false;
    }

    std::error_code existsError;
    const bool stillExists = std::filesystem::exists(walPath(fileId), existsError);
    if (existsError) {
        return false;
    }

    if (!removed && !stillExists) {
        return true;
    }

    if (!removed) {
        return false;
    }

    return fsyncDirectory(walDir());
}

bool WriteAheadLog::recoverPending(MetadataManager& metadataManager,
                                   ChunkManager& chunkManager) {
    bool allRecovered = true;

    const auto entries = listEntries();
    for (const auto& entry : entries) {
        if (entry.state == WalState::Pending) {
            const auto referencedChunkIds = collectReferencedChunkIds(metadataManager, entries, entry.fileId);

            for (const auto& chunk : entry.newChunks) {
                if (referencedChunkIds.find(chunk.id) == referencedChunkIds.end()) {
                    chunkManager.deleteChunk(chunk.id);
                }
            }

            if (!remove(entry.fileId)) {
                allRecovered = false;
            }
            continue;
        }

        if (entry.state == WalState::Applying) {
            FileMetadata metadata;
            metadata.fileId = entry.fileId;
            metadata.storageKey = entry.fileId;
            metadata.originalFilename = entry.fileId;
            metadata.contentType = "application/octet-stream";
            metadata.fileSize = entry.fileSize;
            metadata.chunks = entry.newChunks;

            FileMetadata previousMetadata;
            if (metadataManager.loadMetadata(entry.fileId, previousMetadata)) {
                metadata.originalFilename = previousMetadata.originalFilename;
                metadata.extension = previousMetadata.extension;
                metadata.contentType = previousMetadata.contentType;
                metadata.checksum = previousMetadata.checksum;
                metadata.uploadedAtEpochMs = previousMetadata.uploadedAtEpochMs;
            }

            if (!metadataManager.saveMetadata(metadata)) {
                allRecovered = false;
                continue;
            }
        }

        if (!remove(entry.fileId)) {
            allRecovered = false;
        }
    }

    return allRecovered;
}

std::vector<WalEntry> WriteAheadLog::listEntries() const {
    std::vector<WalEntry> entries;
    const std::string dir = walDir();
    if (!std::filesystem::exists(dir)) {
        return entries;
    }

    for (const auto& entryPath : std::filesystem::directory_iterator(dir)) {
        if (!entryPath.is_regular_file()) {
            continue;
        }

        WalEntry entry;
        if (loadEntry(entryPath.path().string(), entry)) {
            entries.push_back(std::move(entry));
        }
    }

    return entries;
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
