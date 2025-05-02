#include <opencv2/opencv.hpp>
#include <chrono>
#include <utility>

using namespace std;
using namespace cv;

#ifndef UTILS_HPP
#define UTILS_HPP

#define HIST_WIDTH 50
#define HIST_IMG_W 512
#define HIST_IMG_H 400

void outputHistogram(const vector<int> &histogram, const string &filename, const string &title, const bool quiet);

void readImage(const string &filename, Mat &image);

template <typename Func, typename... Args>
double measureRuntime(Func &&func, Args &&...args)
{
  auto start = chrono::high_resolution_clock::now();
  forward<Func>(func)(forward<Args>(args)...);
  auto end = chrono::high_resolution_clock::now();

  chrono::duration<double, milli> duration = end - start;
  return duration.count();
}

void stackImages(const cv::Mat &img1,
                 const cv::Mat &img2,
                 cv::Mat &output,
                 bool horizontal = true,
                 double dividerPercent = 0.005,
                 const cv::Scalar &dividerColor = cv::Scalar(0));

void generateCombinedOutputs(
    const cv::Mat &beforeImage,
    const cv::Mat &afterImage,
    const std::string &beforeHistPath,
    const std::string &afterHistPath,
    const std::string &beforeCombinedPath,
    const std::string &afterCombinedPath,
    const std::string &resultCombinedPath);

#endif