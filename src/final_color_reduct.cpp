#include "final_color_reduct.h"


// 切比雪夫距离
float chebyshevDistance(const cv::Vec3f& v1, const cv::Vec3f& v2) {
    float dx = cv::abs(v1[0] - v2[0]);
    float dy = cv::abs(v1[1] - v2[1]);
    float dz = cv::abs(v1[2] - v2[2]);
    
    return cv::max({dx, dy, dz});
}


/**
 * 输入两个带聚类点数的中心，返回一个新的合并中心
*/
std::pair<cv::Vec3f, float> mergeTwoCenters(std::pair<cv::Vec3f, float> c1, std::pair<cv::Vec3f, float> c2) {
    float sumCount = c1.second + c2.second;
    cv::Vec3f center = (c1.first * c1.second + c2.first * c2.second) / sumCount;
    return std::make_pair(center, sumCount);
}

/**
 * 合并切比雪夫距离小于h的聚类中心
 * centersWithCount: 存储类心及其聚类时的成员个数
 * h: 默认为32
*/
std::vector<cv::Vec3f> mergeClusterCenter(std::vector<std::pair<cv::Vec3f, float>>& centersWithCount, int h = 32) {

    std::vector<std::pair<cv::Vec3f, float>> newCenters;
    bool merged = true;
    // 由于合并中可能产生新的可合并中心，则进行循环直至没有可合并中心
    while (merged) {
        merged = false;
        for (auto center: centersWithCount) {
            // 查看是否有切比雪夫距离小于 h的聚类中心，如果有，则合并
            bool nowMerged = false;
            for (int i = 0; i < newCenters.size(); i ++ ) {
                std::pair<cv::Vec3f, float>& newCenter = newCenters[i];
                if (chebyshevDistance(newCenter.first, center.first) < h) {
                    newCenter = mergeTwoCenters(newCenter, center);
                    nowMerged = true;
                    merged = true;
                }
            }
            if (!nowMerged) {
                newCenters.push_back(center);
            }   
        }

        // 产生了新的中心, 则继续合并
        if (merged) {
            centersWithCount = newCenters;
            newCenters.clear();
        }
    }
    
    // 处理结果返回
    std::vector<cv::Vec3f> centers(newCenters.size());
    for (int i = 0; i < newCenters.size(); i ++ ) {
        centers[i] = newCenters[i].first;
    }

    return centers;
}


/**
 * 找到对应颜色的最近聚类中心
*/
cv::Vec3b findClosestCenter(cv::Vec3b pixel, const std::vector<cv::Vec3f>& centers) {
    float minDistance = 1e4;
    cv::Vec3f closestCenter;

    // 遍历中心，找到切比雪夫距离最近的聚类中心
    for (auto center: centers) {
        float dis = chebyshevDistance(center, pixel);
        if (dis < minDistance) {
            minDistance = dis;
            closestCenter = center;
        }
    }
    return static_cast<cv::Vec3b>(closestCenter);
}

/**
 * 对图像进行处理，将图像色彩归为各个聚类
*/
cv::Mat processColorReduct(cv::Mat& img, std::vector<std::pair<cv::Vec3f, float>>& centersWithCount) {

    // 合并前的点
    // for (auto centerWithCount: centersWithCount) {
    //     std::cout << centerWithCount.first << '\n';
    // }

    std::vector<cv::Vec3f> centers = mergeClusterCenter(centersWithCount);

    // 合并后的点
    // std::cout << "合并类心后\n";
    // for (auto center: centers) {
    //     std::cout << center << '\n';
    // }

    int h = img.rows, w = img.cols;
    for (int i = 0; i < h; i ++ ) {
        for (int j = 0; j < w; j ++ ) {
            cv::Vec3b& pixel = img.at<cv::Vec3b>(i, j);
            // 对每个像素点以切比雪夫距离找到最近的聚类中心
            pixel = findClosestCenter(pixel, centers);
        }
    }

    return img;
}