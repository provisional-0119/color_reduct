#include "edge_smooth.h"

/**
 * 采用矩阵运算优化
*/


/**
 * 对图像进行平滑处理，消除突出噪音
*/
cv::Mat edgeSmooth(cv:: Mat& img) {
    int h = img.rows, w = img.cols;
    // 在周围扩展一层, 以使得平滑后图片大小不变
    cv::Mat extendImg;
    cv::copyMakeBorder(img, extendImg, 1, 1, 1, 1, cv::BORDER_REPLICATE);


    cv::Mat result = cv::Mat(cv::Size(w, h), CV_8UC3);

    // 获取八个方向和中心的偏移矩阵，用于计算di
    cv::Mat shift[3][3];

    for (int i = 0; i < 3; i ++ ) {
        for (int j = 0; j < 3; j ++ ) {
            shift[i][j] = extendImg(cv::Range(i, i + h), cv::Range(j, j + w));
            // 转为32位浮点数类型，防止运算溢出
            shift[i][j].convertTo(shift[i][j], CV_32FC3);
        }
    }

    // 求取di矩阵
    // di = (|ri - rj| + |bi - bj| + |gi - gj|) / (3 * 255)
    cv::Mat d[3][3];
    for (int i = 0; i < 3; i ++ ) {
        for (int j = 0; j < 3; j ++ ) {
            // 中心跳过
            if (i == 1 && j == 1) continue;
            // 对每个矩阵减去中心矩阵，并取绝对值相加
            d[i][j] = cv::abs(shift[i][j] - shift[1][1]);
            std::vector<cv::Mat> channels;
            cv::split(d[i][j], channels);
            
            cv::Mat sd = cv::Mat::zeros(img.size(), CV_32F);
            for (int k = 0; k < 3; k ++ ) {
                sd += channels[k];
            }
            sd /= 3 * 255.0;
            d[i][j] = sd;
        }
    }


    // 计算ci
    // ci = (1 - di) ^ p
    // p: 默认为10
    cv::Mat c[3][3] = {};
    const int p = 10;

    cv::Mat cSum = cv::Mat::zeros(img.size(), CV_32F);

    for (int i = 0; i < 3; i ++ ) {
        for (int j = 0; j < 3; j ++ ) {
            // 矩阵初始化，用于存储后面的结果
            c[i][j] = cv::Mat::zeros(img.size(), CV_32F);
            // 中心跳过
            if (i == 1 && j == 1) continue;
            cv::pow(1 - d[i][j], p, c[i][j]);
            cSum += c[i][j];
        }
    }
    
    // 求取C矩阵 并将其作为滤波的卷积核
    // C = [[c1, c2, c3], [c4, 0, c5], [c6, c7, c8]] / (c1 + c2 + ... + c8)
    const float eps = 1e-6;
    for (int i = 0; i < 3; i ++ ) {
        for (int j = 0; j < 3; j ++ ) {
            // 中心跳过
            if (i == 1 && j == 1) continue;
            
            cv::divide(c[i][j], cSum, c[i][j]);
            // 将除数为0的非零结果置为0
            // c[i][j].setTo(0, cSum < eps);
        }
    }

    // 使用C矩阵对每个通道进行卷积，平滑图像
    std::vector<cv::Mat> channels = {cv::Mat::zeros(img.size(), CV_32F), 
        cv::Mat::zeros(img.size(), CV_32F), cv::Mat::zeros(img.size(), CV_32F)};

    for (int i = 0; i < 3; i ++ ) {
        for (int j = 0; j < 3; j ++ ) {

            std::vector<cv::Mat> shiftChannels;
            cv::split(shift[i][j], shiftChannels);
            
            for (int k = 0; k < 3; k ++ ) {
                channels[k] += shiftChannels[k].mul(c[i][j]);
            }
        }
    }

    // 合并通道
    for (int i = 0; i < 3; i ++ ) {
        channels[i].convertTo(channels[i], CV_8U);
    }
    cv::Mat smoothImage;
    cv::merge(channels, smoothImage);
    
    return smoothImage;
}


