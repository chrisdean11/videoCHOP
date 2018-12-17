/*
    VideoCHOP.h

    
*/

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class VideoCHOP
{
public:
    VideoCHOP() {}
    bool chop(std::string video, std::string filename, std::string dest);
    bool crop(std::string video, int width, int height, int color, std::string dest);

private:

};