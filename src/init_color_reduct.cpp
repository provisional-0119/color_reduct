#include "init_color_reduct.h"

/**
 * 获取图像的rgb三通道直方图
 * img: 输入图像
 * mask: 需要被直方图统计的掩码
*/
cv::Mat getImgHist(cv::Mat& img, cv::Mat& mask) {
    const int bin = 256;
    cv::Mat hist;
    int channels[] = {0, 1, 2};
    int histSize[] = {bin, bin, bin};
    float dranges[] = {0, bin};
    const float* ranges[] = {dranges, dranges, dranges};
    cv::calcHist(&img, 1, channels, mask, hist, 3, histSize, ranges);
    return hist;
}

/**
 * 比较两个初始类心，所在类数量更多的排前面
*/
bool compareCenter(const std::pair<float, cv::Vec3f> & p1, const std::pair<float, cv::Vec3f>& p2) {
    return p1.first > p2.first;
}
/**
 * 比较初始点
*/
bool compareInitCenter(const std::pair<float, cv::Vec3i> & p1, const std::pair<float, cv::Vec3i>& p2) {
    return p1.first > p2.first;
}


/**
 * 对采样的颜色点进行初步聚集
 * img: 输入图像
 * mask: 掩码
 * dh: 采样立方体半直径，默认为32
*/
std::vector<cv::Vec3f> getInitColorSet(cv::Mat& img, cv::Mat& mask, int dh) {
    int color_num = 0;

    // 获取图像的三维直方图
    const int bin = 256;
    int sz[] = {bin, bin, bin};
    cv::Mat hist = getImgHist(img, mask);

    // 构造三通道各自的坐标值乘积 即r * p(r, g, b), b * p(r, g, b), g * p(r, g, b), 以方便后面的矩阵运算
    cv::Mat bs(3, sz, CV_32F), gs(3, sz, CV_32F), rs(3, sz, CV_32F);
    for (int i = 0; i < bin; i ++ ) {
        for (int j = 0; j < bin; j ++ ) {
            for (int k = 0; k < bin; k ++ ) {
                float val = hist.at<float>(i, j, k);
                bs.at<float>(i, j, k) = i * val;
                gs.at<float>(i, j, k) = j * val;
                rs.at<float>(i, j, k) = k * val;
            }
        }
    }

    std::vector<std::pair<float, cv::Vec3i>> initCenters;

    const float eps = 1e-6;
    for (int i = 0; i < bin; i ++ ) {
        for (int j = 0; j < bin; j ++ ) {
            for (int k = 0; k < bin; k ++ ) {
                float cnt = hist.at<float>(i, j, k);
                if (cnt < eps) {
                    continue;
                }
                initCenters.push_back({cnt, cv::Vec3i(i, j, k)});
           }
        }
    }
    // 将个数多的排前面，提高类心质量
    sort(initCenters.begin(), initCenters.end(), compareInitCenter);

    // 是否被标记的掩码
    cv::Mat visited = cv::Mat::zeros(3, sz, CV_8U);
    // 记录每个立方体中心
    std::vector<std::pair<float, cv::Vec3f>> centersWithCount;
    
    for (auto& initCenterWithCount: initCenters) {
        cv::Vec3i initcenter = initCenterWithCount.second;
        
        // 已经遍历过了则跳过
        if (visited.at<uchar>(initcenter)) {
            continue;
        }
        int x = initcenter[0], y = initcenter[1], z = initcenter[2];

        // 获取 半直径为dh的立方体 三个通道矩阵, 计算当前立方体的平均颜色, 并标记
        cv::Range ranges[3] = {};
        for (int i = 0; i < 3; i ++ ) {
            ranges[i] = cv::Range(cv::max(0, initcenter[i] - dh), cv::min(bin, initcenter[i] + dh + 1));
        }
        cv::Mat cube(hist, ranges), bc(bs, ranges), gc(gs, ranges), rc(rs, ranges), subVis(visited, ranges);
        cv::Vec3f center(cv::sum(bc)[0], cv::sum(gc)[0], cv::sum(rc)[0]);
        // 立方体内像素个数
        float cnt = cv::sum(cube)[0];
        // 平均颜色作为初始类心给下一阶段使用
        center /= cnt;
        // 访问置1
        subVis.setTo(1);

        centersWithCount.push_back({cnt, center});
    }

    // 让颜色个数多的中心排前面，以提高后续聚类的准确性
    sort(centersWithCount.begin(), centersWithCount.end(), compareCenter);
    std::vector<cv::Vec3f> centers;
    for (auto centerWithCount : centersWithCount) {
        centers.push_back(centerWithCount.second);
    }

    return centers;
}


