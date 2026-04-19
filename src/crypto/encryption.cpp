#include "encryption.h"

std::vector<char> encryptData(const std::vector<char>& data, const std::string& key) {
    std::vector<char> encrypted = data;

    for (size_t i = 0; i < data.size(); i++) {
        encrypted[i] = data[i] ^ key[i % key.size()];
    }

    return encrypted;
}

std::vector<char> decryptData(const std::vector<char>& data, const std::string& key) {
    // XOR is symmetric
    return encryptData(data, key);
}