//
// Perform edge and line detection.
//

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <array>
#include <stack>
#include "edges.h"

using namespace cv;
using std::vector;

static int median(Mat grayImage) {
    std::array<int, 256> histogram;
    histogram.fill(0);
    for (int i = 0; i < grayImage.rows; i++) {
        auto row = grayImage.ptr(i);
        for (int j = 0; j < grayImage.cols; j++) {
            histogram[row[j]]++;
        }
    }
    int target = grayImage.rows * grayImage.cols / 2;
    int runningTotal = 0;
    for (int i = 0; i < histogram.size(); i++) {
        runningTotal += histogram[i];
        if (runningTotal > target)
            return i;
    }
    return 255;
}

/**
 * Trace the edge that includes row, col. Assume image[row, col] > 0. For each visited x,y, set image[x, y] = 0.
 * Remark: each pixel has 8 neighbors.
 * Return a vector of the positions of all pixels on the edge.
 */
static std::vector<Point> traceEdge(Mat image, int row, int col) {
    auto stack = std::stack<Point>();
    auto neighbors = std::vector<Point>();
    stack.push(Point{row, col});
    image.at<uchar>(row, col) = 0;
    while (stack.size() > 0) {
        auto pos = stack.top();
        stack.pop();
        neighbors.push_back(pos);
        for (int i = std::max(0, pos.x - 1); i < std::min(image.rows, pos.x + 2); i++) {
            for (int j = std::max(0, pos.y - 1); j < std::min(image.cols, pos.y + 2); j++) {
                if (image.at<uchar>(i, j) > 0) {
                    stack.push(Point{i, j});
                    image.at<uchar>(i, j) = 0;
                }
            }
        }
    }
    return neighbors;
}

/**
 * Compute a bin for the angle in range [0, pi].
 */
static inline int bin(double theta, int nbins) {
    return (int) (theta / ((CV_2PI) / nbins));
}

/**
 * Find the angle of a component, return an int in range [0, 180], where 0 means vertical, 90 means horizontal.
 */
static inline int componentAngle(const vector<Point>& component) {
    // Use opencv fitLine to obtain the vector that fits the component best.
    Vec4f outLine;
    fitLine(component, outLine, CV_DIST_L2, 0, 0.01, 0.01);
    int angle = (int) (180 / CV_PI * std::atan(outLine[0] / outLine[1]));
    return (angle >= 0) ? angle: angle+180;
}

/**
 * Find the difference between angles a and b, which are degrees in [0, 180].
 * Result is in [0, 90].
 */
static inline int angleDiff(int a, int b) {
    int hi = std::max(a, b);
    int lo = std::min(a, b);
    int diff = hi - lo;
    if (diff <= 90) {
        return diff;
    } else {
        return std::abs(hi - 180 - lo);
    }
}

Mat rotateMatrix(Mat in, int degrees) {
    Point2f center(in.cols/2.0F, in.rows/2.0F);
    Mat rot = getRotationMatrix2D(center, degrees, 1.0);
    cv::Rect bbox = cv::RotatedRect(center,in.size(), degrees).boundingRect();
    rot.at<double>(0,2) += bbox.width/2.0 - center.x;
    rot.at<double>(1,2) += bbox.height/2.0 - center.y;
    Mat out;
    int diag = (int) std::sqrt(in.rows * in.rows + in.cols * in.cols);
    // Ensure the rotate causes no cropping.
    //warpAffine(in, out, rotationMatrix, Size(diag, diag));
    warpAffine(in, out, rot, Size(in.rows, in.cols));
    return out;
}

/**
 * Create a vector of all the connected components (i.e. edges) of an image.
 */
std::vector<std::vector<Point>> connectedComponents(const Mat& grayImage) {
    // Construct a copy of the image for bookkeeping.
    std::vector<std::vector<Point>> edgesOutput;
    Mat cpy = grayImage.clone();
    for (int i = 0; i < cpy.rows; i++) {
        const uchar *row = cpy.ptr<uchar>(i);
        for (int j = 0; j < cpy.cols; j++) {
            if (row[j] > 0) {
                edgesOutput.push_back(traceEdge(cpy, i, j));
            }
        }
    }
    return edgesOutput;
}

/**
 * Set the elements of short edges (connected components of size < threshold) to zero of a grayImage, i.e. Mat<uchar>.
 * Return a vector of the remaining edges, all those whose length exceeds threshold.
 */
std::vector<std::vector<Point>> removeShortEdges(Mat grayImage, const std::vector<std::vector<Point>>& components, int threshold) {
    std::vector<std::vector<Point>> longEdges;
    for (auto& edge: components) {
        if (edge.size() > 0 && edge.size() < threshold) {
            for (auto &pos : edge) {
                grayImage.at<uchar>(pos.x, pos.y) = 0;
            }
        } else {
            longEdges.push_back(edge);
        }
    }
    return longEdges;
}

// Idea: run Canny on each channel of the matrix?
std::pair<Mat, int> canny(Mat image) {
    // Apply blurring.
    GaussianBlur(image, image, Size(3, 3), 2);
    Mat cannyOutput(image.size(), CV_8U);
    Mat linesOutput(image.size(), CV_8U);
    // Perform Canny edge detection with computed parameters.
    int v = median(image);
    Canny(image, cannyOutput, std::max(0.0, 0.33 * v), std::min(255.0, 1.33 * v), 3, true);
    imshow("edges", cannyOutput);
    // Filter out small components.
    auto components = connectedComponents(cannyOutput);
    components = removeShortEdges(cannyOutput, components, 120);
    auto out = findLines(cannyOutput, components);
    for (auto& line: std::get<1>(out)) {
        drawComponent<uchar>(linesOutput, line, 255);
    }
    imshow("no short", linesOutput);
    return std::make_pair(linesOutput, std::get<0>(out));
}

/**
 * Find the long straight lines corresponding to the most frequent peak of a Hough transform.
 * Return the dominant angle, in degrees, and all the connected components that roughly align with this angle.
 */
std::pair<int, vector<vector<Point>>> findLines(Mat image, const vector<vector<Point>>& components) {
    constexpr int nbins = 30; // must evenly divide 90.
    vector<Vec2f> houghLines;
    vector<vector<Point>> lineComponents;
    std::array<int, nbins> histogram;
    histogram.fill(0);
    HoughLines(image, houghLines, 1, CV_PI / 180, 100);
    // We bin the lines' angles into a histogram and find the one with the most matches.
    // I assume that we will find this dominant angle somewhere +-15 degrees from vertical.
    for (Vec2f &points : houghLines) {
        // FIXME??
        int deg = (int) (points[1] * 180 / CV_PI);
        if (angleDiff(deg, 0) <= 15 || angleDiff(deg, 180) <= 15)
            histogram[bin(points[1], nbins)]++;
    }
    int angle = (360 / nbins) * arrayMax(histogram) + 90;
    // Now we find the connected components which share this angle.
    // 30 degrees around dominant angle, according to Chen et. al is a good range.
    for (auto& component : components) {
        if (angleDiff(componentAngle(component), angle) <= 15) {
            lineComponents.push_back(component);
        }
    }
    return std::make_pair(angle, lineComponents);
}
