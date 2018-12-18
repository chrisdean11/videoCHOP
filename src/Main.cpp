/*
    Sample program from https://docs.opencv.org/4.0.0/db/df5/tutorial_linux_gcc_cmake.html
*/

#include <stdio.h>
#include <sstream>

#include "LOG.h"
#include "VideoCHOP.h"

int main(int argc, char** argv )
{
    VideoCHOP vc;

    // Parse arguments
    if(argc == 7 && std::string(argv[1]).compare("crop") == 0) // CROP
    {
        std::string srcFile = std::string(argv[2]);
        std::string dstFile = std::string(argv[3]);
        std::stringstream w,h,c;
        int width, height, color;
        w << argv[4];
        h << argv[5];
        c << std::hex << argv[6];
        w >> width;
        h >> height;
        c >> color; 

        if (!vc.crop(srcFile, width, height, color, dstFile))
        {
            return 1;
        }
    }
    else if (argc == 5 && std::string(argv[1]).compare("chop") == 0) // CHOP
    {
        std::string src = std::string(argv[2]);
        std::string times = std::string(argv[3]);
        std::string dst = std::string(argv[4]);
    
        if (!vc.chop(src, times, dst))
        {
            return 1;
        }
    }
    else
    {
        LOG << "Usage: ./ProgramName chop /path/to/filename.mp4 /path/to/timestamps.txt /path/to/destinationfolder\n";
        LOG << "Usage: ./ProgramName crop /path/to/filename.mp4 /path/to/destfile.mp4 width height color\n";
        return 1;
    }

    return 0;
}