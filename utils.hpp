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

class ImageType
{
private:
  uint8_t *data;
  size_t _rows;
  size_t _cols;

public:
  // Constructor
  ImageType(size_t rows = 0, size_t cols = 0) : _rows(rows), _cols(cols)
  {
    data = rows > 0 && cols > 0 ? new uint8_t[rows * cols] : nullptr;
  }

  // Destructor
  ~ImageType()
  {
    delete[] data;
  }

  // Assignment operator (for deep copy)
  ImageType &operator=(const ImageType &other)
  {
    if (this != &other)
    {
      delete[] data;
      _rows = other._rows;
      _cols = other._cols;
      data = new uint8_t[_rows * _cols];
      memcpy(data, other.data, _rows * _cols);
    }
    return *this;
  }

  // Size and data access
  size_t rows() const { return _rows; }
  size_t cols() const { return _cols; }
  uint8_t *getData() { return data; }
  const uint8_t *getData() const { return data; }

  // Element access
  uint8_t &at(size_t row, size_t col)
  {
    return data[row * _cols + col];
  }

  const uint8_t &at(size_t row, size_t col) const
  {
    return data[row * _cols + col];
  }

  // Resize
  void resize(size_t rows, size_t cols)
  {
    delete[] data;
    _rows = rows;
    _cols = cols;
    data = new uint8_t[rows * cols];
  }
};

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