#include "sub_sample.h"

/**
 * 计算单个通道G值 
 * Gc = sqrt(Grow^2 + Gcol^2)
*/
cv::Mat getGchannel(const cv::Mat& channel) {
    cv::Mat gx, gy;
    // 获取横纵方向上的Sobel
    cv::Sobel(channel, gx, CV_32F, 1, 0, 3);
    cv::Sobel(channel, gy, CV_32F, 0, 1, 3);
    cv::Mat gradient_channel;
    cv::sqrt(gx.mul(gx) + gy.mul(gy), gradient_channel);
    return gradient_channel;
}

/**
 * 获取 对应梯度灰度图
*/
cv::Mat getG(const cv::Mat& img) {
    std::vector<cv::Mat> channels;
    cv::split(img, channels);
    

    cv::Mat b = channels[0];
    cv::Mat g = channels[1];
    cv::Mat r = channels[2];
    
    cv::Mat G = cv::max(cv::max(getGchannel(b), getGchannel(g)), getGchannel(r));

    return G;
}


/**
 * 获取对于某个方向上的偏移切片
 * 例如 dx = -1即  取(0, len - 1), (1, len)进行比较
*/
void getSlice(int dx, int len, cv::Range& sli, cv::Range& slice_d) {

    if (dx > 0) {
        sli = cv::Range(dx, len);
        slice_d = cv::Range(0, len - dx);
    } else if (dx < 0) {
        sli = cv::Range(0, len + dx);
        slice_d = cv::Range(-dx, len);
    } else {
        sli = cv::Range::all();
        slice_d = cv::Range::all();
    }
}

/**
 * 获取要采样的像素点掩码
*/
cv::Mat subMask(const cv::Mat& G) {
    std::vector<cv::Point> directions = {{0, -1}, {0, 1}, {1, 0}, {-1, 0}, {-1, 1}, {-1, -1}, {1, 1}, {1, -1}};

    cv::Mat mask = cv::Mat::ones(G.size(), CV_8U);

    // 对于每个像素点和周围八个方向的大小进行处理，找出像素点的值<=周围所有值的掩码
    for (const auto& direction : directions) {
        int dx = direction.x;
        int dy = direction.y;
        cv::Range slice_x, slice_dx, slice_y, slice_dy;

        // 获取某个方向的位移切片
        getSlice(dx, G.rows, slice_x, slice_dx);
        getSlice(dy, G.cols, slice_y, slice_dy);

        cv::Mat G_slice_dx_dy = G(slice_dx, slice_dy);
        cv::Mat G_slice_x_y = G(slice_x, slice_y);

        // 对这部分的掩码进行更新
        cv::Mat mask_slice_x_y = mask(slice_x, slice_y);
        cv::Mat result;
        cv::compare(G_slice_dx_dy, G_slice_x_y, result, cv::CMP_GE);
        cv::bitwise_and(result, mask_slice_x_y, mask_slice_x_y);

    }

    return mask;
}



/**
 * 进行像素点采样
 * img: 输入的图像
 * 
 * return: 返回该图像采样像素点对应的掩码 
*/
cv::Mat subSampling(const cv::Mat& img) {

    cv::Mat G = getG(img);
    
    cv::Mat mask = subMask(G);
    
    // cv::Mat sub = img.clone();
    
    // sub.setTo(cv::Scalar(255, 255, 255), mask == 0);
    
    return mask;
}
