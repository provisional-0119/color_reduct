#include "mean_shift.h"


// 根据shift判断是否收敛
bool checkShiftConverge(cv::Vec3f shift, int dm) {
    double s = 0;
    for (int i = 0; i < 3; i ++ ) {
        s += cv::abs(shift[i]);
    }
    return s <= dm;
}

/**
 * hist: 图像bgr三通道直方图
 * centers: 初始聚类中心
 * h: 聚类中心到其他被聚类点的最大立方半径, 默认为32
 * dm: meanShift收敛的阈值, 默认为3
*/
std::vector<std::pair<cv::Vec3f, float>> meanShift(cv::Mat& hist, std::vector<cv::Vec3f>& centers, int h, int dm) {

    // 构造三通道各自的坐标值乘积 即r * p(r, g, b), b * p(r, g, b), g * p(r, g, b), 以方便后面的矩阵运算
    const int bin = 256;
    int sz[] = {bin, bin, bin};

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

    // 用于标记是否还未被访问，没访问的值则可参与计算
    cv::Mat mask = cv::Mat::ones(3, sz, CV_8U);
    // 存储聚类中心
    std::vector<std::pair<cv::Vec3f, float>> centersWithCount;

    // 用于浮点数衡量是否为0；
    const float eps = 1e-6;
    
    for (cv::Vec3f& center: centers) {
        // 记录当前类的点数，用于代表整个类
        float allCount = 0.0;

        // if (!mask.at<uchar>(static_cast<cv::Vec3i>(center))) {
        //     continue;
        // }

        // mask.setTo(1);

        while (true) {
            // 截取对应的立方体大小切片

            cv::Vec3i ci = static_cast<cv::Vec3i>(center);

            // std::cout << center << ' ' << ci << '\n';

            cv::Range ranges[3] = {};
            for (int i = 0; i < 3; i ++ ) {
                ranges[i] = cv::Range(cv::max(0, ci[i] - h), cv::min(bin, ci[i] + h + 1));
            }
            cv::Mat cube(hist, ranges), bc(bs, ranges), gc(gs, ranges), rc(rs, ranges);

            // 经过试验发现不去除已访问颜色聚类中心更合适
            
            // // // 乘上掩码，去除已访问颜色
            // cv::Mat subMask(mask, ranges), floatSubMask;
            // subMask.convertTo(floatSubMask, CV_32F);
            // cube = cube.mul(floatSubMask);
            // bc = bc.mul(floatSubMask);
            // gc = gc.mul(floatSubMask);
            // rc = rc.mul(floatSubMask);

            // // 掩码置0
            // subMask.setTo(0);

            // 计算新的聚类中心
            float cnt = cv::sum(cube)[0];

            // 若当前聚类中心无像素点，则跳过
            if (cnt < eps) {
                break;
            }
            cv::Vec3f newCenter(cv::sum(bc)[0], cv::sum(gc)[0], cv::sum(rc)[0]);
            newCenter = (newCenter + center * allCount) / (cnt + allCount);
            allCount += cnt;
            

            cv::Vec3f shift = newCenter - center;

            // 更新聚类中心
            center = newCenter;
            // 已收敛
            if (checkShiftConverge(shift, dm)) {
                centersWithCount.push_back({center, allCount});
                break;
            }
        }
    }


    return centersWithCount;
}




