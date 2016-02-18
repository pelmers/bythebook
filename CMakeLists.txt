cmake_minimum_required(VERSION 3.3)
project(by-the-book)

set(OpenCV_DIR /usr/local/Cellar/opencv3/3.1.0/share/OpenCV)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp edges.cpp segment.cpp text.cpp)
add_executable(spinecrop ${SOURCE_FILES})
target_link_libraries(spinecrop ${OpenCV_LIBS})
