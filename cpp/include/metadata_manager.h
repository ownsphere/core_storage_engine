#pragma once
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>
#include "models/chunk_info.h"

struct FileMetadata {
    std::string fileId;
    std::string storageKey;
    std::string originalFilename;
    std::string extension;
    std::string contentType;
    std::string checksum;
    std::string versionId;
    std::string previousVersionId;
    std::int64_t uploadedAtEpochMs = 0;
    std::int64_t createdAtEpochMs = 0;
    size_t fileSize = 0;
    std::vector<ChunkInfo> chunks;
};

class MetadataManager {
public:
    explicit MetadataManager(std::string storageRoot = "data");

    bool saveMetadata(const FileMetadata& metadata);

    bool loadMetadata(const std::string& fileId, FileMetadata& metadata);
    bool loadMetadataVersion(const std::string& fileId,
                             const std::string& versionId,
                             FileMetadata& metadata);
    std::vector<FileMetadata> listMetadataVersions(const std::string& fileId);
    std::vector<ChunkInfo> loadChunks(const std::string& fileId);
    std::vector<std::string> listFileIds();
    std::unordered_set<std::string> collectReferencedChunkIds(
        const std::string& excludedFileId = "");
    bool deleteMetadata(const std::string& fileId);
    void cleanupTempFiles();

    std::string metadataDir() const;
    std::string metadataPath(const std::string& fileId) const;
    std::string versionMetadataDir(const std::string& fileId) const;
    std::string versionMetadataPath(const std::string& fileId, const std::string& versionId) const;

private:
    std::string storageRoot_;
};
