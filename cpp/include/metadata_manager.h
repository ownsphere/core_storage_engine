#pragma once
#include <string>
#include <vector>
#include "models/chunk_info.h"

struct FileMetadata {
    size_t fileSize = 0;
    std::vector<ChunkInfo> chunks;
};

class MetadataManager {
public:
    explicit MetadataManager(std::string storageRoot = "data");

    bool saveMetadata(const std::string& fileId,
                      const std::vector<ChunkInfo>& chunks,
                      size_t size);

    bool loadMetadata(const std::string& fileId, FileMetadata& metadata);
    std::vector<ChunkInfo> loadChunks(const std::string& fileId);
    bool deleteMetadata(const std::string& fileId);
    void cleanupTempFiles();

    std::string metadataDir() const;
    std::string metadataPath(const std::string& fileId) const;

private:
    std::string storageRoot_;
};
