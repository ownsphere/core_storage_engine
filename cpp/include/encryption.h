#pragma once
#include <vector>
#include <string>
#include <iosfwd>

#include "checksum.h"

std::vector<char> encryptData(const std::vector<char>& data, const std::string& key);
std::vector<char> decryptData(const std::vector<char>& data, const std::string& key);
bool decryptStream(std::istream& input,
                   std::ostream& output,
                   const std::string& key,
                   ChecksumState* checksum = nullptr);
