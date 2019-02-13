/*
    VideoCHOP.cpp

*/
#include <fstream>
#include <strings.h>
#include "VideoCHOP.h"
#include "SelectHSV.h"
#include "LOG.h"

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
        LOG << "Error: " << videoname << " not opened successfully\n";
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
        LOG<<"Current Frame: "<<current<<"\n";
    }
*/
    /*New*/

    return success;
}

// Make a new video by cropping a source video while following a chosen object
bool VideoCHOP::crop(std::string videoname, int width, int height, std::string dest, int frameSpeed = 1, std::string trackMethod)
{
    VideoCapture vid = VideoCapture(videoname);
    method = trackMethod;

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

#if 0 // Troubleshoot motion tracking - show the tracked object
        circle(mat, loc, 5, Scalar( 0, 255, 255 ), -1);
#endif

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

#if 0 // Troubleshoot motion tracking - show the final center location
        circle(mat, loc, 5, Scalar( 255, 0, 255 ), -1);
#endif

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

    LOG<<"leaving getTimesFromFile()\n";

    return success;
}

// Copy video into vector of matrices. Explodes memory usage and is great for crashing your machine.
bool VideoCHOP::getAllFramesFromVideo(std::vector<Mat> &frames, const std::string &videoname, int &codec, double &fps, Size &size)
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
        LOG<<"Processing video...\n";
        
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
                LOG << "Invalid tracker type chosen: " << method << "\n";
                stayCenter = true;
                return Point(frame.cols/2, frame.rows/2);   
            }

            bool fromCenter = false;
            rect = selectROI(frame, fromCenter); 

            bool ok = tracker->init(frame, rect);

            if (ok)
            {
                LOG<<"Rect is:" << rect << "with center " << (rect.br() + rect.tl())*0.5 << "\n";
            }
            else
            {
                // sucks
                LOG << "Tracking failure\n";
            }

            return (rect.br() + rect.tl())*0.5;
        }
    }

    // Default to center if a threshold/tracker wasn't chosen
    if(stayCenter)
    {
        return Point(frame.cols/2, frame.rows/2);   
    }

    if (method == "COLOR")
    {
        // Convert to HSV and threshold
        Mat frame_HSV, frame_threshold;
        cvtColor(frame, frame_HSV, COLOR_BGR2HSV);
        inRange(frame_HSV, 
                Scalar(SelectHSV::low_H, SelectHSV::low_S, SelectHSV::low_V), 
                Scalar(SelectHSV::high_H, SelectHSV::high_S, SelectHSV::high_V), 
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

        LOG<<"Center is " << mc << "\n";
        return mc;
    }
    else // tracker
    {
        bool ok = tracker->update(frame, rect);

        if (ok)
        {
            LOG<<"Rect is:" << rect << "with center " << (rect.tl() + rect.br())*0.5 << "\n";
        }
        else
        {
            LOG<<"Tracking failure\n";
        }

        return (rect.br() + rect.tl())*0.5;
    }
}