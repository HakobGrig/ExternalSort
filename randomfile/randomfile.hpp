#ifndef _RANDOM_FILE_
#define _RANDOM_FILE_

#include <atomic>
#include <string>

void CreateRandomFile( std::atomic<bool> const & isActive
		     		 , std::string const & filePath
		     		 , uint64_t fileSize);

#endif
