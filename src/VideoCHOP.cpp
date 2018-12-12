/*
    VideoCHOP.cpp

*/
#include <fstream>

#include <opencv2/core.hpp>     // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/imgproc.hpp>  // Gaussian Blur
#include <opencv2/highgui.hpp>  // OpenCV window I/O
#include <opencv2/videoio.hpp>  // Video write

#include "VideoCHOP.h"
#include "LOG.h"

using namespace cv;
static bool getFrames(std::vector<Mat> frames, const std::string videoname, int &codec, double &fps, Size &size);

bool VideoCHOP::chop(std::string videoname, std::string filename, std::string dest)
{
    bool success = true;
    std::vector<timeVal> times;
    int clipnum = 0;

    if (!getTimes(filename, times))
    {
        return false;
    }

    // Get video info and frames
    std::vector<Mat> frames;
    int codec;
    double fps;
    Size size;
    if (!getFrames(frames, videoname, codec, fps, size))
    {
        LOG << "heck\n";
        return false;
    }

    /*
     * Make clips
     */
    for (timeVal &time : times)
    {
        ++clipnum;

        // Make the new clip's name
        std::stringstream ss;
        ss << dest << clipnum << "_" << time.m << ":" << time.s << "_" << time.m2 << ":" << time.s2 << ".avi";
        std::string clipname = ss.str();

        // Make WideoWriter
        VideoWriter clip = VideoWriter(clipname, codec, fps, size, true);

        // Get the frame numbers for this clip
        int start = fps * ((60*time.m) + time.s + time.ms/60);
        int end = fps * ((60*time.m2) + time.s2 + time.ms/60);

        while (start <= end)
        {
            clip << frames[start];
        }

        // Destructor will handle the closing of the video file.
    }


    /*
     * Make clips
     */
/* Old method
    for (timeVal &time : times)
    {
        ++clipnum;
        VideoCapture vid = VideoCapture(videoname);

        // Make the new clip's name
        std::stringstream ss;
        ss << dest << clipnum << "_" << time.m << ":" << time.s << "_" << time.m2 << ":" << time.s2 << ".avi";
        std::string clipname = ss.str();

        if (!vid.isOpened())
        {
            LOG << "Error: " << videoname << " not opened successfully on making " << clipname << "\n";
            success = false;
            break;
        }

        // Get clip info: size, fourcc, fps
        Size S = Size( (int)vid.get(CAP_PROP_FRAME_WIDTH), (int)vid.get(CAP_PROP_FRAME_HEIGHT));
        int ex = static_cast<int>(vid.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
        double fps = vid.get(CAP_PROP_FPS);

        // Make WideoWriter
        VideoWriter clip = VideoWriter(clipname, ex, fps,S, true);

        // Get the frame numbers for this clip
        int start = fps * ((60*time.m) + time.s);
        int end = fps * ((60*time.m2) + time.s2);
        int current = 1;

        // Iterate and add the appropriate frames
        while(current < start) { Mat mat; vid >> mat; ++current; }
        while(current < end) { Mat mat; vid >> mat; clip << mat; ++current;}

        // Destructor will handle closing the video file.
    }
*/
    return success;
}

// File is a list of pairs of times. Each line is: "m:s m2:s2"
bool VideoCHOP::getTimes(std::string filename, std::vector<timeVal> &times)
{
    bool success = true;
    std::ifstream file;
    file.open(filename);
    std::string line;
    
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        timeVal tv;
        char c, c2;

        if ( !(iss >> tv.m >> c >> tv.s >> tv.m2 >> c2 >> tv.s2) ) 
        {
            LOG << "ERROR: " << c << c2 << " " << tv.toString();
            success = false;
            break;
        }
        
        LOG << "Clip: " << tv.toString();
        times.push_back(tv);
    }

    return success;
}

// Copy video into vector of matrices.
static bool getFrames(std::vector<Mat> frames, const std::string videoname, int &codec, double &fps, Size &size)
{
    VideoCapture2 vid = VideoCapture(videoname);

    if (!vid.isOpened())
    {
        LOG << "Error: " << videoname << " not opened successfully.\n";
        return false;
    }

    // Get video info
    size = Size( (int)vid.get(CAP_PROP_FRAME_WIDTH), (int)vid.get(CAP_PROP_FRAME_HEIGHT));
    codec = static_cast<int>(vid.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
    double fps = vid.get(CAP_PROP_FPS);

    for(;;)
    { 
        Mat mat; 
        vid >> mat;

        if(mat.empty())
        {
            break;
        }

        frames.push_back(mat);
    }

    return true;
}