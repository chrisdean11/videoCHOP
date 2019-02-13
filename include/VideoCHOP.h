/*
    VideoCHOP.h
    
*/

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>     // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/imgproc.hpp>  // Gaussian Blur
#include <opencv2/highgui.hpp>  // OpenCV window I/O
#include <opencv2/videoio.hpp>  // Video write

class VideoCHOP
{
public:
    VideoCHOP() {}
    bool chop(std::string video, std::string filename, std::string dest);
    bool crop(std::string video, int width, int height, std::string dest, int speed, std::string method = "mosse");

    std::string method;

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

    bool getTimesFromFile(std::string filename, std::vector<timeVal> &times);
    bool getAllFramesFromVideo(std::vector<cv::Mat> &frames, const std::string &videoname, int &codec, double &fps, cv::Size &size);
    cv::Point findObject(const cv::Mat &mat);
};