# 颜色减少方法

## 环境

* C++: g++4.8.5(C++11及以上)
* cmake: 3.28.2(3.5及以上)
* opencv: 4.5.4(4.0.0及以上)

## API

1. **edge_smooth.cpp**

   对应论文第II节：EDGE PRESERVING SMOOTHING

   函数声明:

   ```c++
   cv::Mat edgeSmooth(cv:: Mat& img);
   ```

   输入图片，返回被平滑后的图片

2. **sub_sample.cpp**

   对应论文第III-A节：Sub-Sampling

   - 入口函数**subSampling**

     ```c++
     cv::Mat subSampling(const cv::Mat& img);
     ```

     输入被 `edgeSmooth` 处理过的图片，返回采样的掩码。

   - 其他被调用函数

     - **getG**

       ```c++
       cv::Mat getG(const cv::Mat& img);
       ```

       由 `subSampling` 调用，返回论文中第III-A节的 `G(x,y)` 组成的矩阵

     - **subMask**

       ```c++
       cv::Mat subMask(const cv::Mat& G);
       ```

       对 `getG` 返回的矩阵处理，返回采样点的掩码(1代表被采样，0代表不采样)

3. **init_color_reduct.cpp**

   对应论文第III-B节： Initial Color Reduction

   - 入口函数**getInitColorSet**

     ```c++
     std::vector<cv::Vec3f> getInitColorSet(cv::Mat& img, cv::Mat& mask, int dh);
     ```

     参数:

     - `img`: 输入 `edgeSmooth` 函数处理后的图像
     - `mask`: 输入 `subSampling` 函数获得的采样点掩码
     - `dh`: 论文中所提到的参数h, 头文件中声明了默认值为32
     - 返回值: 返回获得的立方体加权平均中心集合，对应论文的 S<sub>m</sub>

   - 其他主要被调用函数

     根据掩码获取采样点的直方图，对应论文的p(r,g,b)

     ```c++
     cv::Mat getImgHist(cv::Mat& img, cv::Mat& mask);
     ```

     参数:

     - `img`: 输入 `edgeSmooth` 函数处理后的图像
     - `mask`: 输入 `subSampling` 函数获得的采样点掩码

4. **mean_shift.cpp**

   对应论文第III-C节： Mean-Shift Procedure

   主要函数**meanShift**

   ```c++
   std::vector<std::pair<cv::Vec3f, float>> meanShift(cv::Mat& hist, std::vector<cv::Vec3f>& centers, int h, int dm);
   ```

   参数:

   - `hist`：使用 `init_color_reduct.cpp` 中 `getImgHist` 函数获得的采样点直方图
   - `centers`：上一阶段`getInitColorSet` 函数返回的初始集合S<sub>m</sub>
   - `h`: 论文中的参数h，默认值32
   - `dm`: 论文中的参数d<sub>m</sub>, 默认值3
   - 返回值: 返回带聚类点个数的聚类中心集合

5. **final_color_reduct.cpp**

   对应论文第IV节：FINAL COLOR REDUCTION

   - 入口函数**processColorReduct**

     ```c++
     cv::Mat processColorReduct(cv::Mat& img, std::vector<std::pair<cv::Vec3f, float>>& centersWithCount);
     ```

     参数:

     - `img`: 输入 `edgeSmooth` 函数处理后的图像
     - `centersWithCount`: `meanShift` 返回的带聚类点个数的聚类中心集合
     - 返回值: 颜色减少后的图像

   - 其他主要被调用函数

     - `mergeClusterCenter` : 合并`centersWithCount` 中切比雪夫距离小于h的中心

       ```c++
       std::vector<cv::Vec3f> mergeClusterCenter(std::vector<std::pair<cv::Vec3f, float>>& centersWithCount, int h = 32);
       ```

     - `findClosestCenter` : 为输入的像素点找到切比雪夫距离最近的中心

       ```c++
       cv::Vec3b findClosestCenter(cv::Vec3b pixel, const std::vector<cv::Vec3f>& centers);
       ```

6. **post_process.cpp**

   对应论文第V节：POST PROCESSING

   ```c++
   cv::Mat postProcess(cv::Mat& img);
   ```

   参数:

   - `img`: 输入`final_color_reduct.cpp` 中`processColorReduct` 函数返回的图像，即已进行颜色减少处理的图像
   - 返回: 返回使用第V节后处理方法后的图像

## 运行

- 将 `main` 函数中 `path` 和 `filenames` 换成想要处理的图片文件夹路径，和图片名称

- 进入build文件夹，使用cmake生成MakeFile等文件后返回主文件夹，运行存储在bin文件夹下的main可执行文件

  ```shell
  cd build
  cmake ..
  make
  cd ..
  ./bin/main
  ```

- main函数

  - `colorReduct` 函数

    ```c++
    cv::Mat colorReduct(cv::Mat img);
    ```

    输入图像返回颜色减少后的图像

  - `work` 函数

    ```c++
    void work(cv::Mat img, std::string name);
    ```

    参数: 

    - `img`: 原图像
    - `name`: 图像文件名

    最终输出图片与 `colorReduct` 函数一样, 但是会输出各个阶段的运行时间，以及平滑后图像，采样图，灰度图等信息，有利于调试。

## 结果

- 若运行`colorReduct` 函数，则结果存储在`5_postProcess` 文件夹下
- 若运行`work` 函数，则中间各个过程分别存储至`1_edgeSmooth`、 `2_gradient`、 `3_subSample`、 `4_finalColor`、 `5_postProcess` 。