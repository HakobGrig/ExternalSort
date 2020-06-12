#include <iostream>
#include <atomic>
#include <string>

#include <signal.h>

#include <unistd.h> // For getopt

#include "checkfile.hpp"

// Global variables
static std::atomic<bool> gIsActive(true);

void signal_handler(int sig) {
    std::cerr << "Got a signal, exitting..." << std::endl;
    gIsActive.store(false);
}

void PrintUsageAndExit() {
    std::cout << "Usage: " << std::endl;
    std::cout << "checkfile {-h | <-i inputFilePath> <-e elementSize> [-r]}" << std::endl;
    std::cout << "  -h \t print this usage and exit" << std::endl;
    std::cout << "  -i \t file path"<< std::endl;
    std::cout << "  -e \t element size in bytes, range [1, 8], should be pow of 2" << std::endl;
    std::cout << "  -r \t reverse check, default is increasing order" << std::endl;
    exit(-1);
}

int main(int argc, char **argv)
{
    // Parse input arguments
    std::string filePath;
    uint32_t elementSize = 0;
    bool reverse = false;
    int c;
    while ((c = getopt (argc, argv, "hi:e:r")) != -1) {
        switch (c)
        {
            case 'i':
                filePath = std::string (optarg);
                break;
            case 'e':
                elementSize = std::stoi (optarg);
                if (  elementSize < 1
                   || elementSize > 8
                   || (elementSize & (elementSize - 1)) != 0) {
                       PrintUsageAndExit();
                   }
                break;
            case 'r':
		reverse = true;
                break;
            case 'h':
            case '?':
                PrintUsageAndExit();
        }
    }
    if (  filePath.empty()
       || 0 == elementSize) {
        PrintUsageAndExit();
    }

    // Register signal handler
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    try {
	    bool ret = CheckSortedFile( gIsActive
			              , filePath
			              , elementSize
			              , reverse);
	    if (ret) {
		    std::cerr << "Array is sorted." << std::endl;
	    } else {
		    std::cerr << "Array is not sorted." << std::endl;
	    }
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(-1);
    }
}
