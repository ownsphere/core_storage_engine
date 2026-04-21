#include "chunk_manager.h"
#include <fstream>
#include <filesystem>
#include <iostream>

bool ChunkManager::writeChunk(const std::string& chunkId, const std::vector<char>& data) {
    std::filesystem::create_directories("data/chunks");
    std::ofstream out("data/chunks/" + chunkId, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "ERROR: Cannot write chunk: " << chunkId << std::endl;
        return false;
    }
    out.write(data.data(), data.size());
    return true;
}

std::vector<char> ChunkManager::readChunk(const std::string& chunkId) {
    std::ifstream in("data/chunks/" + chunkId, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "ERROR: Cannot open chunk: " << chunkId << std::endl;
        return {};
    }

    return std::vector<char>((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
}