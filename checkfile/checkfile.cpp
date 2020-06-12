#include "checkfile.hpp"
#include "../utils/utils.hpp"

#include <fstream>
#include <iostream>

namespace {
	int64_t readNextElement( std::fstream & inFile
			       		   , uint32_t elementSize) {
	    int64_t element = 0;
	    inFile.read(reinterpret_cast<char*>(&element), elementSize);
	    int read = inFile.gcount();

		if (read && read != sizeof(int64_t)) {
			FixUnaligned(element, read);
		}
		return element;
	}
};

bool CheckSortedFile( std::atomic<bool> const & isActive
		   			, std::string const & filePath
		   			, uint32_t elementSize
		   			, bool reverse) {
    std::fstream inFile(filePath, std::ios::binary | std::ios::in);
    if (!inFile.is_open()) {
        throw std::runtime_error("Unable to open file...");
    }
	int64_t magic = 0;

    int64_t sample = readNextElement(inFile, elementSize);
	magic ^= sample;

	bool ret = true;
    while (isActive ) {
	    int64_t nextSample = readNextElement(inFile, elementSize);
		 if (!inFile.eof()) {
			magic ^= nextSample;
			if (ret) {
				if (!reverse) {
					if (sample < nextSample) {
						ret = false;
					}
				} else {
					if (sample > nextSample) {
						ret = false;
					}
				}
				sample = nextSample;
			}
		} else {
			break;
		}
    }
	std::cout << "Magic: " << magic << std::endl;
    return ret;
}
