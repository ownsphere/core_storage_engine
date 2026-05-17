#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "models/chunk_info.h"

struct FileMetadata {
    std::string versionId;
    std::string previousVersionId;
    std::int64_t createdAtEpochMs = 0;
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
    bool loadMetadataVersion(const std::string& fileId,
                             const std::string& versionId,
                             FileMetadata& metadata);
    std::vector<FileMetadata> listMetadataVersions(const std::string& fileId);
    std::vector<ChunkInfo> loadChunks(const std::string& fileId);
    bool deleteMetadata(const std::string& fileId);
    void cleanupTempFiles();

    std::string metadataDir() const;
    std::string metadataPath(const std::string& fileId) const;
    std::string versionMetadataDir(const std::string& fileId) const;
    std::string versionMetadataPath(const std::string& fileId, const std::string& versionId) const;

private:
    std::string storageRoot_;
};
