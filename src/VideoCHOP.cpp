/*
    VideoCHOP.cpp

*/
#include <fstream>
#include <strings.h>
#include "VideoCHOP.h"
#include "SelectHSV.h"
#include "Log.h"

#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

using namespace cv;
bool first = true;

bool VideoCHOP::chop(std::string videoname, std::string filename, std::string dest)
{
    bool success = true;
    std::vector<timeVal> times;

    if (!getTimesFromFile(filename, times))
    {
        return false;
    }

    /*
     * Make clips
     */

    /*Old - takes 16 seconds on a 26s test clip*/

    int clipnum = 0;
    for (timeVal &time : times)
    {
        ++clipnum;
        VideoCapture vid = VideoCapture(videoname);

        // Make the new clip's name
        std::stringstream ss;
        ss << dest << clipnum << "_" << time.m << time.s << "-" << time.m2 << time.s2 << ".mp4";
        std::string clipname = ss.str();

        if (!vid.isOpened())
        {
            Log::Log("Error: %s not opened successfully while making %s\n", videoname.c_str(), clipname.c_str());
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
    /*Old */

    /*  
        New - takes 17 seconds on a 26s test clip. 
        It should be much faster than the old method with a larger video or more clips
    */

    /*
     * Make clips
     */
/*
    VideoCapture vid = VideoCapture(videoname);

    if (!vid.isOpened())
    {
        Log::Log("Error: %s not opened successfully\n", videoname.c_str());
        return false;
    }

    // Get clip info: size, fourcc, fps
    Size S = Size( (int)vid.get(CAP_PROP_FRAME_WIDTH), (int)vid.get(CAP_PROP_FRAME_HEIGHT));
    int ex = static_cast<int>(vid.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
    double fps = vid.get(CAP_PROP_FPS);

    // Make writer streams
    std::vector<VideoWriter> clips;

    for(uint i = 0; i < times.size(); i++)
    {
        // Make the new clip's name
        std::stringstream ss;
        ss << dest << "clip" << i << "_" << times[i].m << times[i].s << "-" << times[i].m2 << times[i].s2 << ".mp4";
        std::string clipname = ss.str();

        VideoWriter clip = VideoWriter(clipname, ex, fps, S, true);

        clips.push_back(clip);
    }

    int current = 0; // Current frame
    //int lastFrame = 60*times.back().m2 + times.back().s2;
    Mat mat;

    while(vid.read(mat))
    {
        // Add this frame to each clip it belongs to
        for(uint i = 0; i < clips.size(); ++i)
        {
            // Get the frame numbers for this clip
            int start = fps * ((60*(times[i].m)) + times[i].s);
            int end = fps * ((60*(times[i].m2)) + times[i].s2);

            if(current >= start && current < end)
            {
                clips[i] << mat; //.clone();
            }
        }

        ++current;
        Log::Log("Current Frame: %d\n", current);
    }
*/
    /*New*/

    return success;
}

// Make a new video by cropping a source video while following a chosen object
bool VideoCHOP::crop(std::string videoname, int width, int height, std::string dest, int frameSpeed, std::string trackMethod, bool dots)
{
    VideoCapture vid = VideoCapture(videoname);
    method = trackMethod;

    if (!vid.isOpened())
    {
        Log::Log("Error: %s not opened successfully.\n", videoname.c_str());
        return false;
    }

    // Get source clip info: size, fourcc, fps
    Size S = Size( (int)vid.get(CAP_PROP_FRAME_WIDTH), (int)vid.get(CAP_PROP_FRAME_HEIGHT));
    int ex = static_cast<int>(vid.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
    double fps = vid.get(CAP_PROP_FPS);

    // Make WideoWriter
    Size s = Size(width, height);
    VideoWriter cropped = VideoWriter(dest, ex, fps, s, true);

    // Go through each frame, making a new cropped frame from each boy
    Point prevLoc;
    bool firstFrame = true;
    for(;;)
    {
        Mat mat; 

        if(!vid.read(mat))
        {
            break;
        }

        Mat mat2 = Mat(s, mat.type());

        // Obtain location
        Point loc = findObject(mat);
        if(loc.x == -1) loc = prevLoc;
        
        if (dots)
        {
            // Show the tracked object
            circle(mat, loc, 5, Scalar( 0, 255, 255 ), -1); 
        }

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

            // Limit movement to frameSpeed
            if (moveX > frameSpeed)         moveX = frameSpeed;  // moveX too far in + direction
            else if (moveX < -frameSpeed)   moveX = -frameSpeed; // moveX too far in - direction
            if (moveY > frameSpeed)         moveY = frameSpeed;  // moveY too far in + direction
            else if (moveY < -frameSpeed)   moveY = -frameSpeed; // moveY too far in - direction
        
            // Make move of Loc
            loc.x = prevLoc.x + moveX;
            loc.y = prevLoc.y + moveY;
        }

        // Pull back if loc goes over edge.
        {
            // Dividing odd value returns the floor
            if (loc.x - (width/2) < 0)              loc.x = width/2;
            else if (loc.x + (width/2) > S.width)   loc.x = S.width - width/2;
            if (loc.y - (height/2) < 0)             loc.y = height/2;
            else if (loc.y + (height/2) > S.height) loc.y = S.height - height/2;
        }

        // This is the final crop location for this frame
        prevLoc = loc;

        if (dots)
        {
            // Show the final center location
            circle(mat, loc, 5, Scalar( 255, 0, 255 ), -1); 
        }

        // Crop to final location
        mat2 = Mat(mat, Range((loc.y - s.height/2), (loc.y + s.height/2)), Range((loc.x - s.width/2), (loc.x + s.width/2)));

        cropped << mat2; 
    }

    return true;

}

// File is a list of pairs of times. Each line is: "m:s m2:s2"
bool VideoCHOP::getTimesFromFile(std::string filename, std::vector<timeVal> &times)
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
            Log::Log("ERROR: %c %c %s", c, c2, tv.toString().c_str());
            success = false;
            break;
        }
        
        Log::Log("Clip: %s", tv.toString().c_str());
        times.push_back(tv);
    }

    Log::Log("times: \n");

    for (auto time : times)
    {
        Log::Log("%s\n", time.toString().c_str());
    } 

    Log::Log("leaving getTimesFromFile()\n");

    return success;
}

// Copy video into vector of matrices. Explodes memory usage and is great for crashing your machine.
bool VideoCHOP::getAllFramesFromVideo(std::vector<Mat> &frames, const std::string &videoname, int &codec, double &fps, Size &size)
{
    VideoCapture vid = VideoCapture(videoname);

    if (!vid.isOpened())
    {
        Log::Log("Error: %s not opened successfully.\n", videoname.c_str());
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
            Log::Log("Finished reading video frames\n");
            break;
        }

        frames.push_back(mat.clone());
    }

    return true;
}

// Returns center point of the biggest object detected after threshold.
Point VideoCHOP::findObject(const Mat &frame)
{
    // TODO just make data members out of these static and globals
    static bool stayCenter = false;
    static Rect2d rect;
    static Ptr<Tracker> tracker;

    // Make threshold/tracker
    if(first)
    {
        first = false;
        Log::Log("Processing video...\n");
        
        if (strcasecmp(method.c_str(), "COLOR") == 0)
        {
            // Get HSV threshold from the first frame
            if(SelectHSV::showAndSelectColor(frame) == 0)
            {
                stayCenter = true;
            }
        } 
        else // tracker
        {
            if (strcasecmp(method.c_str(), "BOOSTING") == 0)
            {
                tracker = TrackerBoosting::create();
            }
            else if (strcasecmp(method.c_str(), "MIL") == 0)
            {
                tracker = TrackerMIL::create();
            }
            else if (strcasecmp(method.c_str(), "KCF") == 0)
            {
                tracker = TrackerKCF::create();
            }
            else if (strcasecmp(method.c_str(), "TLD") == 0)
            {
                tracker = TrackerTLD::create();
            }
            else if (strcasecmp(method.c_str(), "MEDIANFLOW") == 0)
            {
                tracker = TrackerMedianFlow::create();
            }
            else if (strcasecmp(method.c_str(), "GOTURN") == 0)
            {
                tracker = TrackerGOTURN::create();
            }
            else if (strcasecmp(method.c_str(), "MOSSE") == 0)
            {
                tracker = TrackerMOSSE::create();
            }
            else if (strcasecmp(method.c_str(), "CSRT") == 0)
            {
                tracker = TrackerCSRT::create();
            }
            else
            {
                Log::Log("Invalid tracker type chosen: %s\n", method.c_str());
                stayCenter = true;
                return Point(frame.cols/2, frame.rows/2);   
            }

            bool fromCenter = false;
            rect = selectROI(frame, fromCenter); 

            bool ok = tracker->init(frame, rect);

            if (ok)
            {
                Log::Log("Rect is:(%.0f,%.0f) to (%.0f,%.0f) with center (%.0f,%.0f)\n", rect.tl().x, rect.tl().y, rect.br().x, rect.br().y, ((rect.br() + rect.tl())*0.5).x, ((rect.br() + rect.tl())*0.5).y);
            }
            else
            {
                // sucks
                Log::Log("Tracking failure\n");
            }

            return (rect.br() + rect.tl())*0.5;
        }
    }

    // Default to center if a threshold/tracker wasn't chosen
    if(stayCenter)
    {
        return Point(frame.cols/2, frame.rows/2);   
    }

    if (strcasecmp(method.c_str(), "COLOR") == 0)
    {
        // Convert to HSV and threshold
        Mat frame_HSV, frame_threshold;
        cvtColor(frame, frame_HSV, COLOR_BGR2HSV);

        Log::Log("Threshold: ");
        Log::Log("H:%d-%d S:%d-%d V:%d-%d\n", SelectHSV::getLH(), SelectHSV::getHH(), SelectHSV::getLS(), SelectHSV::getHS(), SelectHSV::getLV(), SelectHSV::getHV());

        inRange(frame_HSV, 
                Scalar(SelectHSV::getLH(), SelectHSV::getLS(), SelectHSV::getLV()), 
                Scalar(SelectHSV::getHH(), SelectHSV::getHS(), SelectHSV::getHV()), 
                frame_threshold);
    
        // Process thresholded image, remove any small spots etc
        // ...
    
        // Detect the object based on HSV Range Values
        std::vector<std::vector<Point>> contours;
        Mat contourOutput = frame_threshold.clone();
        findContours(contourOutput, contours, RETR_LIST, CHAIN_APPROX_NONE);
    
        // Find the biggest object
        int biggestIndex;
        int biggestArea = 0;
        for(size_t i = 0; i < contours.size(); i++)
        {
            int area = contourArea(contours[i]); // Vector of points 
    
            if(area > biggestArea)
            {
                biggestArea = area;
                biggestIndex = i;
            }
        }
    
        // Find center
        Moments mu = moments(contours[biggestIndex]);
        Point mc = Point( (int)((float)mu.m10 / ((float)mu.m00 + 1e-5)) , 
                            (int)((float)mu.m01 / ((float)mu.m00 + 1e-5)) ); //add 1e-5 to avoid division by zero

        Log::Log("Center is (%d,%d)\n", mc.x, mc.y);
        return mc;
    }
    else // tracker
    {
        bool ok = tracker->update(frame, rect);

        if (ok)
        {
            Log::Log("Rect is:(%.0f,%.0f) to (%.0f,%.0f) with center (%.0f,%.0f)\n", rect.tl().x, rect.tl().y, rect.br().x, rect.br().y, ((rect.br() + rect.tl())*0.5).x, ((rect.br() + rect.tl())*0.5).y);
        }
        else
        {
            Log::Log("Tracking failure\n");
        }

        return (rect.br() + rect.tl())*0.5;
    }
}