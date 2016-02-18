//
// Spine segmentation routines.
//

#include <iostream>
#include <opencv2/highgui.hpp>
#include "segment.h"
#include "edges.h"

using namespace cv;
using std::vector;

inline static std::pair<int, int> yExtremes(const vector<Point>& component) {
    int min = component[0].x;
    int max = component[0].x;
    for (auto& point: component) {
        if (point.x < min)
            min = point.x;
        if (point.x > max)
            max = point.x;
    }
    return std::make_pair(min, max);
}

static bool compareLines(const vector<Point>& a, const vector<Point>& b) {
    auto aKey = yExtremes(a);
    auto bKey = yExtremes(b);
    return std::get<0>(aKey) < std::get<0>(bKey);
}

inline static int xSpan(const vector<Point>& component) {
    int min = component[0].y;
    int max = component[0].y;
    for (auto& point : component) {
        if (point.y < min)
            min = point.y;
        if (point.y > max)
            max = point.y;
    }
    return max - min;
}

vector<Range> segmentSpines(Mat& grayImage) {
    vector<Range> spines;
    vector<int> candidates;
    vector<int> filtered_candidates;
    // New process: trace each edge. if we find some segment that spans > 0.3x of the width
    // with < 10px height variation, then take that average height value as a spine candidate.
    const int heightThreshold = 10;
    const int widthThreshold = (int) (0.3 * grayImage.cols);
    const int gapThreshold = heightThreshold * 2;
    for (auto& edge: connectedComponents(grayImage)) {
        // TODO: do tracing
        int i = 0;
        while (i < edge.size()) {
            auto start = edge[i];
            int j;
            for (j = i; j < edge.size(); j++) {
                if (std::abs(edge[j].x - start.x) > heightThreshold)
                    break;
            }
            if (std::abs(start.y - edge[j - 1].y) > widthThreshold) {
                candidates.push_back((edge[j - 1].x + start.x) / 2);
            }
            i = j;
        }
    }
    // TODO: filter too-close candidates out.
    std::sort(candidates.begin(), candidates.end());
    for (int i = 0; i < candidates.size(); i++) {
        if (filtered_candidates.size() == 0 ||
            candidates[i] - filtered_candidates[filtered_candidates.size() - 1] > gapThreshold)
            filtered_candidates.push_back(candidates[i]);
    }
    for (int i = 0; i < filtered_candidates.size() - 1; i++) {
        spines.push_back(Range(filtered_candidates[i], filtered_candidates[i+1]));
    }
    return spines;
}