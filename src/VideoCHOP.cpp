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

#define FRAMESPEED 1 // Max number of pixels a cropped video can move in x or y, per frame.

using namespace cv;

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

// "Private" function declarations
bool getTimes(std::string filename, std::vector<timeVal> &times);
bool getFrames(std::vector<Mat> &frames, const std::string &videoname, int &codec, double &fps, Size &size);
Point findObject(const Mat &mat, int color);

bool VideoCHOP::chop(std::string videoname, std::string filename, std::string dest)
{
    bool success = true;
    std::vector<timeVal> times;
    int clipnum = 0;

    if (!getTimes(filename, times))
    {
        return false;
    }

    /*
     * Make clips
     */
    for (timeVal &time : times)
    {
        ++clipnum;
        VideoCapture vid = VideoCapture(videoname);

        // TODO: get file extension of destination and apply it to source

        // Make the new clip's name
        std::stringstream ss;
        ss << dest << clipnum << "_" << time.m << time.s << "-" << time.m2 << time.s2 << ".mp4";
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

    return success;
}

// Make a new video by cropping a source video while following an object of a certain color
bool VideoCHOP::crop(std::string videoname, int width, int height, int color, std::string dest)
{
    VideoCapture vid = VideoCapture(videoname);

    if (!vid.isOpened())
    {
        LOG << "Error: " << videoname << " not opened successfully.\n";
        return false;
    }

    // Get source clip info: size, fourcc, fps
    Size S = Size( (int)vid.get(CAP_PROP_FRAME_WIDTH), (int)vid.get(CAP_PROP_FRAME_HEIGHT));
    int ex = static_cast<int>(vid.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
    double fps = vid.get(CAP_PROP_FPS);

    // Make WideoWriter
    Size s = Size(width, height);
    VideoWriter cropped = VideoWriter(dest, ex, fps, s, true);
    Point prevLoc;
    bool firstFrame = true;

    // Go through each frame, making a new cropped frame from each boy
    for(;;)
    {
        Mat mat; 

        if(!vid.read(mat))
        {
            LOG << "Finished reading video frames\n";
            break;
        }

        Mat mat2 = Mat(s, mat.type());

        // Obtain location
        Point loc = findObject(mat, color);
        if(loc.x == -1) loc = prevLoc;

        // Reconcile loc with previous frame location
        if (firstFrame)
        {
            prevLoc = loc;
            firstFrame = false;
        }
        else
        {
            int moveX = loc.x - prevLoc.x; // If prevLoc needs to move negative, this will be negative
            int moveY = loc.y - prevLoc.y;

            // Limit movement to FRAMESPEED
            if (moveX > FRAMESPEED)             moveX = FRAMESPEED;
            else if ((0 - moveX) > FRAMESPEED)  moveX = -FRAMESPEED;
            if (moveY > FRAMESPEED)             moveY = FRAMESPEED;
            else if ((0 - moveY) > FRAMESPEED)  moveY = -FRAMESPEED;
        
            loc.x = loc.x + moveX;
            loc.y = loc.y + moveY;
        }

        // Pull back if loc goes over edge.
        {
            // Dividing odd value returns the floor
            if (loc.x - (width/2) < 0)              loc.x = width/2;
            else if (loc.x + (width/2) > S.width)   loc.x = S.width - width/2;
            if (loc.y - (height/2) < 0)             loc.y = height/2;
            else if (loc.y + (height/2) > S.height) loc.y = S.height - height/2;
        }

        // This is the final crop location
        prevLoc = loc;

        // Crop to final location
        mat2 = Mat(mat, Range((loc.y - s.height/2), (loc.y + s.height/2)), Range((loc.x - s.width/2), (loc.x + s.width/2)));

        cropped << mat2; 
    }

    return true;

}

// File is a list of pairs of times. Each line is: "m:s m2:s2"
bool getTimes(std::string filename, std::vector<timeVal> &times)
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

        if ( !(iss >> tv.m >> c >> tv.s >> tv.m2 >> c2 >> tv.s2 ) ) 
        {
            LOG << "ERROR: " << c << c2 << " " << tv.toString();
            success = false;
            break;
        }
        
        LOG << "Clip: " << tv.toString();
        times.push_back(tv);
    }

    LOG << "times: \n";

    for (auto time : times)
    {
        LOG << time.toString()<<"\n";
    } 

    LOG<<"leaving getTimes()\n";

    return success;
}

// Copy video into vector of matrices. Explodes memory usage and is great for crashing your machine.
bool getFrames(std::vector<Mat> &frames, const std::string &videoname, int &codec, double &fps, Size &size)
{
    VideoCapture vid = VideoCapture(videoname);

    if (!vid.isOpened())
    {
        LOG << "Error: " << videoname << " not opened successfully.\n";
        return false;
    }

    // Get video info
    size = Size( (int)vid.get(CAP_PROP_FRAME_WIDTH), (int)vid.get(CAP_PROP_FRAME_HEIGHT));
    codec = static_cast<int>(vid.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
    fps = vid.get(CAP_PROP_FPS);

    for(;;)
    { 
        Mat mat;

        if(!vid.read(mat))
        {
            LOG << "Finished reading video frames\n";
            break;
        }

        frames.push_back(mat.clone());
    }

    return true;
}

// Returns center of mass of a certain color. If it failed to find the object, it will return (-1,-1)
Point findObject(const Mat &mat, int color)
{
    if (0) LOG << "color is " << color <<"\n";
    // For now, return very center
    return Point(mat.cols/2, mat.rows/2);

    // Threshold image

    // Process, remove any small spots etc

    // Find center
}
