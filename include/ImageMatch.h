#include <opencv2/core.hpp>
/*
    Compare a test image against a set of samples.
    Create with the directory containing the sample images. 
    The contents of that directory are loaded into memory. This class handles all file I/O.

    TODO: 
        getBestMatch() should return a Mat of the same size and type as the input image.
        'images' should be a set of pointers to images of any size
 */
class ImageMatch
{
public:
    ImageMatch(std::string dir);
    std::vector<float> getScores(cv::Mat img) const;
    cv::Mat getBestMatch(cv::Mat img) const;

    // Information about the images getBestMatch() will return
    cv::Size getSize() const;
    int getType() const;

private:
    int orb(const cv::Mat mat) const;

    int minHessian;
    std::vector<cv::Mat> images;
    std::vector<std::vector<cv::KeyPoint>> keypoints;
    std::vector<cv::Mat> descriptors;
};