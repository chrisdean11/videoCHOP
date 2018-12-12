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

private:
    struct timeVal
    {
        int m;
        int s;
        int ms;
        int m2;
        int s2;
        int ms2;
    
        std::string toString()
        {
            std::stringstream ss;
            ss << "m:s:ms,m2:s2:ms2=" << m << ":" << s << ":" << ms << "," << m2 << ":" << s2 << ":" << ms2 << "\n";
            return ss.str();
        }
    };

    bool getTimes(std::string filename, std::vector<timeVal> &times);
    //bool getFrames(std::vector<Mat> frames, const std::string videoname, int &codec, double &fps, Size &size);
};