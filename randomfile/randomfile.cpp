#include "randomfile.hpp"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>

void CreateRandomFile( std::atomic<bool> const & isActive
		             , std::string const & filePath
		             , uint64_t fileSize) {
    std::fstream outFile(filePath, std::ios::binary
                                 | std::ios::out
                                 | std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Unable to open file..." << std::endl;
        return;
    }

    std::srand(std::time(nullptr));

    uint64_t currentSize = 0;
    while (currentSize < fileSize && isActive.load()) {
        unsigned char tmp = std::rand() % 256;
        outFile.write(reinterpret_cast<char*>(&tmp), 1);
        currentSize += sizeof(char);
    }
    return;
}
