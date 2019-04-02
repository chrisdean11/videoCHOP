#include "Log.h"
#include "ImageMatch.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "opencv2/opencv_modules.hpp"
# include "opencv2/core/core.hpp"
# include "opencv2/features2d/features2d.hpp"
# include "opencv2/highgui/highgui.hpp"
# include "opencv2/nonfree/features2d.hpp"

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
    return images[surf(img)];
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

/*
    Matching
*/

/**
 * @file SURF_FlannMatcher
 * @brief SURF detector + descriptor + FLANN Matcher
 * @author A. Huaman
 */

#ifndef HAVE_OPENCV_NONFREE

int ImageMatch::surf(const Mat img, )
{
    Log::Log("The sample requires nonfree module that is not available in your OpenCV distribution.\n");
    throw exception;
    return 0;
}

#else

// Returns an index to the images vector
int ImageMatch::surf(const Mat mat)
{
    // For each slide, get a keypoint matching score
    std::vector<int> scores;

    int minHessian = 400;

    for (auto img : images)
    {
        // Get keypoints
        SurfFeatureDetector detector( minHessian );
        std::vector<KeyPoint> keypoints1, keypoints2;
        detector.detect(mat, keypoints1);
        detector.detect(img, keypoints2);

        // Get descriptors (feature vectors)
        SurfDescriptorExtractor extractor;
        Mat descriptors1, descriptors2;
        extractor.compute(mat, keypoints1, descriptors1);
        extractor.compute(img, keypoints2, descriptors2);

        // Get matches using a FLANN matcher
        FlannBasedMatcher matcher;
        std::vector< DMatch > matches;
        matcher.match(descriptors1, descriptors2, matches);

        //-- Filter matches by distance. They shouldn't have moved very far.
        std::vector<DMatch> good_matches;
        int maxDistance = mat.cols/10;
        for (size_t i = 0; i < matches.size(); i++)
        {
            if (matches[i].distance < maxDistance )
            {
                good_matches.push_back(matches[i]);
            }
        }

        // Record the total number of matches
        scores.push_back(good_matches.size());
    }
  

    Log::Log("\n");
    int bestMatch = -1;
    int bestValue = 0;
    for (int i = 0; i < scores.size(); i++)
    {
        Log::Log(" %d ",);
        if (scores[i] > bestValue)
        {
            bestValue = scores[i];
            bestMatch = i;
        }
    }
    Log::Log("\n");

    return bestMatch;
}

#endif