#include <opencv2/opencv.hpp>


cv::Mat getImgHist(cv::Mat& img, cv::Mat& mask);

std::vector<cv::Vec3f> getInitColorSet(cv::Mat& img, cv::Mat& mask, int dh = 32);
