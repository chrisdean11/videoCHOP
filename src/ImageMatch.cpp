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

// Return a list of match scores. Perhaps this will be a different score for every method.
std::vector<float> ImageMatch::getScores(Mat img)
{
    std::vector<float> ret;
    return ret;
}

Mat ImageMatch::getBestMatch(Mat img)
{
    return img;
}

// Get the size of the slides
Size ImageMatch::getSize() const
{
    if (images.size() == 0) return Size(0,0);
    else return images[0].size();
}

// Get the image type of the slides
int ImageMatch::getType() const
{
    if (images.size() == 0) return -1;
    else return images[0].type();
}