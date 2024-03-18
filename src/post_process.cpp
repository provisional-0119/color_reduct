#include "sub_sample.h"

// 曼哈顿距离
int manhattanDistance(const cv::Vec3i& v1, const cv::Vec3i& v2) {
    int dx = cv::abs(v1[0] - v2[0]);
    int dy = cv::abs(v1[1] - v2[1]);
    int dz = cv::abs(v1[2] - v2[2]);
    
    return dx + dy + dz;
}


/**
 * 对图像进行后处理，将边缘处容易聚类错的像素点修正
*/
cv::Mat postProcess(cv::Mat& img) {
    // 获取图像边缘强度
    cv::Mat G = getG(img);

    cv::Mat postImg = img.clone();
    int h = img.rows, w = img.cols;

    // 下、右、下右、上右 四个方向
    int dir[][2] = {{1, 0}, {0, 1}, {1, 1}, {-1, 1}};
    for (int i = 1; i + 1 < h; i ++ ) {
        for (int j = 1; j + 1 < w; j ++ ) {

            int max_diff_dir = -1, maxValue = -1;

            // 遍历四个方向，获取像素点曼哈顿距离最大的一对
            for (int k = 0; k < 4; k ++ ) {
                int dx = dir[k][0], dy = dir[k][1];

                // 使用cv::Vec3i接受防止运算溢出
                cv::Vec3i p1 = img.at<cv::Vec3b>(i + dx, j + dy);
                cv::Vec3i p2 = img.at<cv::Vec3b>(i - dx, j - dy);
                int dis = manhattanDistance(p1, p2);
                if (dis > maxValue) {
                    maxValue = dis;
                    max_diff_dir = k;
                }
            }

            // 取出选取到的距离最大的一对点及其方向
            int dx = dir[max_diff_dir][0], dy = dir[max_diff_dir][1];
            cv::Vec3b p1 = img.at<cv::Vec3b>(i + dx, j + dy);
            cv::Vec3b p2 = img.at<cv::Vec3b>(i - dx, j - dy);
            cv::Vec3b center = img.at<cv::Vec3b>(i, j);

            // 选取颜色距离更近的像素点，若该像素点边缘强度小于中心像素点，用其替换中心像素点
            if (manhattanDistance(p1, center) <= manhattanDistance(p2, center)) {
                if (G.at<float>(i, j) > G.at<float>(i + dx, j + dy)) {
                    postImg.at<cv::Vec3b>(i, j) = p1;
                }
            } else {
                if (G.at<float>(i, j) > G.at<float>(i - dx, j - dy)) {
                    postImg.at<cv::Vec3b>(i, j) = p2;
                }
            }
        }
    }

    return postImg;
}
