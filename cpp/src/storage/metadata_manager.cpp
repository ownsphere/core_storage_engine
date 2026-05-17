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

std::string serializeMetadata(const FileMetadata& metadata) {
    std::ostringstream content;
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

bool loadMetadataFromPath(const std::string& path, FileMetadata& metadata) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    return parseMetadata(in, metadata);
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
bool MetadataManager::saveMetadata(const std::string& fileId,
                                   const std::vector<ChunkInfo>& chunks,
                                   size_t fileSize)
{
    const std::string dir = metadataDir();
    const std::string versionsDir = versionMetadataDir(fileId);
    std::filesystem::create_directories(dir);
    std::filesystem::create_directories(versionsDir);

    const std::string finalPath = metadataPath(fileId);
    const std::string tempPath = finalPath + ".tmp";

    FileMetadata previousMetadata;
    const bool hadPrevious = loadMetadataFromPath(finalPath, previousMetadata);

    FileMetadata metadata;
    metadata.versionId = createVersionId();
    metadata.previousVersionId = hadPrevious ? previousMetadata.versionId : "";
    metadata.createdAtEpochMs = currentEpochMillis();
    metadata.fileSize = fileSize;
    metadata.chunks = chunks;

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

    std::lock_guard<std::mutex> lock(metadataCacheMutex);
    metadataCache[cacheKeyFor(storageRoot_, fileId)] = std::move(metadata);
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
    return loadMetadataFromPath(versionMetadataPath(fileId, versionId), metadata);
}

std::vector<FileMetadata> MetadataManager::listMetadataVersions(const std::string& fileId)
{
    std::vector<FileMetadata> versions;
    std::unordered_map<std::string, size_t> seenVersionIds;

    FileMetadata currentMetadata;
    if (loadMetadataFromPath(metadataPath(fileId), currentMetadata)) {
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
