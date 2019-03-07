#include "Log.h"

ImageMatch::ImageMatch(std::string dir)
{
    // Load slides
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

std::vector<float> ImageMatch::getScores(cv::Mat img)
{
    std::vector<float> ret;
    return ret;
}

cv::Mat ImageMatch::getBestMatch(cv::Mat img)
{
    return img;
}