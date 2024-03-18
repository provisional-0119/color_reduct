cmake_minimum_required(VERSION 3.5)
project(color_reduct)

# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(main main.cpp src/sub_sample.cpp src/init_color_reduct.cpp src/edge_smooth.cpp src/mean_shift.cpp
         src/final_color_reduct.cpp src/post_process.cpp)
# add_library(sub_sample SHARED sub_sample.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11" )
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)   # 输出到bin
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libso)



find_package(OpenCV REQUIRED)

TARGET_LINK_LIBRARIES(main ${OpenCV_LIBS})
# TARGET_LINK_LIBRARIES(sub_sample ${OpenCV_LIBS})