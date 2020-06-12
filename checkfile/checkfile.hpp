#ifndef _RANDOM_FILE_
#define _RANDOM_FILE_

#include <atomic>
#include <string>

bool CheckSortedFile( std::atomic<bool> const & isActive
		    		, std::string const & filePath
		    		, uint32_t elementSize
		    		, bool reverse);

#endif
