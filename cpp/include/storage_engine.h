#pragma once
#include <mutex>
#include <string>
#include <thread>
#include <vector>  
#include <functional>

#include "models/chunk_info.h"

using ProgressCallback = std::function<void(int)>;

// ✅ ADD THIS
class StorageEngine {
public:
    explicit StorageEngine(std::string storageRoot = "data");
    ~StorageEngine();

    StorageEngine(const StorageEngine&) = delete;
    StorageEngine& operator=(const StorageEngine&) = delete;

    bool storeFile(const std::string& filePath,
                   const std::string& fileId,
                   ProgressCallback progressCallback = nullptr);

    bool retrieveFile(const std::string& fileId,
                      const std::string& outputPath,
                      ProgressCallback progressCallback = nullptr);

    std::vector<std::string> listFiles();
    bool deleteFile(const std::string& fileId);

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
