#include <opencv2/opencv.hpp>
#include "edges.h"
#include "segment.h"
#include "text.h"

using namespace std;
using namespace cv;

/**
 * The plan:
 * 1. Detect edges
 * 2. Filter out short edges
 * (TODO 3. Split image into shelves)
 * 4. Calculate dominant angle
 * 5. Segment spines
 * 6. Crop spines
 * 7. Isolate text features
 * 8. Send to OCR
 * 9. Interface.
 */


static Mat scaleToLimit(Mat image, double rowLimit) {
    Mat resImage;
    // Resize if it's too big. Limit height to 800px.
    if (image.rows > rowLimit) {
        resize(image, resImage, Size(0, 0), rowLimit / image.rows, rowLimit / image.rows);
    } else {
        resImage = image;
    }
    return resImage;
}

int main(int argc, char **argv) {
    // Read the file into a matrix.
    Mat image = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    imshow("original", image);
    image = scaleToLimit(image, 800);
    Mat grayImage(image.size(), CV_8U);
    cvtColor(image, grayImage, CV_BGR2GRAY);
    auto cannyOutput = canny(image);
    Mat matchedLines = std::get<0>(cannyOutput);
    int dominantAngle = std::get<1>(cannyOutput);
    Mat linesRotated = rotateMatrix(matchedLines, dominantAngle);
    imshow("lines", linesRotated);
    Mat grayRotated = rotateMatrix(grayImage, dominantAngle);
    imshow("orotated", grayRotated);
    auto crops = segmentSpines(linesRotated);
    for (int i = 0; i < crops.size(); i++) {
        Mat crop = grayRotated.rowRange(crops[i]);
        imshow("ocropped" + std::to_string(i), crop);
        //imwrite("cropped" + std::to_string(i) + ".png", crop);
        auto textMat = isolateText(crop);
        //imshow("cropped" + std::to_string(i), textMat);
        imwrite("cropped" + std::to_string(i) + ".png", textMat);
    }
    waitKey(0);
    return 0;
}
