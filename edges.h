//
// Wrapper around Canny edge detection process.
//

#ifndef BOOKVIEW_CANNY_H
#define BOOKVIEW_CANNY_H

#include <opencv2/core/mat.hpp>

cv::Mat rotateMatrix(cv::Mat in, int degrees);

std::vector<std::vector<cv::Point>> connectedComponents(const cv::Mat& grayImage);

std::vector<std::vector<cv::Point>>
removeShortEdges(cv::Mat grayImage, const std::vector<std::vector<cv::Point>>& components, int threshold);

std::pair<cv::Mat, int> canny(cv::Mat image);

std::pair<int, std::vector<std::vector<cv::Point>>>
findLines(cv::Mat image, const std::vector<std::vector<cv::Point>>& components);

/**
 * Set all elements of a component to 255 on output matrix.
 */
template<class T>
void drawComponent(cv::Mat output, const std::vector<cv::Point>& component, T val) {
    for (auto& point : component) {
        output.at<T>(point.x, point.y) = val;
    }
}

/**
 * Find the index of the maximum value in array.
 */
template <unsigned long N>
int arrayMax(std::array<int, N> arr) {
    int max = 0;
    for (int i = 1; i < arr.size(); i++) {
        if (arr[i] > arr[max])
            max = i;
    }
    return max;
}

#endif //BOOKVIEW_CANNY_H
