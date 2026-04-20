#include "encryption.h"

#include <array>
#include <cstring>

#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonRandom.h>

namespace {

constexpr size_t kAes256KeySize = 32;
constexpr size_t kAesBlockSize = kCCBlockSizeAES128;

std::array<unsigned char, kAes256KeySize> deriveKey(const std::string& keyMaterial) {
    std::array<unsigned char, kAes256KeySize> key{};
    CC_SHA256(reinterpret_cast<const unsigned char*>(keyMaterial.data()),
              static_cast<CC_LONG>(keyMaterial.size()),
              key.data());
    return key;
}

}  // namespace

std::vector<char> encryptData(const std::vector<char>& data, const std::string& key) {
    if (data.empty()) {
        return {};
    }

    const auto derivedKey = deriveKey(key);

    std::array<unsigned char, kAesBlockSize> iv{};
    if (CCRandomGenerateBytes(iv.data(), iv.size()) != kCCSuccess) {
        return {};
    }

    std::vector<char> ciphertext(data.size() + kAesBlockSize);
    size_t producedBytes = 0;
    const CCCryptorStatus status = CCCrypt(kCCEncrypt,
                                           kCCAlgorithmAES,
                                           kCCOptionPKCS7Padding,
                                           derivedKey.data(),
                                           derivedKey.size(),
                                           iv.data(),
                                           data.data(),
                                           data.size(),
                                           ciphertext.data(),
                                           ciphertext.size(),
                                           &producedBytes);
    if (status != kCCSuccess) {
        return {};
    }

    ciphertext.resize(producedBytes);

    std::vector<char> encrypted;
    encrypted.reserve(iv.size() + ciphertext.size());
    encrypted.insert(encrypted.end(), reinterpret_cast<const char*>(iv.data()), reinterpret_cast<const char*>(iv.data() + iv.size()));
    encrypted.insert(encrypted.end(), ciphertext.begin(), ciphertext.end());
    return encrypted;
}

std::vector<char> decryptData(const std::vector<char>& data, const std::string& key) {
    if (data.size() < kAesBlockSize) {
        return {};
    }

    const auto derivedKey = deriveKey(key);
    const void* iv = data.data();
    const void* ciphertext = data.data() + kAesBlockSize;
    const size_t ciphertextSize = data.size() - kAesBlockSize;

    std::vector<char> plaintext(ciphertextSize);
    size_t producedBytes = 0;
    const CCCryptorStatus status = CCCrypt(kCCDecrypt,
                                           kCCAlgorithmAES,
                                           kCCOptionPKCS7Padding,
                                           derivedKey.data(),
                                           derivedKey.size(),
                                           iv,
                                           ciphertext,
                                           ciphertextSize,
                                           plaintext.data(),
                                           plaintext.size(),
                                           &producedBytes);
    if (status != kCCSuccess) {
        return {};
    }

    plaintext.resize(producedBytes);
    return plaintext;
}
