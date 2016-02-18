//
// Text isolation routines.
//

#ifndef BOOKVIEW_TEXT_H
#define BOOKVIEW_TEXT_H

#include <opencv2/core/mat.hpp>

cv::Mat isolateText(const cv::Mat& grayImage);

#endif //BOOKVIEW_TEXT_H
