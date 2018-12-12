/*
    Sample program from https://docs.opencv.org/4.0.0/db/df5/tutorial_linux_gcc_cmake.html
*/

#include <stdio.h>
//#include <opencv2/opencv.hpp> // opencv installation put all my headers under /usr/local/include/opencv4. See Makefile.

#include "LOG.h"
#include "VideoCHOP.h"

int main(int argc, char** argv )
{
    // Parse arguments
    if (argc != 3 && argc != 4)
    {
        LOG << "Usage: ./ProgramName /path/to/filename.avi /path/to/timestamps.txt /path/to/destinationfolder(optional)\n"<<argc<<" arguments provided.\n";
        return 1;
    }

    std::string src = std::string(argv[1]);
    std::string times = std::string(argv[2]);
    std::string dst = "./";

    if(argc == 3)
    {
        dst = std::string(argv[3]);
    }

    VideoCHOP videochop;

    if (!videochop.chop(src, times, dst))
    {
        return 1;
    }

    return 0;
}