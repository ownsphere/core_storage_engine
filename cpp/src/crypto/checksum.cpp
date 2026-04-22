#include "checksum.h"

#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>

namespace {

constexpr std::array<uint32_t, 64> kRoundConstants = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

constexpr std::array<uint32_t, 8> kInitialHash = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

uint32_t rotateRight(uint32_t value, uint32_t shift) {
    return (value >> shift) | (value << (32 - shift));
}

uint32_t choose(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

uint32_t majority(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

uint32_t bigSigma0(uint32_t value) {
    return rotateRight(value, 2) ^ rotateRight(value, 13) ^ rotateRight(value, 22);
}

uint32_t bigSigma1(uint32_t value) {
    return rotateRight(value, 6) ^ rotateRight(value, 11) ^ rotateRight(value, 25);
}

uint32_t smallSigma0(uint32_t value) {
    return rotateRight(value, 7) ^ rotateRight(value, 18) ^ (value >> 3);
}

uint32_t smallSigma1(uint32_t value) {
    return rotateRight(value, 17) ^ rotateRight(value, 19) ^ (value >> 10);
}

std::vector<unsigned char> padMessage(const std::vector<char>& data) {
    std::vector<unsigned char> padded(data.begin(), data.end());
    const uint64_t bitLength = static_cast<uint64_t>(padded.size()) * 8;

    padded.push_back(0x80);

    while ((padded.size() % 64) != 56) {
        padded.push_back(0x00);
    }

    for (int shift = 56; shift >= 0; shift -= 8) {
        padded.push_back(static_cast<unsigned char>((bitLength >> shift) & 0xff));
    }

    return padded;
}

}  // namespace

std::string computeChecksum(const std::vector<char>& data) {
    std::array<uint32_t, 8> hash = kInitialHash;
    const std::vector<unsigned char> padded = padMessage(data);

    for (size_t offset = 0; offset < padded.size(); offset += 64) {
        std::array<uint32_t, 64> schedule{};

        for (size_t i = 0; i < 16; ++i) {
            const size_t index = offset + (i * 4);
            schedule[i] = (static_cast<uint32_t>(padded[index]) << 24) |
                          (static_cast<uint32_t>(padded[index + 1]) << 16) |
                          (static_cast<uint32_t>(padded[index + 2]) << 8) |
                          static_cast<uint32_t>(padded[index + 3]);
        }

        for (size_t i = 16; i < schedule.size(); ++i) {
            schedule[i] = smallSigma1(schedule[i - 2]) + schedule[i - 7] +
                          smallSigma0(schedule[i - 15]) + schedule[i - 16];
        }

        uint32_t a = hash[0];
        uint32_t b = hash[1];
        uint32_t c = hash[2];
        uint32_t d = hash[3];
        uint32_t e = hash[4];
        uint32_t f = hash[5];
        uint32_t g = hash[6];
        uint32_t h = hash[7];

        for (size_t i = 0; i < schedule.size(); ++i) {
            const uint32_t temp1 = h + bigSigma1(e) + choose(e, f, g) + kRoundConstants[i] + schedule[i];
            const uint32_t temp2 = bigSigma0(a) + majority(a, b, c);

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
        hash[5] += f;
        hash[6] += g;
        hash[7] += h;
    }

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');

    for (uint32_t value : hash) {
        ss << std::setw(8) << value;
    }

    return ss.str();
}
