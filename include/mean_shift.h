#include <opencv2/opencv.hpp>

std::vector<std::pair<cv::Vec3f, float>> meanShift(cv::Mat& hist, std::vector<cv::Vec3f>& centers, int h = 32, int dm = 3);