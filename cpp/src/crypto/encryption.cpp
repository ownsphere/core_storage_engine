#include "encryption.h"

#include <array>
#include <istream>
#include <memory>
#include <ostream>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

namespace {

constexpr size_t kAes256KeySize = 32;
constexpr size_t kAesBlockSize = 16;

std::array<unsigned char, kAes256KeySize> deriveKey(const std::string& keyMaterial) {
    std::array<unsigned char, kAes256KeySize> key{};
    SHA256(reinterpret_cast<const unsigned char*>(keyMaterial.data()),
           keyMaterial.size(),
           key.data());
    return key;
}

using EvpCipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

EvpCipherContext createCipherContext() {
    return EvpCipherContext(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free);
}

}  // namespace

std::vector<char> encryptData(const std::vector<char>& data, const std::string& key) {
    if (data.empty()) {
        return {};
    }

    const auto derivedKey = deriveKey(key);

    std::array<unsigned char, kAesBlockSize> iv{};
    if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
        return {};
    }

    auto context = createCipherContext();
    if (!context) {
        return {};
    }

    if (EVP_EncryptInit_ex(context.get(), EVP_aes_256_cbc(), nullptr, derivedKey.data(), iv.data()) != 1) {
        return {};
    }

    std::vector<char> ciphertext(data.size() + kAesBlockSize);
    int bytesWritten = 0;
    if (EVP_EncryptUpdate(context.get(),
                          reinterpret_cast<unsigned char*>(ciphertext.data()),
                          &bytesWritten,
                          reinterpret_cast<const unsigned char*>(data.data()),
                          static_cast<int>(data.size())) != 1) {
        return {};
    }

    int finalBytes = 0;
    if (EVP_EncryptFinal_ex(context.get(),
                            reinterpret_cast<unsigned char*>(ciphertext.data()) + bytesWritten,
                            &finalBytes) != 1) {
        return {};
    }

    ciphertext.resize(static_cast<size_t>(bytesWritten + finalBytes));

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

    auto context = createCipherContext();
    if (!context) {
        return {};
    }

    if (EVP_DecryptInit_ex(context.get(),
                           EVP_aes_256_cbc(),
                           nullptr,
                           derivedKey.data(),
                           reinterpret_cast<const unsigned char*>(iv)) != 1) {
        return {};
    }

    std::vector<char> plaintext(ciphertextSize);
    int bytesWritten = 0;
    if (EVP_DecryptUpdate(context.get(),
                          reinterpret_cast<unsigned char*>(plaintext.data()),
                          &bytesWritten,
                          reinterpret_cast<const unsigned char*>(ciphertext),
                          static_cast<int>(ciphertextSize)) != 1) {
        return {};
    }

    int finalBytes = 0;
    if (EVP_DecryptFinal_ex(context.get(),
                            reinterpret_cast<unsigned char*>(plaintext.data()) + bytesWritten,
                            &finalBytes) != 1) {
        return {};
    }

    plaintext.resize(static_cast<size_t>(bytesWritten + finalBytes));
    return plaintext;
}

bool decryptStream(std::istream& input,
                   std::ostream& output,
                   const std::string& key,
                   ChecksumState* checksum) {
    std::array<unsigned char, kAesBlockSize> iv{};
    input.read(reinterpret_cast<char*>(iv.data()), static_cast<std::streamsize>(iv.size()));
    if (input.gcount() != static_cast<std::streamsize>(iv.size())) {
        return false;
    }

    const auto derivedKey = deriveKey(key);
    auto context = createCipherContext();
    if (!context) {
        return false;
    }

    if (EVP_DecryptInit_ex(context.get(), EVP_aes_256_cbc(), nullptr, derivedKey.data(), iv.data()) != 1) {
        return false;
    }

    std::array<char, 64 * 1024> encryptedBuffer{};
    std::array<char, (64 * 1024) + kAesBlockSize> plaintextBuffer{};

    while (input) {
        input.read(encryptedBuffer.data(), static_cast<std::streamsize>(encryptedBuffer.size()));
        const std::streamsize bytesRead = input.gcount();
        if (bytesRead <= 0) {
            break;
        }

        int bytesWritten = 0;
        if (EVP_DecryptUpdate(
                context.get(),
                reinterpret_cast<unsigned char*>(plaintextBuffer.data()),
                &bytesWritten,
                reinterpret_cast<const unsigned char*>(encryptedBuffer.data()),
                static_cast<int>(bytesRead)) != 1) {
            return false;
        }

        if (bytesWritten > 0) {
            if (checksum != nullptr) {
                checksum->update(plaintextBuffer.data(), static_cast<size_t>(bytesWritten));
            }

            output.write(plaintextBuffer.data(), bytesWritten);
            if (!output) {
                return false;
            }
        }
    }

    if (input.bad()) {
        return false;
    }

    int finalBytes = 0;
    if (EVP_DecryptFinal_ex(
            context.get(),
            reinterpret_cast<unsigned char*>(plaintextBuffer.data()),
            &finalBytes) != 1) {
        return false;
    }

    if (finalBytes > 0) {
        if (checksum != nullptr) {
            checksum->update(plaintextBuffer.data(), static_cast<size_t>(finalBytes));
        }

        output.write(plaintextBuffer.data(), finalBytes);
        if (!output) {
            return false;
        }
    }

    return true;
}
