//
// Attempt to isolate text on a detected spine.
// ref: http://web.stanford.edu/~hchen2/papers/ICIP2011_RobustTextDetection.pdf
//

#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "text.h"

using namespace cv;
using std::vector;

static inline bool inRectangle(int x, int y, const Rect& rect, int threshold = 0) {
    return (x + threshold >= rect.x && x - threshold <= rect.x + rect.width
            && y + threshold >= rect.y && y - threshold <= rect.y + rect.height);
}

// Find the bounding box around a vector of rectangles.
static Rect mergeBoxes(const vector<Rect>& rectangles) {
    if (rectangles.size() == 0)
        return Rect(0, 0, 1, 1);
    int xMin = rectangles[0].x, xMax = rectangles[0].x, yMin = rectangles[0].y, yMax = rectangles[0].y;
    for (auto& rect : rectangles) {
        xMin = std::min(rect.x, xMin);
        xMax = std::max(rect.x + rect.width, xMax);
        yMin = std::min(rect.y, yMin);
        yMax = std::max(rect.y + rect.height, yMax);
    }
    return Rect(xMin, yMin, xMax - xMin, yMax - yMin);
}

// Find the bounding box around a vector of rectangles.
static void deleteBG(Mat img, const Rect& rect, int threshold = 20) {
    // Average the four corners to try to get the background.
    int bg = (img.at<uchar>(rect.y, rect.x) + img.at<uchar>(rect.y + rect.height, rect.x) + img.at<uchar>(rect.y, rect.x + rect.width) + img.at<uchar>(rect.y + rect.height, rect.x + rect.height)) / 4;
    for (int y = rect.y; y < rect.y + rect.height; y++) {
        for (int x = rect.x; x < rect.x + rect.width; x++) {
            if (std::abs(img.at<uchar>(y,x) - bg) <= threshold) {
                img.at<uchar>(y, x) = 255;
            }
        }
    }
}

// http://stackoverflow.com/questions/23506105/extracting-text-opencv/23565051#23565051
// TODO: do this better
std::vector<cv::Rect> detectLetters(const cv::Mat& img_gray) {
    std::vector<cv::Rect> boundRect;
    cv::Mat img_sobel, img_threshold, element;
    //cv::Mat img_gray, img_sobel, img_threshold, element;
    //cvtColor(img, img_gray, CV_BGR2GRAY);
    cv::Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
    cv::threshold(img_sobel, img_threshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
    element = getStructuringElement(cv::MORPH_RECT, cv::Size(17, 3) );
    cv::morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element);
    std::vector< std::vector< cv::Point> > contours;
    cv::findContours(img_threshold, contours, 0, 1);
    std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
    for( int i = 0; i < contours.size(); i++ )
        if (contours[i].size()>100)
        {
            cv::approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );
            cv::Rect appRect( boundingRect( cv::Mat(contours_poly[i]) ));
            if (appRect.width>appRect.height)
                boundRect.push_back(appRect);
        }

    return boundRect;
}

Mat isolateText(const Mat& grayImage) {
    std::vector<cv::Rect> boundRects = detectLetters(grayImage);
    Mat newCopy;
    grayImage.copyTo(newCopy);
    // Set everything outside bounding rectangles to 0.
    for (int y = 0; y < grayImage.rows; y++) {
        for (int x = 0; x < grayImage.cols; x++) {
            bool found = false;
            for (auto& rect: boundRects) {
                if (inRectangle(x, y, rect, 50)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                newCopy.at<uchar>(y, x) = 255;
            }
        }
    }
    return newCopy(mergeBoxes(boundRects));
}
