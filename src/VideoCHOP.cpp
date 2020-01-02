/*
    VideoCHOP.cpp

*/
#include <fstream>
#include <strings.h>
#include "VideoCHOP.h"
#include "SelectHSV.h"
#include "Log.h"
#include "ImageMatch.h"

#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

using namespace cv;
bool first = true;

// Define static members
Point VideoCHOP::a = Point(-1,-1);
Point VideoCHOP::b = Point(-1,-1);
Point VideoCHOP::c = Point(-1,-1);
Point VideoCHOP::d = Point(-1,-1);

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

// Returns center point of the object detected by the chosen method
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

            bool fromCenter = false; // How the rectangle is drawn
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

    // Convert to HSV and find center of thresholded object
    if (strcasecmp(method.c_str(), "COLOR") == 0)
    {
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

bool VideoCHOP::threshold(std::string src/*, std::string dst*/)
{
    Mat image = imread(src.c_str(), IMREAD_COLOR);
    method = "color";
    Mat image_resized;
    cv::resize(image, image_resized, cv::Size(), 0.20, 0.20);

    //apply threshold to full-size image
    //imwrite(image_thresholded, dst);
    return true;
}

bool VideoCHOP::slideshow(std::string srcname, std::string dstname, std::string imageFolder)
{
    std::vector<Mat> slides;
    VideoCapture src;
    VideoWriter dst;
    ImageMatch imageMatch(imageFolder);
    Size slideSize = imageMatch.getSize();
    std::string window = "debug window"; 

    // Open video files
    src = VideoCapture(srcname);

    if (!src.isOpened())
    {
        Log::Log("Error: %s not opened successfully.\n", srcname.c_str());
        return false;
    }

    // Get source clip info: size, fourcc, fps
    Size S = Size( (int)src.get(CAP_PROP_FRAME_WIDTH), (int)src.get(CAP_PROP_FRAME_HEIGHT));
    int ex = static_cast<int>(src.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
    double fps = src.get(CAP_PROP_FPS);

    // Output video is same size as source image
    dst = VideoWriter(dstname, ex, fps, slideSize, true);

    // Match each frame:
    bool firstFrame = true;
    for(;;)
    {
        Mat mat;

        if(!src.read(mat))
        {
            break;
        }
        else if (firstFrame)
        {
            // On first frame, select four corners of screen
            std::string selectionwindow = "Select presentation screen";
            namedWindow(selectionwindow);
            Point p;
            setMouseCallback(selectionwindow, mouseCallback, &mat);

            imshow(selectionwindow, mat);
            waitKey(0);
            destroyWindow(selectionwindow);

            firstFrame = false;
        }

        // Grab and perform affine transformation
        Point2f srcTri[3];
        Point2f dstTri[3];
        Mat warp_mat( 2, 3, CV_32FC1 );
        Mat warp_dst(Mat::zeros(imageMatch.getSize(), imageMatch.getType()));

        srcTri[0] = a;
        srcTri[1] = b;
        srcTri[2] = d;

        dstTri[0] = Point2f(0, 0);
        dstTri[1] = Point2f(imageMatch.getSize().width, 0);
        dstTri[2] = Point2f(0, imageMatch.getSize().height);

        warp_mat = getAffineTransform(srcTri, dstTri);
        warpAffine(mat, warp_dst, warp_mat, warp_dst.size());

        // Choose best match
        Mat result = imageMatch.getBestMatch(warp_dst);

        // Add this slide to the output video
        dst << result;

        imshow(window, mat);
        waitKey(50);
    }

    return true;
}

// Select points on left click
void VideoCHOP::mouseCallback(int event, int x, int y, int flags, void* userdata)
{
    Point *p; // point to set if this is a click
    Mat *ptr = (Mat *)userdata;

    Mat img = ptr->clone();

    if (d != Point(-1,-1))
    {
        // All four points have been selected
        return;
    }
    else if (c != Point(-1,-1))
    {
        // Three points have already been selected. Draw full box with current position.
        //void line(InputOutputArray img, Point pt1, Point pt2, const Scalar& color, int thickness=1, int lineType=LINE_8, int shift=0 )
        line(img, a, b, Scalar(255, 0, 0), 2, LINE_8, 0);
        line(img, b, c, Scalar(255, 0, 0), 2, LINE_8, 0);
        line(img, c, Point(x,y), Scalar(255, 0, 0), 2, LINE_8, 0);
        line(img, Point(x,y), a, Scalar(255, 0, 0), 2, LINE_8, 0);
        p = &d;
    }
    else if (b != Point(-1,-1))
    {
        // Two points have already been selected.
        line(img, a, b, Scalar(255, 0, 0), 2, LINE_8, 0);
        line(img, b, Point(x,y), Scalar(255, 0, 0), 2, LINE_8, 0);
        p = &c;
    }
    else if (a != Point(-1,-1))
    {
        // One point has already been selected.
        line(img, a, Point(x,y), Scalar(255, 0, 0), 2, LINE_8, 0);
        p = &b;
    }
    else
    {
        // This is the first point to be selected.
        p = &a;
    }

    std::string windowname = "Select presentation screen";
    cv::imshow(windowname, img);

    if (event == EVENT_LBUTTONDOWN)
    {
        *p = Point(x,y);
        return;
    }
}

// Returns 1 for red, 2 for green, 3 for yellow. 0 if not found.
// inImg becomes the green thresholded mat
int VideoCHOP::containsBox(Mat& img)
{
    Mat hsv;
    cvtColor(img, hsv, CV_BGR2HSV);

    // Frame 37770
    uchar minRedH = 159;
    uchar maxRedH = 180;
    uchar minRedS = 63;
    uchar maxRedS = 183;
    uchar minRedV = 103;
    uchar maxRedV = 239;

    // Frame 3700
    uchar minGrnH = 49;
    uchar maxGrnH = 82;
    uchar minGrnS = 76;
    uchar maxGrnS = 167;
    uchar minGrnV = 182;
    uchar maxGrnV = 255;

    uchar minYelH = 0;
    uchar maxYelH = 0;
    uchar minYelS = 0;
    uchar maxYelS = 0;
    uchar minYelV = 0;
    uchar maxYelV = 0;

    int startRow = 1080/5; // Cut off top 20%
    int endColumn = 1920/2; // Cut off right 50%

    int redMatches = 0;
    int greenMatches = 0;
    int yellowMatches = 0;
    int minMatches = 300; // Minimum number of matching pixels to consider this a match
    // x=250 pixels wide typical box, 750 typical perimeter, 1 pixel thick

    // Add up the matching pixels. This whole thing hinges on this being accurate.
    for (int row = startRow; row < 1080; row++)
    {
        // For each pixel, check if it's a match
        for (int col = 0; col < endColumn; col++)
        {
            Vec3b pixel = hsv.at<Vec3b>(Point(col, row));
            uchar hue = pixel.val[0];
            uchar sat = pixel.val[1];
            uchar val = pixel.val[2];

            if ( 
                minGrnH <= hue &&
                maxGrnH >= hue  &&
                minGrnS <= sat &&
                maxGrnS >= sat &&
                minGrnV <= val &&
                maxGrnV >= val
                )
            {
                ++greenMatches;
                if (minMatches < greenMatches) return 2;
            }

            // DEBUG VERSION BELOW
/*
            //Paint the right side black
            img.at<Vec3b>(row, (col + 1920/2)) = Vec3b(0, 0, 0);

            if ( minRedH <= hue &&
                maxRedH >= hue  &&
                minRedS <= sat &&
                maxRedS >= sat &&
                minRedV <= val &&
                maxRedV >= val
                )
            {
                ++redMatches;
            }
            else if ( 
                minGrnH <= hue &&
                maxGrnH >= hue  &&
                minGrnS <= sat &&
                maxGrnS >= sat &&
                minGrnV <= val &&
                maxGrnV >= val
                )
            {
                //Paint the matches white
                img.at<Vec3b>(row, (col + 1920/2)) = Vec3b(255, 255, 255);
                ++greenMatches;
            }
            else if (
                minYelH <= hue &&
                maxYelH >= hue  &&
                minYelS <= sat &&
                maxYelS >= sat &&
                minYelV <= val &&
                maxYelV >= val
                )
            {
                ++yellowMatches;
            }
*/
        }
    }

    //Log::Log("red=%d grn=%d yel=%d       ",redMatches,greenMatches,yellowMatches);

    //if (minMatches < redMatches) return 1; 
    if (minMatches < greenMatches) return 2;
    //else if (minMatches < yellowMatches) return 3;

    return 0;
}

bool VideoCHOP::greenBoxTimes(std::string srcname, std::string dstname)
{
    bool boxPresent = false; // If currently following a green box
    int waitCount = 0; 
    int waitFor = 120; // 30 * 4 seconds
    int start, end;
    VideoCapture src;
    namedWindow("Video",1);

    // File writer
    std::ofstream file;
    file.open(dstname);

    // Open video file
    src = VideoCapture(srcname);

    if (!src.isOpened())
    {
        Log::Log("Error: %s not opened successfully.\n", srcname.c_str());
        return false;
    }

    int skipCount = 3600; // Skip roughly the first 2 minutes. 30*60*2
    int count = 0;

    // Find green boxes
    for(;;)
    {
        Mat mat;
        ++count;

        if(!src.read(mat))
        {
            break;
        }
        // Optionally skip
        else if (--skipCount > 0) 
        {
            continue;
        }

        Mat result = mat.clone();
        int found = containsBox(result);

        if (found)
        {
            // Print result
            //if (found == 1) { Log::Log("RED"); }
            //else if (found == 2) { Log::Log("GREEN"); }
            //else if (found == 3) { Log::Log("YELLOW"); }

            // Begin new clip if starting track on new box
            if (!boxPresent)
            {
                if (count < waitFor)
                {
                    start = 0;
                }
                else start = count - waitFor;
            }

            boxPresent = true;
            waitCount = 0;
        }
        else // not found
        {
            if (boxPresent)
            {
                ++waitCount;
                if (waitCount > waitFor)
                {
                    boxPresent = false;
                    end = count;

                    // Write start and end to file
                    int startMinutes = start/(30*60);
                    int startSeconds = (start - startMinutes*30*60)/30;
                    int endMinutes = end/(30*60);
                    int endSeconds = (end - endMinutes*30*60)/30;
                    file<<startMinutes<<":"<<startSeconds<<" "<<endMinutes<<":"<<endSeconds<<std::endl;

                    printf("END CLIP, %d:%d(%d) - %d:%d(%d)\n", startMinutes,startSeconds,start,endMinutes,endSeconds,end);
                }
            }
        }

        //Log::Log("\n");
        
        //imshow("Video", result);
        //if(waitKey(30) >= 0) break;
    }

    file.close();
    return true;
}

void VideoCHOP::showAndSelectColorAtTime(std::string srcname, int frameNum)
{
    // Open video and cycle to frame
    VideoCapture src;
    namedWindow("Video",1);

    // Open video file
    src = VideoCapture(srcname);

    if (!src.isOpened())
    {
        Log::Log("Error: %s not opened successfully.\n", srcname.c_str());
        return;
    }

    Mat mat;
    int count = 0; 
    // Find threshold values
    for(;;)
    {
        if(!src.read(mat))
        {
            break;
        }
        else if (++count < frameNum)
        {
            continue;
        }

        // This is the frame

        SelectHSV::showAndSelectColor(mat);
        break;
    }

}