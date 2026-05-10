#include <gtest/gtest.h>

#include <sstream>
#include <vector>
#include <string>

#include "encryption.h"

namespace {

std::vector<char> toVector(const std::string& text) {
    return std::vector<char>(text.begin(), text.end());
}

}  // namespace

TEST(EncryptionTest, RoundTripRestoresOriginalPlaintext) {
    const std::vector<char> plaintext = toVector("Hello AES-256");
    const std::string key = "mysecretkey";

    const auto encrypted = encryptData(plaintext, key);
    const auto decrypted = decryptData(encrypted, key);

    EXPECT_FALSE(encrypted.empty());
    EXPECT_NE(encrypted, plaintext);
    EXPECT_EQ(decrypted, plaintext);
}

TEST(EncryptionTest, RandomNonceProducesDifferentCiphertext) {
    const std::vector<char> plaintext = toVector("same plaintext");
    const std::string key = "mysecretkey";

    const auto encrypted1 = encryptData(plaintext, key);
    const auto encrypted2 = encryptData(plaintext, key);

    EXPECT_FALSE(encrypted1.empty());
    EXPECT_FALSE(encrypted2.empty());
    EXPECT_NE(encrypted1, encrypted2);
}

TEST(EncryptionTest, WrongKeyDoesNotRestorePlaintext) {
    const std::vector<char> plaintext = toVector("sensitive chunk");
    const std::string key = "mysecretkey";
    const std::string wrongKey = "not-the-right-key";

    const auto encrypted = encryptData(plaintext, key);
    const auto decrypted = decryptData(encrypted, wrongKey);

    EXPECT_NE(decrypted, plaintext);
}

TEST(EncryptionTest, StreamDecryptRestoresOriginalPlaintextAndChecksum) {
    const std::vector<char> plaintext(70000, 'Q');
    const std::string key = "mysecretkey";
    const auto encrypted = encryptData(plaintext, key);

    std::istringstream input(std::string(encrypted.begin(), encrypted.end()), std::ios::binary);
    std::ostringstream output(std::ios::binary);
    ChecksumState checksum;

    ASSERT_TRUE(decryptStream(input, output, key, &checksum));
    EXPECT_EQ(output.str(), std::string(plaintext.begin(), plaintext.end()));
    EXPECT_EQ(checksum.finalize(), computeChecksum(plaintext));
}
