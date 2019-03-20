#include "Log.h"
#include "ImageMatch.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

ImageMatch::ImageMatch(std::string dir)
{
    // Load slides into images list
    std::vector<std::string> filenames;
    glob(dir + "*", filenames, false);

    int count = filenames.size();

    for (int i = 0; i < count; i++)
    {
        Mat image;

        try
        {
            image = imread(filenames[i], IMREAD_COLOR);
        }
        catch (const std::exception & e)
        {
            Log::Log("Could not load %s: %s\n", filenames[i].c_str(), e.what());
            continue;
        }

        images.push_back(image);
    }
}

std::vector<float> ImageMatch::getScores(Mat img)
{
    std::vector<float> ret;
    return ret;
}

Mat ImageMatch::getBestMatch(Mat img)
{
    return img;
}

Size ImageMatch::getSize()
{
    if (images.size() == 0) return Size(0,0);
    else return Size(images[0].size());
}