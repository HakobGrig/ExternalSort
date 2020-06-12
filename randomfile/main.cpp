#include <iostream>
#include <atomic>
#include <string>

#include <signal.h>

#include <unistd.h> // For getopt

#include "randomfile.hpp"

// Global variables
static std::atomic<bool> gIsActive(true);

void signal_handler(int sig) {
    std::cerr << "Got a signal, exitting..." << std::endl;
    gIsActive.store(false);
}

void PrintUsageAndExit() {
    std::cout << "Usage: " << std::endl;
    std::cout << "randomfile { -h | <-o outputFilePath> <-s fileSize>}" << std::endl;
    std::cout << "  -h \t print this usage and exit" << std::endl;
    std::cout << "  -o \t file path"<< std::endl;
    std::cout << "  -s \t file size in bytes"<< std::endl;
    exit(-1);
}

int main(int argc, char **argv)
{
    // Parse input arguments
    std::string outputFilePath;
    uint64_t fileSize = 0;
    int c;
    while ((c = getopt (argc, argv, "ho:s:")) != -1) {
        switch (c)
        {
            case 'o':
                outputFilePath = std::string (optarg);
                break;
            case 's':
                fileSize = std::stoll (optarg);
                break;
            case 'h':
            case '?':
                PrintUsageAndExit();
        }
    }
    if (  outputFilePath.empty()
       || 0 == fileSize) {
        PrintUsageAndExit();
    }

    // Register signal handler
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    try {
	    CreateRandomFile( gIsActive
			            , outputFilePath
			            , fileSize);
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(-1);
    }
}
