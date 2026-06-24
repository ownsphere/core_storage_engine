#include "metadata_manager.h"
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <unistd.h>

namespace {
using MetadataCacheKey = std::string;

std::mutex metadataCacheMutex;
std::unordered_map<MetadataCacheKey, FileMetadata> metadataCache;
std::atomic<unsigned long long> metadataVersionCounter{0};

struct MetadataIndex {
    bool loaded = false;
    std::unordered_set<std::string> fileIds;
    std::unordered_map<std::string, std::unordered_set<std::string>> chunkIdsByFile;
};

std::mutex metadataIndexMutex;
std::unordered_map<std::string, MetadataIndex> metadataIndexes;

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

MetadataCacheKey cacheKeyFor(const std::string& storageRoot, const std::string& fileId) {
    return storageRoot + "::" + fileId;
}

std::string createVersionId() {
    const auto timestamp = static_cast<unsigned long long>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    const auto counter = metadataVersionCounter.fetch_add(1);
    return std::to_string(timestamp) + "_" + std::to_string(counter);
}

std::int64_t currentEpochMillis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

bool isNumber(const std::string& value) {
    if (value.empty()) {
        return false;
    }

    return std::all_of(value.begin(), value.end(), [](unsigned char ch) {
        return std::isdigit(ch) != 0;
    });
}

std::string normalizeExtension(const std::string& extension) {
    if (extension.empty()) {
        return "";
    }
    if (extension.front() == '.') {
        return extension.substr(1);
    }
    return extension;
}

std::string serializeMetadata(const FileMetadata& metadata) {
    std::ostringstream content;
    content << "FILE_ID " << metadata.fileId << "\n";
    content << "STORAGE_KEY " << metadata.storageKey << "\n";
    content << "ORIGINAL_FILENAME " << metadata.originalFilename << "\n";
    content << "EXTENSION " << normalizeExtension(metadata.extension) << "\n";
    content << "CONTENT_TYPE " << metadata.contentType << "\n";
    content << "CHECKSUM " << metadata.checksum << "\n";
    content << "UPLOADED_AT " << metadata.uploadedAtEpochMs << "\n";
    content << "VERSION_ID " << metadata.versionId << "\n";
    content << "PREVIOUS_VERSION_ID "
            << (metadata.previousVersionId.empty() ? "-" : metadata.previousVersionId) << "\n";
    content << "CREATED_AT " << metadata.createdAtEpochMs << "\n";
    content << "FILE_SIZE " << metadata.fileSize << "\n";
    for (const auto& chunk : metadata.chunks) {
        content << "CHUNK " << chunk.id << " " << chunk.checksum << "\n";
    }
    return content.str();
}

bool parseMetadata(std::istream& in, FileMetadata& metadata) {
    std::string firstToken;
    if (!(in >> firstToken)) {
        return false;
    }

    if (isNumber(firstToken)) {
        metadata.fileSize = static_cast<size_t>(std::strtoull(firstToken.c_str(), nullptr, 10));

        std::string chunkId;
        std::string checksum;
        while (in >> chunkId >> checksum) {
            ChunkInfo info;
            info.id = chunkId;
            info.checksum = checksum;
            metadata.chunks.push_back(info);
        }
        return true;
    }

    std::string value;
    if (!(in >> value)) {
        return false;
    }

    auto assignField = [&](const std::string& key, const std::string& fieldValue) {
        if (key == "VERSION_ID") {
            metadata.versionId = fieldValue;
            return true;
        }
        if (key == "FILE_ID") {
            metadata.fileId = fieldValue;
            return true;
        }
        if (key == "STORAGE_KEY") {
            metadata.storageKey = fieldValue;
            return true;
        }
        if (key == "ORIGINAL_FILENAME") {
            metadata.originalFilename = fieldValue;
            return true;
        }
        if (key == "EXTENSION") {
            metadata.extension = fieldValue;
            return true;
        }
        if (key == "CONTENT_TYPE") {
            metadata.contentType = fieldValue;
            return true;
        }
        if (key == "CHECKSUM") {
            metadata.checksum = fieldValue;
            return true;
        }
        if (key == "UPLOADED_AT") {
            metadata.uploadedAtEpochMs = std::strtoll(fieldValue.c_str(), nullptr, 10);
            return true;
        }
        if (key == "PREVIOUS_VERSION_ID") {
            metadata.previousVersionId = (fieldValue == "-" ? "" : fieldValue);
            return true;
        }
        if (key == "CREATED_AT") {
            metadata.createdAtEpochMs = std::strtoll(fieldValue.c_str(), nullptr, 10);
            return true;
        }
        if (key == "FILE_SIZE") {
            metadata.fileSize = static_cast<size_t>(std::strtoull(fieldValue.c_str(), nullptr, 10));
            return true;
        }
        if (key == "CHUNK") {
            ChunkInfo info;
            info.id = fieldValue;
            if (!(in >> info.checksum)) {
                return false;
            }
            metadata.chunks.push_back(info);
            return true;
        }
        return false;
    };

    if (!assignField(firstToken, value)) {
        return false;
    }

    std::string key;
    while (in >> key >> value) {
        if (!assignField(key, value)) {
            return false;
        }
    }

    return true;
}

void applyLegacyDefaults(const std::string& fileId, FileMetadata& metadata) {
    if (metadata.fileId.empty()) {
        metadata.fileId = fileId;
    }
    if (metadata.storageKey.empty()) {
        metadata.storageKey = metadata.fileId;
    }
    if (metadata.originalFilename.empty()) {
        metadata.originalFilename = metadata.fileId;
    }
    if (metadata.extension.empty()) {
        const std::filesystem::path filename(metadata.originalFilename);
        if (filename.has_extension()) {
            metadata.extension = normalizeExtension(filename.extension().string());
        }
    }
    if (metadata.contentType.empty()) {
        metadata.contentType = "application/octet-stream";
    }
    if (metadata.uploadedAtEpochMs == 0) {
        metadata.uploadedAtEpochMs = metadata.createdAtEpochMs;
    }
}

bool loadMetadataFromPath(const std::string& path, FileMetadata& metadata) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    return parseMetadata(in, metadata);
}

void addMetadataChunksToIndex(MetadataIndex& index,
                              const std::string& fileId,
                              const FileMetadata& metadata) {
    auto& chunkIds = index.chunkIdsByFile[fileId];
    for (const auto& chunk : metadata.chunks) {
        chunkIds.insert(chunk.id);
    }
}

MetadataIndex buildMetadataIndex(const std::string& storageRoot) {
    MetadataIndex index;
    index.loaded = true;

    const std::filesystem::path metadataRoot =
        std::filesystem::path(storageRoot) / "metadata";
    if (!std::filesystem::exists(metadataRoot)) {
        return index;
    }

    for (const auto& entry : std::filesystem::directory_iterator(metadataRoot)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".meta") {
            continue;
        }

        const std::string fileId = entry.path().stem().string();
        index.fileIds.insert(fileId);
        index.chunkIdsByFile.try_emplace(fileId);

        FileMetadata metadata;
        if (loadMetadataFromPath(entry.path().string(), metadata)) {
            addMetadataChunksToIndex(index, fileId, metadata);
        }

        const std::filesystem::path versionsDir =
            metadataRoot / "versions" / fileId;
        if (!std::filesystem::exists(versionsDir)) {
            continue;
        }

        for (const auto& versionEntry : std::filesystem::directory_iterator(versionsDir)) {
            if (!versionEntry.is_regular_file() ||
                versionEntry.path().extension() != ".meta") {
                continue;
            }

            FileMetadata versionMetadata;
            if (loadMetadataFromPath(versionEntry.path().string(), versionMetadata)) {
                addMetadataChunksToIndex(index, fileId, versionMetadata);
            }
        }
    }

    return index;
}

MetadataIndex& ensureMetadataIndexLocked(const std::string& storageRoot) {
    auto& index = metadataIndexes[storageRoot];
    if (!index.loaded) {
        index = buildMetadataIndex(storageRoot);
    }
    return index;
}
}

MetadataManager::MetadataManager(std::string storageRoot)
    : storageRoot_(std::move(storageRoot)) {}

std::string MetadataManager::metadataDir() const {
    return (std::filesystem::path(storageRoot_) / "metadata").string();
}

std::string MetadataManager::metadataPath(const std::string& fileId) const {
    return (std::filesystem::path(metadataDir()) / (fileId + ".meta")).string();
}

std::string MetadataManager::versionMetadataDir(const std::string& fileId) const {
    return (std::filesystem::path(metadataDir()) / "versions" / fileId).string();
}

std::string MetadataManager::versionMetadataPath(const std::string& fileId,
                                                 const std::string& versionId) const {
    return (std::filesystem::path(versionMetadataDir(fileId)) / (versionId + ".meta")).string();
}

// ================= SAVE METADATA =================
bool MetadataManager::saveMetadata(const FileMetadata& sourceMetadata)
{
    const std::string& fileId = sourceMetadata.fileId;
    const std::string dir = metadataDir();
    const std::string versionsDir = versionMetadataDir(fileId);
    std::filesystem::create_directories(dir);
    std::filesystem::create_directories(versionsDir);

    const std::string finalPath = metadataPath(fileId);
    const std::string tempPath = finalPath + ".tmp";

    FileMetadata previousMetadata;
    const bool hadPrevious = loadMetadataFromPath(finalPath, previousMetadata);

    FileMetadata metadata;
    metadata.fileId = fileId;
    metadata.storageKey = sourceMetadata.storageKey;
    metadata.originalFilename = sourceMetadata.originalFilename;
    metadata.extension = normalizeExtension(sourceMetadata.extension);
    metadata.contentType = sourceMetadata.contentType;
    metadata.checksum = sourceMetadata.checksum;
    metadata.uploadedAtEpochMs = sourceMetadata.uploadedAtEpochMs;
    metadata.versionId = createVersionId();
    metadata.previousVersionId = hadPrevious ? previousMetadata.versionId : "";
    metadata.createdAtEpochMs = currentEpochMillis();
    metadata.fileSize = sourceMetadata.fileSize;
    metadata.chunks = sourceMetadata.chunks;

    const std::string versionPath = versionMetadataPath(fileId, metadata.versionId);
    const std::string versionTempPath = versionPath + ".tmp";

    const std::string content = serializeMetadata(metadata);

    if (!writeAndSyncFile(tempPath, content)) {
        std::cerr << "ERROR: Failed writing metadata\n";
        std::filesystem::remove(tempPath);
        return false;
    }

    if (!writeAndSyncFile(versionTempPath, content)) {
        std::cerr << "ERROR: Failed writing version metadata\n";
        std::filesystem::remove(tempPath);
        std::filesystem::remove(versionTempPath);
        return false;
    }

    try {
        std::filesystem::rename(versionTempPath, versionPath);
        std::filesystem::rename(tempPath, finalPath);
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Atomic rename failed: " << e.what() << "\n";
        std::filesystem::remove(tempPath);
        std::filesystem::remove(versionTempPath);
        return false;
    }

    if (!fsyncDirectory(versionsDir) || !fsyncDirectory(dir)) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(metadataCacheMutex);
        metadataCache[cacheKeyFor(storageRoot_, fileId)] = metadata;
    }

    {
        std::lock_guard<std::mutex> lock(metadataIndexMutex);
        auto& index = metadataIndexes[storageRoot_];
        if (index.loaded) {
            index.fileIds.insert(fileId);
            addMetadataChunksToIndex(index, fileId, metadata);
        }
    }
    return true;
}

// ================= LOAD METADATA =================
bool MetadataManager::loadMetadata(const std::string& fileId, FileMetadata& metadata)
{
    const MetadataCacheKey key = cacheKeyFor(storageRoot_, fileId);
    {
        std::lock_guard<std::mutex> lock(metadataCacheMutex);
        auto it = metadataCache.find(key);
        if (it != metadataCache.end()) {
            metadata = it->second;
            return true;
        }
    }

    const std::string path = metadataPath(fileId);
    FileMetadata loadedMetadata;
    if (!loadMetadataFromPath(path, loadedMetadata)) {
        std::cerr << "ERROR: Cannot open metadata for file: " << fileId << std::endl;
        return false;
    }

    applyLegacyDefaults(fileId, loadedMetadata);

    {
        std::lock_guard<std::mutex> lock(metadataCacheMutex);
        metadataCache[key] = loadedMetadata;
    }

    metadata = std::move(loadedMetadata);
    return true;
}

bool MetadataManager::loadMetadataVersion(const std::string& fileId,
                                          const std::string& versionId,
                                          FileMetadata& metadata)
{
    if (!loadMetadataFromPath(versionMetadataPath(fileId, versionId), metadata)) {
        return false;
    }
    applyLegacyDefaults(fileId, metadata);
    return true;
}

std::vector<FileMetadata> MetadataManager::listMetadataVersions(const std::string& fileId)
{
    std::vector<FileMetadata> versions;
    std::unordered_map<std::string, size_t> seenVersionIds;

    FileMetadata currentMetadata;
    if (loadMetadataFromPath(metadataPath(fileId), currentMetadata)) {
        applyLegacyDefaults(fileId, currentMetadata);
        seenVersionIds[currentMetadata.versionId] = versions.size();
        versions.push_back(std::move(currentMetadata));
    }

    const std::filesystem::path dir(versionMetadataDir(fileId));
    if (std::filesystem::exists(dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".meta") {
                continue;
            }

            FileMetadata metadata;
            if (!loadMetadataFromPath(entry.path().string(), metadata)) {
                continue;
            }
            applyLegacyDefaults(fileId, metadata);

            if (!metadata.versionId.empty() &&
                seenVersionIds.find(metadata.versionId) != seenVersionIds.end()) {
                continue;
            }

            seenVersionIds[metadata.versionId] = versions.size();
            versions.push_back(std::move(metadata));
        }
    }

    std::sort(versions.begin(), versions.end(), [](const FileMetadata& lhs, const FileMetadata& rhs) {
        if (lhs.createdAtEpochMs != rhs.createdAtEpochMs) {
            return lhs.createdAtEpochMs > rhs.createdAtEpochMs;
        }
        return lhs.versionId > rhs.versionId;
    });

    return versions;
}

std::vector<ChunkInfo> MetadataManager::loadChunks(const std::string &fileId)
{
    FileMetadata metadata;
    if (!loadMetadata(fileId, metadata)) {
        return {};
    }

    return metadata.chunks;
}

std::vector<std::string> MetadataManager::listFileIds()
{
    std::vector<std::string> fileIds;
    {
        std::lock_guard<std::mutex> lock(metadataIndexMutex);
        const auto& index = ensureMetadataIndexLocked(storageRoot_);
        fileIds.assign(index.fileIds.begin(), index.fileIds.end());
    }

    std::sort(fileIds.begin(), fileIds.end());
    return fileIds;
}

std::unordered_set<std::string> MetadataManager::collectReferencedChunkIds(
    const std::string& excludedFileId)
{
    std::unordered_set<std::string> referencedChunkIds;
    std::lock_guard<std::mutex> lock(metadataIndexMutex);
    const auto& index = ensureMetadataIndexLocked(storageRoot_);

    for (const auto& [fileId, chunkIds] : index.chunkIdsByFile) {
        if (!excludedFileId.empty() && fileId == excludedFileId) {
            continue;
        }

        referencedChunkIds.insert(chunkIds.begin(), chunkIds.end());
    }

    return referencedChunkIds;
}

bool MetadataManager::deleteMetadata(const std::string& fileId)
{
    const std::string path = metadataPath(fileId);
    const std::string versionsPath = versionMetadataDir(fileId);

    std::error_code ec;
    const bool removed = std::filesystem::remove(path, ec);

    if (ec) {
        return false;
    }

    std::error_code versionsEc;
    std::filesystem::remove_all(versionsPath, versionsEc);
    if (versionsEc) {
        return false;
    }

    std::lock_guard<std::mutex> lock(metadataCacheMutex);
    metadataCache.erase(cacheKeyFor(storageRoot_, fileId));
    {
        std::lock_guard<std::mutex> indexLock(metadataIndexMutex);
        auto& index = metadataIndexes[storageRoot_];
        if (index.loaded) {
            index.fileIds.erase(fileId);
            index.chunkIdsByFile.erase(fileId);
        }
    }
    return removed || !std::filesystem::exists(path);
}

// ================= CLEANUP TEMP FILES =================
void MetadataManager::cleanupTempFiles()
{
    const std::string dir = metadataDir();

    if (!std::filesystem::exists(dir)) return;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
        std::string path = entry.path().string();

        if (!entry.is_regular_file()) {
            continue;
        }

        if (path.size() >= 4 && path.substr(path.size() - 4) == ".tmp") {
            std::remove(path.c_str());
        }
    }
}
