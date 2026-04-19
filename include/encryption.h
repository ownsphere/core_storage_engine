#pragma once
#include <vector>
#include <string>

std::vector<char> encryptData(const std::vector<char>& data, const std::string& key);
std::vector<char> decryptData(const std::vector<char>& data, const std::string& key);