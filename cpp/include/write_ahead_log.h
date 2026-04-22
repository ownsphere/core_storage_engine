#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "models/chunk_info.h"

class MetadataManager;
class ChunkManager;

enum class WalState {
    Pending,
    Applying,
    Committed
};

struct WalEntry {
    std::string fileId;
    size_t fileSize = 0;
    WalState state = WalState::Pending;
    std::vector<std::string> oldChunkIds;
    std::vector<ChunkInfo> newChunks;
};

class WriteAheadLog {
public:
    explicit WriteAheadLog(std::string storageRoot = "data");

    bool begin(const std::string& fileId,
               size_t fileSize,
               const std::vector<std::string>& oldChunkIds);

    bool appendChunk(const std::string& fileId,
                     const ChunkInfo& chunk);

    bool markApplying(const std::string& fileId);
    bool markCommitted(const std::string& fileId);
    bool remove(const std::string& fileId);

    bool recoverPending(MetadataManager& metadataManager,
                        ChunkManager& chunkManager);

private:
    std::string walDir() const;
    std::string walPath(const std::string& fileId) const;
    bool writeEntry(const WalEntry& entry);
    bool loadEntry(const std::string& path, WalEntry& entry) const;

    std::string storageRoot_;
};
