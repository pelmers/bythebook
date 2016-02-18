//
// Spine segmentation routines, given an image properly rotated such that spines should lay flat.
//

#ifndef BOOKVIEW_SEGMENT_H
#define BOOKVIEW_SEGMENT_H

#include <opencv2/core/mat.hpp>

std::vector<cv::Range> segmentSpines(cv::Mat& mat);

#endif //BOOKVIEW_SEGMENT_H
