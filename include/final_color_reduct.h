#include <opencv2/opencv.hpp>

cv::Mat processColorReduct(cv::Mat& img, std::vector<std::pair<cv::Vec3f, float>>& centersWithCount);