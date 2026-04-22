#pragma once
#include <string>
#include <vector>
#include "models/chunk_info.h"


class MetadataManager {
public:
    explicit MetadataManager(std::string storageRoot = "data");

    bool saveMetadata(const std::string& fileId,
                      const std::vector<ChunkInfo>& chunks,
                      size_t size);

    std::vector<ChunkInfo> loadChunks(const std::string& fileId);
    void cleanupTempFiles();

    std::string metadataDir() const;
    std::string metadataPath(const std::string& fileId) const;

private:
    std::string storageRoot_;
};
