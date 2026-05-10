#pragma once
#include <memory>
#include <openssl/evp.h>

#include <vector>
#include <string>

class ChecksumState {
public:
    ChecksumState();
    ~ChecksumState();

    ChecksumState(ChecksumState&& other) noexcept;
    ChecksumState& operator=(ChecksumState&& other) noexcept;

    ChecksumState(const ChecksumState&) = delete;
    ChecksumState& operator=(const ChecksumState&) = delete;

    void update(const char* data, size_t size);
    void update(const std::vector<char>& data);
    std::string finalize();

private:
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context_;
    bool finalized_;
};

std::string computeChecksum(const std::vector<char>& data);
