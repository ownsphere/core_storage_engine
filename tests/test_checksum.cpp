#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "checksum.h"

// ================= BASIC TESTS =================

// Same input → same output
TEST(ChecksumTest, Deterministic) {
    std::vector<char> data = {'a', 'b', 'c'};

    std::string hash1 = computeChecksum(data);
    std::string hash2 = computeChecksum(data);

    EXPECT_EQ(hash1, hash2);
}

// Different input → different output
TEST(ChecksumTest, DifferentInput) {
    std::vector<char> data1 = {'a', 'b', 'c'};
    std::vector<char> data2 = {'a', 'b', 'd'};

    EXPECT_NE(computeChecksum(data1), computeChecksum(data2));
}

// ================= EDGE CASES =================

// Empty input
TEST(ChecksumTest, EmptyData) {
    std::vector<char> data;

    std::string hash = computeChecksum(data);

    // Should not crash and should return a valid string
    EXPECT_FALSE(hash.empty());
}

// Single character
TEST(ChecksumTest, SingleCharacter) {
    std::vector<char> data = {'x'};

    std::string hash = computeChecksum(data);

    EXPECT_FALSE(hash.empty());
}

// ================= STABILITY =================

// Same large input → same output
TEST(ChecksumTest, LargeDataDeterministic) {
    std::vector<char> data(10000, 'A');

    std::string hash1 = computeChecksum(data);
    std::string hash2 = computeChecksum(data);

    EXPECT_EQ(hash1, hash2);
}

// ================= SENSITIVITY =================

// Small change → different hash
TEST(ChecksumTest, SmallChangeChangesHash) {
    std::vector<char> data1 = {'H','e','l','l','o'};
    std::vector<char> data2 = {'H','e','l','l','o','!'};

    EXPECT_NE(computeChecksum(data1), computeChecksum(data2));
}

// ================= BINARY DATA =================

// Works with non-text data
TEST(ChecksumTest, BinaryData) {
    std::vector<char> data = {0, 1, 2, 3, 4, 5, 6};

    std::string hash = computeChecksum(data);

    EXPECT_FALSE(hash.empty());
}