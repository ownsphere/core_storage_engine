#pragma once
#include <vector>
#include <string>

class ChunkManager {
public:
    explicit ChunkManager(std::string storageRoot = "data");

    bool writeChunk(const std::string& chunkId, const std::vector<char>& data);
    std::vector<char> readChunk(const std::string& chunkId);
    bool deleteChunk(const std::string& chunkId);

    std::string chunksDir() const;

private:
    std::string storageRoot_;
};
