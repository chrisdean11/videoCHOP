#include "Log.h"
#include "ImageMatch.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "opencv2/opencv_modules.hpp"
# include "opencv2/core/core.hpp"
# include "opencv2/features2d/features2d.hpp"
# include "opencv2/highgui/highgui.hpp"
//# include "opencv2/nonfree/features2d.hpp"

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

    // Calculate keypoints and descriptors
    minHessian = 800;
    for (int i = 0; i < count; i++)
    {
        Ptr<ORB> detector = ORB::create(800);
        std::vector<KeyPoint> img_keypoints;
        Mat img_descriptors;
        detector->detectAndCompute(images[i], noArray(), img_keypoints, img_descriptors);
        keypoints.push_back(img_keypoints);
        descriptors.push_back(img_descriptors);

        Log::Log("%d keypoints\n", (int)img_keypoints.size());
    }
}

// Return a list of match scores. Perhaps this will be a different score for every method.
std::vector<float> ImageMatch::getScores(Mat img) const
{
    std::vector<float> ret;
    return ret;
}

Mat ImageMatch::getBestMatch(Mat img) const
{
    return images[orb(img)];
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

// Returns an index to the images vector
int ImageMatch::orb(const Mat mat) const
{
    // For each slide, get a keypoint matching score
    std::vector<int> scores;
    
    // Get keypoints and descriptors from input image
    Ptr<ORB> detector = ORB::create(minHessian);
    std::vector<KeyPoint> mat_keypoints;
    Mat mat_descriptors;
    detector->detectAndCompute(mat, noArray(), mat_keypoints, mat_descriptors);

    std::vector<std::vector<DMatch>> kept_matches;

    for (uint i = 0; i < images.size(); ++i)
    {
        // Get matches
        Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE);
        std::vector<DMatch> matches;
        matcher->match(mat_descriptors, descriptors[i], matches);

        // Filter matches by distance. They shouldn't have moved very far.
        std::vector<DMatch> good_matches;
        int maxDistance = 200;
        for (size_t j = 0; j < matches.size(); j++)
        {
            if (matches[j].distance < maxDistance )
            {
                good_matches.push_back(matches[j]);
            }
        }

        // Record the total number of matches
        scores.push_back(good_matches.size());
        kept_matches.push_back(good_matches);
    }

    Log::Log("\n");
    int bestMatch = 0;
    int bestValue = 0;
    for (uint i = 0; i < scores.size(); i++)
    {
        Log::Log(" %d ", scores[i]);

        if (scores[i] > bestValue)
        {
            bestValue = scores[i];
            bestMatch = i;
        }
    }
    Log::Log("\n");

    // Draw matches
    Mat img_matches;
    drawMatches( mat, mat_keypoints, images[bestMatch], keypoints[bestMatch], kept_matches[bestMatch], img_matches, Scalar::all(-1),
                 Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    // Show detected matches
    imshow("Good Matches", img_matches );
    waitKey(50);

    return bestMatch;
}