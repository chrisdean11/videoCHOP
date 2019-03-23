#include <opencv2/core.hpp>

class ImageMatch
{
public:
    ImageMatch(std::string dir);
    std::vector<float> getScores(cv::Mat img);
    cv::Mat getBestMatch(cv::Mat img);
    cv::Size getSize() const;
    int getType() const;

private:
    std::vector<cv::Mat> images;

    // Methods to select from
    bool histogram;
    bool text;
};