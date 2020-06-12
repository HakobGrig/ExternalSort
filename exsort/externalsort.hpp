#ifndef _EXTERNAL_SORT_HPP_
#define _EXTERNAL_SORT_HPP_

#include <atomic>
#include <string>

void ExternalSort ( std::atomic<bool> const & isActive
                  , std::string const & inputFilePath
                  , std::string const & outputFilePath
                  , uint64_t memorySize
                  , uint32_t sampleSize
                  , bool reverse);

#endif