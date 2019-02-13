#include <opencv2/core.hpp>     // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/imgproc.hpp>  // Gaussian Blur
#include <opencv2/highgui.hpp>  // OpenCV window I/O

/*
    Code below was adapted from https://docs.opencv.org/3.4/da/d97/tutorial_threshold_inRange.html
    It controls the user's color selection and saves the hsv high and low values.
    No point in doing it myself.
*/

namespace SelectHSV{

using namespace std;

static const int max_value_H = 360/2;
static const int max_value = 255;
static const std::string window_capture_name = "Video Capture";
static const std::string window_detection_name = "Object Detection";
static int low_H = 0, low_S = 0, low_V = 0;
static int high_H = max_value_H, high_S = max_value, high_V = max_value;

// int min (int a, int b)
// {
//     return a>b?b:a;
// }
// 
// int max (int a, int b)
// {
//     return a>b?a:b;
// }

    // Callbacks for showAndSelectColor() trackbar
static void on_low_H_thresh_trackbar(int, void *)
{
    low_H = cv::min(high_H-1, low_H);
    cv::setTrackbarPos("Low H", window_detection_name, low_H);
}
static void on_high_H_thresh_trackbar(int, void *)
{
    high_H = cv::max(high_H, low_H+1);
    cv::setTrackbarPos("High H", window_detection_name, high_H);
}
static void on_low_S_thresh_trackbar(int, void *)
{
    low_S = cv::min(high_S-1, low_S);
    cv::setTrackbarPos("Low S", window_detection_name, low_S);
}
static void on_high_S_thresh_trackbar(int, void *)
{
    high_S = cv::max(high_S, low_S+1);
    cv::setTrackbarPos("High S", window_detection_name, high_S);
}
static void on_low_V_thresh_trackbar(int, void *)
{
    low_V = cv::min(high_V-1, low_V);
    cv::setTrackbarPos("Low V", window_detection_name, low_V);
}
static void on_high_V_thresh_trackbar(int, void *)
{
    high_V = cv::max(high_V, low_V+1);
    cv::setTrackbarPos("High V", window_detection_name, high_V);
}

int showAndSelectColor(cv::Mat frame)
{
std::cerr<<"entering showAndSelectColor\n";
    //cap
    cv::namedWindow(window_detection_name);

    // Trackbars to set thresholds for HSV values
    cv::createTrackbar("Low H", window_detection_name, &low_H, max_value_H, on_low_H_thresh_trackbar);
    cv::createTrackbar("High H", window_detection_name, &high_H, max_value_H, on_high_H_thresh_trackbar);
    cv::createTrackbar("Low S", window_detection_name, &low_S, max_value, on_low_S_thresh_trackbar);
    cv::createTrackbar("High S", window_detection_name, &high_S, max_value, on_high_S_thresh_trackbar);
    cv::createTrackbar("Low V", window_detection_name, &low_V, max_value, on_low_V_thresh_trackbar);
    cv::createTrackbar("High V", window_detection_name, &high_V, max_value, on_high_V_thresh_trackbar);
    cv::Mat frame_HSV, frame_threshold;

    while (true) 
    {
        // Convert from BGR to HSV colorspace
        cv::cvtColor(frame, frame_HSV, cv::COLOR_BGR2HSV);
        // Detect the object based on HSV Range Values
        cv::inRange(frame_HSV, cv::Scalar(low_H, low_S, low_V), cv::Scalar(high_H, high_S, high_V), frame_threshold);
        // Show the frames
        cv::imshow(window_detection_name, frame_threshold);
        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27 /*esc*/ || key == ' ' || key == 13 /* \r */ || key == 10 /* \n */)
        {
            break;
        }
    }

    cv::destroyWindow(window_detection_name);

std::cerr<<"leaving showAndSelectColor\n";

    // Check if user chose no threshold.
    if(
        low_H == 0 &&
        low_S == 0 &&
        low_V == 0 &&
        high_H == max_value_H &&
        high_S == max_value &&
        high_V == max_value
        )
    {
        return 0;
    }

    return 1;
}

}


