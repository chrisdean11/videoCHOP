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
    bool crop(std::string video, Size s, int color, std::string dest);

private:
    struct timeVal
    {
        int m;
        int s;
        int m2;
        int s2;
    
        std::string toString()
        {
            std::stringstream ss;
            ss << "m:s,m2:s2=" << m << ":" << s << "," << m2 << ":" << s2 << "\n";
            return ss.str();
        }
    };

    bool getTimes(std::string filename, std::vector<timeVal> &times);
    //bool getFrames(std::vector<Mat> frames, const std::string videoname, int &codec, double &fps, Size &size);
};