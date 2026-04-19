#include "checksum.h"
#include <sstream>

std::string computeChecksum(const std::vector<char>& data) {
    unsigned long hash = 5381;

    for (char c : data) {
        hash = ((hash << 5) + hash) + c; // djb2
    }

    std::stringstream ss;
    ss << hash;
    return ss.str();
}