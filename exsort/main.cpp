#include <iostream>
#include <atomic>
#include <string>

#include <signal.h>

#include <unistd.h> // For getopt

#include "externalsort.hpp"

// Global variables
static std::atomic<bool> gIsActive(true);
static std::string gInputFilePath;
static std::string gOutputFilePath("/tmp/exsort.out");
static uint64_t gAvailableRamSize(1024);
static uint32_t gElementSize(4);
static bool gReverse(false);

void signal_handler(int sig) {
    std::cerr << "Got a signal, exitting..." << std::endl;
    gIsActive.store(false);
}

void PrintUsageAndExit() {
    std::cout << "Usage: " << std::endl;
    std::cout << "exsort { -h | <-i inputFilePath> [-o outputFilePath] [-s ramSize] [-e elSize] [-r]}" << std::endl;
    std::cout << "  -h \t print this usage and exit" << std::endl;
    std::cout << "  -i \t input file path"<< std::endl;
    std::cout << "  -o \t output file path, default is: " << gOutputFilePath << std::endl;
    std::cout << "  -s \t available ram size to use for sorting in bytes, defult is: " << gAvailableRamSize << std::endl;
    std::cout << "  -e \t element size in bytes, range [1, 8], should be pow of 2, defult is: " << gElementSize << std::endl;
    std::cout << "  -r \t reverse sort, default is increasing order." << std::endl;
    exit(-1);
}

int main(int argc, char **argv)
{
    // Parse input arguments
    int c;
    while ((c = getopt (argc, argv, "hi:o:s:e:r")) != -1) {
        switch (c)
        {
            case 'i':
                gInputFilePath = std::string (optarg);
                break;
            case 'o':
                gOutputFilePath = std::string (optarg);
                break;
            case 's':
                gAvailableRamSize = std::stoll (optarg);
                break;
            case 'e':
                gElementSize = std::stoi (optarg);
                if (  gElementSize < 1
                   || gElementSize > 8
                   || (gElementSize & (gElementSize - 1)) != 0) {
                       PrintUsageAndExit();
                   }
                break;
            case 'r':
		gReverse = true;
                break;
            case 'h':
            case '?':
                PrintUsageAndExit();
        }
    }
    if (gInputFilePath.empty()) {
        PrintUsageAndExit();
    }

    // Register signal handler
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    try {
	    ExternalSort( gIsActive
			        , gInputFilePath
			        , gOutputFilePath
			        , gAvailableRamSize
			        , gElementSize
			        , gReverse);
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(-1);
    }
}
