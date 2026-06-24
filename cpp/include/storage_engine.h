#pragma once
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>  
#include <functional>

#include "models/chunk_info.h"
#include "metadata_manager.h"

using ProgressCallback = std::function<void(int)>;

struct StoreFileOptions {
    std::string originalFilename;
    std::string extension;
    std::string contentType;
    std::string checksum;
    std::int64_t uploadedAtEpochMs = 0;
};

class StorageEngine {
public:
    explicit StorageEngine(std::string storageRoot = "data");
    ~StorageEngine();

    StorageEngine(const StorageEngine&) = delete;
    StorageEngine& operator=(const StorageEngine&) = delete;

    bool storeFile(const std::string& filePath,
                   const std::string& fileId,
                   ProgressCallback progressCallback = nullptr);
    bool storeFile(const std::string& filePath,
                   const std::string& fileId,
                   const StoreFileOptions& options,
                   ProgressCallback progressCallback = nullptr);

    bool retrieveFile(const std::string& fileId,
                      const std::string& outputPath,
                      ProgressCallback progressCallback = nullptr);
    bool getFileMetadata(const std::string& fileId, FileMetadata& metadata);

    std::vector<std::string> listFiles();
    bool deleteFile(const std::string& fileId);
    bool deleteAllFiles();

    int getProgress(const std::string& fileId);
    void waitForBackgroundTasks();

private:
    std::string metadataPath(const std::string& fileId) const;
    std::string metadataDir() const;
    void startBackgroundMaintenance();
    void runBackgroundMaintenance();

    std::string storageRoot_;
    std::mutex maintenanceMutex_;
    std::thread maintenanceThread_;
};
