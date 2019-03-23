/*
    Sample program demonstrating how the VideoCHOP class is used.
*/

#include <stdio.h>
#include <sstream>

#include "Log.h"
#include "VideoCHOP.h"

int main(int argc, char** argv )
{
    VideoCHOP vc;

    // Parse arguments
    if(argc >= 6 && std::string(argv[1]).compare("crop") == 0) // CROP
    {
        std::string srcFile = std::string(argv[2]);
        std::string dstFile = std::string(argv[3]);
        std::string method = "mosse";
        int width, height; 
        int speed = 1;
        bool dots = false;

        std::stringstream w, h, s;
        w << argv[4];
        h << argv[5];
        w >> width;
        h >> height;

        if (argc > 6)
        {
            s << argv[6];
            s >> speed;
        }

        if (argc > 7)
        {
            method = std::string(argv[7]);
        }

        if (argc > 8)
        {
            if (strcasecmp(argv[8],"on") == 0 || strcasecmp(argv[8],"1") == 0 || strcasecmp(argv[8],"true") == 0 )
            {
                dots = true;
            }
        }

        if (!vc.crop(srcFile, width, height, dstFile, speed, method, dots))
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
    else if (argc >= 5 && std::string(argv[1]).compare("slideshow") == 0)
    {
        std::string src = std::string(argv[2]);
        std::string dst = std::string(argv[3]);
        std::string imageFolder = std::string(argv[4]);

        // Score fit based on:
            // Raw image difference
            // Difference disregarding color saturation
            // Text analysis

        if (!vc.slideshow(src, dst, imageFolder))
        {
            Log::Log("slideshow returned false\n");
        }
    }
    else
    {
        Log::Log("Usage: ./videoCHOP chop /path/to/filename.mp4 /path/to/timestamps.txt /path/to/destinationfolder\n");
        Log::Log("Usage: ./videoCHOP crop /path/to/filename.mp4 /path/to/destfile.mp4 width height [speed] [method] [dots]\n");
        Log::Log("Usage: ./videoCHOP slideshow /path/to/filename.mp4 /path/to/destfile.mp4 /path/to/images/folder\n");
        return 1;
    }

    return 0;
}