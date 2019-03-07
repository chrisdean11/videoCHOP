#include <opencv2/core.hpp>

class ImageMatch
{
public:
    ImageMatch(std::string dir);
    std::vector<float> getScores(cv::Mat img);
    cv::Mat getBestMatch(cv::Mat img);

private:
    std::vector<cv::Mat> images;
    bool histogram;
    bool text;
}