/*
    VideoCHOP.cpp

*/
#include <fstream>
#include "VideoCHOP.h"
#include "LOG.h"

#define FRAMESPEED 1 // Max number of pixels a cropped video can move in x or y, per frame.

using namespace cv;
bool first = true;

// Code to find threshold values from first frame taken from https://docs.opencv.org/3.4/da/d97/tutorial_threshold_inRange.html
int showAndSelectColor(Mat frame);
const int max_value_H = 360/2;
const int max_value = 255;
const String window_capture_name = "Video Capture";
const String window_detection_name = "Object Detection";
int low_H = 0, low_S = 0, low_V = 0;
int high_H = max_value_H, high_S = max_value, high_V = max_value;



bool VideoCHOP::chop(std::string videoname, std::string filename, std::string dest)
{
    bool success = true;
    std::vector<timeVal> times;

    if (!getTimes(filename, times))
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
    /*Old */

    /*New - takes 17 seconds on a 26s test clip*/

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
bool VideoCHOP::crop(std::string videoname, int width, int height, std::string dest)
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
        Point loc = findObject(mat);
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
bool VideoCHOP::getFrames(std::vector<Mat> &frames, const std::string &videoname, int &codec, double &fps, Size &size)
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
    Mat frame_HSV, frame_threshold;

    // Make thresholded image
    if(first)
    {
        // Get HSV threshold from the first frame
        showAndSelectColor(frame);
        first = false;
    }
    cvtColor(frame, frame_HSV, COLOR_BGR2HSV);
    inRange(frame_HSV, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), frame_threshold);

    // Process thresholded image, remove any small spots etc
    // ...

    // Detect the object based on HSV Range Values
    std::vector<std::vector<Point>> contours;
    Mat contourOutput = frame_threshold.clone();
    findContours(contourOutput, contours,  RETR_LIST, CHAIN_APPROX_NONE);

    // Find the biggest object
    int biggestIndex;
    int biggestArea = 0;
    for(size_t i = 0; i < contours.size(); i++)
    {
        int area = contourArea(contours[i]); // Vector of points 

        //for(auto pt : contours[i])
        //{
        //    ++mass;
        //}
        ////try{}catch(zerodivisionerror)

        if(area > biggestArea)
        {
            biggestArea = area;
            biggestIndex = i;
        }
    }

    Moments mu = moments(contours[biggestIndex]);

    //Point2f mc = Point2f(   static_cast<float>(mu.m10 / (mu.m00 + 1e-5)), 
    //                        static_cast<float>(mu.m01 / (mu.m00 + 1e-5)) ); //add 1e-5 to avoid division by zero

    Point mc = Point( (int)((float)mu.m10 / ((float)mu.m00 + 1e-5)) , 
                        (int)((float)mu.m01 / ((float)mu.m00 + 1e-5)) ); //add 1e-5 to avoid division by zero
    
    // Find center
    //return Point(mat.cols/2, mat.rows/2);
    return mc;
}

// TODO fix this nonsense to not use globals. Make it one function.
static void on_low_H_thresh_trackbar(int, void *)
{
    low_H = min(high_H-1, low_H);
    setTrackbarPos("Low H", window_detection_name, low_H);
}
static void on_high_H_thresh_trackbar(int, void *)
{
    high_H = max(high_H, low_H+1);
    setTrackbarPos("High H", window_detection_name, high_H);
}
static void on_low_S_thresh_trackbar(int, void *)
{
    low_S = min(high_S-1, low_S);
    setTrackbarPos("Low S", window_detection_name, low_S);
}
static void on_high_S_thresh_trackbar(int, void *)
{
    high_S = max(high_S, low_S+1);
    setTrackbarPos("High S", window_detection_name, high_S);
}
static void on_low_V_thresh_trackbar(int, void *)
{
    low_V = min(high_V-1, low_V);
    setTrackbarPos("Low V", window_detection_name, low_V);
}
static void on_high_V_thresh_trackbar(int, void *)
{
    high_V = max(high_V, low_V+1);
    setTrackbarPos("High V", window_detection_name, high_V);
}

int showAndSelectColor(Mat frame)
{
    //cap
    namedWindow(window_capture_name);
    namedWindow(window_detection_name);

    // Trackbars to set thresholds for HSV values
    createTrackbar("Low H", window_detection_name, &low_H, max_value_H, on_low_H_thresh_trackbar);
    createTrackbar("High H", window_detection_name, &high_H, max_value_H, on_high_H_thresh_trackbar);
    createTrackbar("Low S", window_detection_name, &low_S, max_value, on_low_S_thresh_trackbar);
    createTrackbar("High S", window_detection_name, &high_S, max_value, on_high_S_thresh_trackbar);
    createTrackbar("Low V", window_detection_name, &low_V, max_value, on_low_V_thresh_trackbar);
    createTrackbar("High V", window_detection_name, &high_V, max_value, on_high_V_thresh_trackbar);
    Mat frame_HSV, frame_threshold;

    while (true) 
    {
        // Convert from BGR to HSV colorspace
        cvtColor(frame, frame_HSV, COLOR_BGR2HSV);
        // Detect the object based on HSV Range Values
        inRange(frame_HSV, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), frame_threshold);
        // Show the frames
        imshow(window_capture_name, frame);
        imshow(window_detection_name, frame_threshold);
        char key = (char) waitKey(30);
        if (key == 'q' || key == 27)
        {
            break;
        }
    }

    return 0;
}