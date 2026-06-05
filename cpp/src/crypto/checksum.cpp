#include "checksum.h"

#include <iomanip>
#include <sstream>

ChecksumState::ChecksumState()
    : context_(EVP_MD_CTX_new(), &EVP_MD_CTX_free),
      finalized_(false) {
    if (context_) {
        EVP_DigestInit_ex(context_.get(), EVP_sha256(), nullptr);
    }
}

ChecksumState::~ChecksumState() = default;

ChecksumState::ChecksumState(ChecksumState&& other) noexcept = default;

ChecksumState& ChecksumState::operator=(ChecksumState&& other) noexcept = default;

void ChecksumState::update(const char* data, size_t size) {
    if (finalized_ || !context_ || size == 0) {
        return;
    }

    EVP_DigestUpdate(context_.get(), data, size);
}

void ChecksumState::update(const std::vector<char>& data) {
    update(data.data(), data.size());
}

std::string ChecksumState::finalize() {
    if (finalized_ || !context_) {
        return "";
    }

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLength = 0;
    if (EVP_DigestFinal_ex(context_.get(), digest, &digestLength) != 1) {
        finalized_ = true;
        return "";
    }
    finalized_ = true;

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < digestLength; ++i) {
        const unsigned char byte = digest[i];
        ss << std::setw(2) << static_cast<int>(byte);
    }

    return ss.str();
}

std::string computeChecksum(const std::vector<char>& data) {
    ChecksumState checksum;
    checksum.update(data);
    return checksum.finalize();
}
