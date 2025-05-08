#include <opencv2/opencv.hpp>
#include <chrono>
#include <utility>
#include <fstream>
#include <cmath>

using namespace std;
using namespace cv;

#ifndef UTILS_HPP
#define UTILS_HPP

#define HIST_WIDTH 50
#define HIST_IMG_W 512
#define HIST_IMG_H 400

#define ImageType vector<vector<uint8_t>>

void outputHistogram(const vector<int> &histogram, const string &filename, const string &title, const bool quiet);

void readImage(const string &filename, ImageType &image);

void writeImage(const string &filename, const ImageType &image);

template <typename Func, typename... Args>
double measureRuntime(const string &outputPath, Func &&func, Args &&...args)
{
  auto start = chrono::high_resolution_clock::now();
  forward<Func>(func)(forward<Args>(args)...);
  auto end = chrono::high_resolution_clock::now();

  chrono::duration<double, milli> duration = end - start;

  double d_duration = duration.count();

  ofstream runtimeFile(outputPath, ios::trunc);
  if (runtimeFile.is_open())
  {
    runtimeFile << d_duration << " ms" << endl;
    runtimeFile.close();
  }
  else
  {
    cerr << "Unable to open file: " << outputPath << endl;
  }

  return d_duration;
}

void stackImages(const Mat &img1,
                 const Mat &img2,
                 Mat &output,
                 bool horizontal = true,
                 double dividerPercent = 0.005,
                 const Scalar &dividerColor = Scalar(0));

void generateCombinedOutputs(
    const ImageType &beforeImage,
    const ImageType &afterImage,
    const string &beforeHistPath,
    const string &afterHistPath,
    const string &beforeCombinedPath,
    const string &afterCombinedPath,
    const string &resultCombinedPath);

#endif