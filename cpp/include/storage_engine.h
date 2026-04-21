#pragma once
#include <string>
#include <vector>  
#include <functional>

#include "models/chunk_info.h"

using ProgressCallback = std::function<void(int)>;

// ✅ ADD THIS
class StorageEngine {
public:
    bool storeFile(const std::string& filePath,
                   const std::string& fileId,
                   ProgressCallback progressCallback = nullptr);

    bool retrieveFile(const std::string& fileId,
                      const std::string& outputPath,
                      ProgressCallback progressCallback = nullptr);

    std::vector<std::string> listFiles();
    bool deleteFile(const std::string& fileId);

    int getProgress(const std::string& fileId);
};