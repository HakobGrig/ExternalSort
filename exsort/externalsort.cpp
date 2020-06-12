#include "externalsort.hpp"
#include "../utils/utils.hpp"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <experimental/filesystem>
//#include <iomanip> // for std::setw

namespace {
    template<typename T>
    struct SubBlockInfo {
        T* mFirstElement;
        T* mCurrentElement;
        T* mLastElement;
        uint32_t mCurrentPosInBlock;
        bool mIsFinished;
        bool mIsBlockFinishedInFile;
    };
/*
    template<typename T>
    void DumbBlock( T* first
                  , T* last) {

        uint32_t lineNumber = 0;
        std::cout << std::setw(3) << std::endl << std::endl << lineNumber++ << " : ";

        for (uint32_t i = 0; first <= last; ++i) {
            if (0 == (i % 16) && 0 != i) {
                std::cout << std::setw(3) << std::endl << lineNumber++ << " : ";
            }
            std::cout << *first++ << " ";
        }
        std::cout << std::endl << std::endl;
    }
*/
    uint32_t ReadBlock( std::fstream & file
                      , uint32_t blockNumber
                      , uint32_t blockSize
                      , char* buff) {
        file.seekg(uint64_t(blockNumber) * blockSize);
        file.read(buff, blockSize);
        return file.gcount();
    }

    uint32_t WriteBlock( std::fstream & file
                       , uint32_t blockSize
                       , uint32_t blockNumber
                       , char* buff
                       , uint32_t size) {
        file.seekg(uint64_t(blockNumber) * blockSize);
        file.write(buff, size);
        return file.gcount();
    }

    template <typename T>
    void ReadSubBlock( std::fstream & file
                     , uint32_t blockNumber
                     , uint32_t blockSize
                     , SubBlockInfo<T> &subBlockInfo
                     , uint32_t subBlockSize) {

        if (subBlockInfo.mIsBlockFinishedInFile) {
            subBlockInfo.mIsFinished = true;
            return;
        }
        uint32_t countToRead = subBlockSize;
        if (subBlockInfo.mCurrentPosInBlock + subBlockSize >= blockSize) {
            countToRead = blockSize - subBlockInfo.mCurrentPosInBlock;
            subBlockInfo.mIsBlockFinishedInFile = true;
        }

        file.seekg(uint64_t(blockNumber) * blockSize + subBlockInfo.mCurrentPosInBlock);
        file.read(reinterpret_cast<char*>(subBlockInfo.mFirstElement), countToRead);

        uint32_t read =  file.gcount();

        subBlockInfo.mCurrentPosInBlock += read;
        if (read != subBlockSize) {
            file.clear();
            subBlockInfo.mIsBlockFinishedInFile = true;

            subBlockInfo.mLastElement = subBlockInfo.mFirstElement + read / sizeof(T);
            if (subBlockInfo.mFirstElement == subBlockInfo.mLastElement) {
                subBlockInfo.mIsFinished = true;
            }
        }
        subBlockInfo.mCurrentElement = subBlockInfo.mFirstElement;
    }

    template<typename T>
    void ExternalSort_ ( std::atomic<bool> const & isActive
                    , std::string const & inputFilePath
                    , std::string const & outputFilePath
                    , uint64_t memorySize
                    , bool reverse) {

        std::fstream inFile(inputFilePath, std::ios::binary | std::ios::in);
        if (!inFile.is_open()) {
            std::cerr << "Unable to open input file..." << std::endl;
            return;
        }

        uint64_t inFileSize = std::experimental::filesystem::file_size(inputFilePath);

        // Calculate block size
        uint32_t blockSize  =  inFileSize > memorySize ? memorySize : inFileSize;

        // Align blockSize
        uint32_t mod = blockSize % sizeof(T);
        blockSize = (blockSize / sizeof(T)) * sizeof(T);
        if (mod) {
            if (blockSize + sizeof(T) <= memorySize) {
                blockSize += sizeof(T);
            }
        }

        std::cout << "MemorySize: " << memorySize << std::endl;
        std::cout << "FileSize: " << inFileSize << std::endl;
        std::cout << "BlockSize: " << blockSize << std::endl;

        // At least 2 samples should feet in to the block
        if (blockSize < sizeof(T) * 2) {
            throw std::runtime_error("Too low memory: BlockSize < (element size * 2)");
        }

        // Calculate block count
        uint32_t blockCount = inFileSize / blockSize;
        if (inFileSize % blockSize) {
            ++ blockCount;
        }

        std::cout << "BlockCount: " << blockCount << std::endl;

        uint32_t subBlockCount = blockCount;
        // keep 1 sub block as a resulting
        uint32_t subBlockSize = memorySize / (subBlockCount + 1);

        // Align sub block size
        subBlockSize = (subBlockSize / sizeof(T)) * sizeof(T);

        std::cout << "SubBlockSize: " << subBlockSize << std::endl;
        std::cout << "SubBlockCount: " << subBlockCount << std::endl;


        // At least 2 samples should feet in to the block
        if (subBlockSize < sizeof(T)) {
            throw std::runtime_error("Too low memory: subBlockSize < element size");
        }

        uint32_t subBlockSampleCount = subBlockSize / sizeof(T);

        // Find resulting sub block size
        uint32_t resBlockSize = memorySize - (subBlockCount * subBlockSize);

        // Align resulting sub block Size
        resBlockSize = (resBlockSize / sizeof(T)) * sizeof(T);

        std::cout << "ResBlockSize: " << resBlockSize << std::endl;

        uint32_t resSubBlockSampleCount = resBlockSize / sizeof(T);

        // Memory allocation
        std::shared_ptr<T[]> buff;
        uint32_t maxSampleCountInBuffer = memorySize / sizeof(T);

        try {
            buff.reset(new T[maxSampleCountInBuffer]);
        } catch (...) {
            std::cerr << "Unable to allocate memory..." << std::endl;
            return;
        }

        // craete temporary file
        const std::string tmpFilePath(outputFilePath + ".tmp");
        std::fstream tmpFile(tmpFilePath, std::ios::binary
                                        | std::ios::in
                                        | std::ios::out
                                        | std::ios::trunc);
        if (!inFile.is_open()) {
            std::cerr << "Unable to open tmp file..." << std::endl;
            return;
        }

        // --------------- Sort -------------------

        // Load each block, sort and write to temporary file
        for (uint32_t i = 0; i < blockCount && isActive; ++i) {
            uint32_t read = ReadBlock( inFile
                                    , i
                                    , blockSize
                                    , reinterpret_cast<char*>(buff.get()));

            uint32_t mod = read % sizeof(T);
            uint32_t sampleCountInBuffer = read / sizeof(T);
            if (mod) {
                ++sampleCountInBuffer;
            }

            T * firstElement = &buff[0];
            T * lastElement = &buff[sampleCountInBuffer - 1];

            // if the file is not aligned
            // with the given element size
            if (mod) {
                FixUnaligned(*lastElement, mod);
            }

            // Choose any memory optimized algorithm here
            // std::sort will call in-place sorting algorithm
            // in case of extra buffer isn't available
            // TODO: it will be nice to pass labmda function
            //       to monitor isActive
            if (!reverse) {
                std::sort( firstElement
                        , lastElement + 1
                        , std::greater<T>());
            } else {
            std::sort( firstElement
                        , lastElement + 1
                        , std::less<T>());
            }

            WriteBlock( tmpFile
                    , i
                    , blockSize
                    , reinterpret_cast<char*>(firstElement)
                    , sampleCountInBuffer * sizeof(T));
        }

        if (!isActive) {
            return;
        }

        // No need for merge
        if (1 == blockCount) {
            tmpFile.close();
            std::experimental::filesystem::rename(tmpFilePath, outputFilePath);
            return;
        }

        // --------------- Mege -------------------

        SubBlockInfo<T> subBlockInfo[subBlockCount];
        SubBlockInfo<T> resBlockInfo;

        // Initialize sub blocks subBlockCount
        for (uint32_t i = 0; i < subBlockCount && isActive; ++i) {
            subBlockInfo[i].mFirstElement = &buff[subBlockSampleCount*i];
            subBlockInfo[i].mLastElement = subBlockInfo[i].mFirstElement + subBlockSampleCount;
            subBlockInfo[i].mCurrentPosInBlock = 0;
            subBlockInfo[i].mIsFinished = false;
            subBlockInfo[i].mIsBlockFinishedInFile = false;
            ReadSubBlock( tmpFile
                        , i
                        , blockSize
                        , subBlockInfo[i]
                        , subBlockSize);
        }

        if (!isActive) {
            return;
        }
        // Initialize resulting block
        resBlockInfo.mFirstElement = subBlockInfo[subBlockCount-1].mLastElement; // &buff[subBlockSize*subBlockCount];
        resBlockInfo.mCurrentElement = resBlockInfo.mFirstElement;
        resBlockInfo.mLastElement = resBlockInfo.mFirstElement + resSubBlockSampleCount;
        resBlockInfo.mIsFinished = false;

        // open output file
        std::fstream outFile(outputFilePath, std::ios::binary
                                        | std::ios::out
                                        | std::ios::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Unable to open output file..." << std::endl;
            return;
        }

        while(isActive) {
            T element = 0;
            bool isFound = false;
            uint32_t elementsSubBlockNumber = -1;

            // Find required element
            for (uint32_t i = 0; i < subBlockCount && isActive; ++i) {
                if (!subBlockInfo[i].mIsFinished) {
                    if (!isFound) {
                        isFound = true;
                        element = *subBlockInfo[i].mCurrentElement;
                        elementsSubBlockNumber = i;
                    } else if (  (!reverse && *subBlockInfo[i].mCurrentElement >  element)
                            || ( reverse && *subBlockInfo[i].mCurrentElement <  element)) {
                        element = *subBlockInfo[i].mCurrentElement;
                        elementsSubBlockNumber = i;
                    }
                }
            }
            if (isFound) {
                // put element in resulting block
                *resBlockInfo.mCurrentElement++ = element;

                // if resulting block is full
                if (resBlockInfo.mCurrentElement == resBlockInfo.mLastElement) {
                    outFile.write(reinterpret_cast<char*>(resBlockInfo.mFirstElement), resBlockSize);
                    resBlockInfo.mCurrentElement = resBlockInfo.mFirstElement;
                }

                ++subBlockInfo[elementsSubBlockNumber].mCurrentElement;
                // if sub block is empty
                if (subBlockInfo[elementsSubBlockNumber].mCurrentElement == subBlockInfo[elementsSubBlockNumber].mLastElement) {
                    ReadSubBlock( tmpFile
                                , elementsSubBlockNumber
                                , blockSize
                                , subBlockInfo[elementsSubBlockNumber]
                                , subBlockSize);

                }
            } else {
                // flush resulting block
                outFile.write(reinterpret_cast<char*>(resBlockInfo.mFirstElement), ( resBlockInfo.mCurrentElement
                                                                                - resBlockInfo.mFirstElement)
                                                                                * sizeof(T));
                break;
            }
        }
        std::experimental::filesystem::remove(tmpFilePath);
    }
}; // anonymous namespace

void ExternalSort ( std::atomic<bool> const & isActive
                  , std::string const & inputFilePath
                  , std::string const & outputFilePath
                  , uint64_t memorySize
                  , uint32_t elementSize
                  , bool reverse) {

    switch(elementSize) {
        case 1:
            ExternalSort_<int8_t>( isActive
                                 , inputFilePath
                                 , outputFilePath
                                 , memorySize
                                 , reverse);
            break;
       case 2:
            ExternalSort_<int16_t>( isActive
                                  , inputFilePath
                                  , outputFilePath
                                  , memorySize
                                  , reverse);
            break;
       case 4:
            ExternalSort_<int32_t>( isActive
                                  , inputFilePath
                                  , outputFilePath
                                  , memorySize
                                  , reverse);
            break;
       case 8:
            ExternalSort_<int64_t>( isActive
                                  , inputFilePath
                                  , outputFilePath
                                  , memorySize
                                  , reverse);
            break;
       default:
            throw std::runtime_error("Invalid element size...");
        break;
    };
}
