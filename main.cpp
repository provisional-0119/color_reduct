#include <opencv2/opencv.hpp>
#include "sub_sample.h"
#include "init_color_reduct.h"
#include "edge_smooth.h"
#include "mean_shift.h"
#include "final_color_reduct.h"
#include "post_process.h"

// 返回颜色减少后的结果，无运行时间和中心过程图片
cv::Mat colorReduct(cv::Mat img) {
    img = edgeSmooth(img);
    cv::Mat mask = subSampling(img);
    std::vector<cv::Vec3f> centers = getInitColorSet(img, mask);
    cv::Mat hist = getImgHist(img, mask);
    std::vector<std::pair<cv::Vec3f, float>> centersWithCount = meanShift(hist, centers);
    cv::Mat finalImg = processColorReduct(img, centersWithCount);
    cv::Mat postImg = postProcess(finalImg);
    return postImg;
}

// 带运行时间信息和中心过程图片
void work(cv::Mat img, std::string name) {
    std::cout << "处理图片:" << name << '\n';

    clock_t start_time, end_time;

    // 平滑处理
    start_time = clock();
    int loop = 1;
    for (int i = 0; i < loop; i ++ ) {
        img = edgeSmooth(img);
    }
    
    end_time = clock();
    std::cout << "边缘保持平滑运行时间:" << (double)(end_time - start_time) / CLOCKS_PER_SEC << "s\n"; 

    cv::imwrite("1_edgeSmooth/" + name, img);

    // 采样
    start_time = clock();
    cv::Mat mask = subSampling(img);
    end_time = clock();
    std::cout << "采样运行时间:" << (double)(end_time - start_time) / CLOCKS_PER_SEC << "s\n"; 

    // 灰度图
    cv::Mat gradient = getG(img);
    cv::imwrite("2_gradient/" + name, gradient);

    // 采样图
    cv::Mat sub = img.clone();
    sub.setTo(cv::Scalar(255, 255, 255), mask == 0);
    cv::imwrite("3_subSample/" + name, sub);

    // 初步减少颜色数量
    start_time = clock();
    std::vector<cv::Vec3f> centers = getInitColorSet(img, mask);
    end_time = clock();
    std::cout << "初始颜色减少运行时间:" << (double)(end_time - start_time) / CLOCKS_PER_SEC << "s\n"; 
    

    // mean-shift聚类减少颜色数量
    start_time = clock();
    cv::Mat hist = getImgHist(img, mask);

    std::cout << "采样个数:" << cv::countNonZero(mask) << '\n';

    std::vector<std::pair<cv::Vec3f, float>> centersWithCount = meanShift(hist, centers);
    end_time = clock();
    std::cout << "mean-shift运行时间:" << (double)(end_time - start_time) / CLOCKS_PER_SEC << "s\n"; 

    // 最终色彩分类
    start_time = clock();
    cv::Mat finalImg = processColorReduct(img, centersWithCount);
    end_time = clock();
    std::cout << "最终色彩分类运行时间:" << (double)(end_time - start_time) / CLOCKS_PER_SEC << "s\n"; 

    cv::imwrite("4_finalColor/" + name, finalImg);

    // 后处理
    start_time = clock();
    cv::Mat postImg = postProcess(finalImg);
    end_time = clock();
    std::cout << "后处理运行时间:" << (double)(end_time - start_time) / CLOCKS_PER_SEC << "s\n"; 

    cv::imwrite("5_postProcess/" + name, postImg);

    std::cout << "\n\n";
}

int main() {
    std::string path = "img/";
    std::string filenames[] = {"1.png", "2.png", "3.png", "4.png", "5.png", "6.png", "7.png",
            "8.png", "9.png", "10.png", "11.png", "12.png"};
    // std::string filenames[] = {"13.png"};

    for (auto &filename: filenames) {
        cv::Mat img = cv::imread(path + filename);
        if (img.empty()) {
            std::cout << "The picture is empty!" << std::endl;
        } else {
            // cv::Mat result = colorReduct(img);
            // cv::imwrite("5_postProcess/" + filename, result);
            
            work(img, filename);
        }
    }
    
    return 0;
}
