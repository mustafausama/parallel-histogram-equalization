#include <opencv2/opencv.hpp>
#include <iostream>
#include "utils.hpp"
#include <fstream>
using namespace std;
using namespace cv;

// Define paths
#define SEQ_RESULT_PATH "output/seq/result_seq.png"
#define OMP_RESULT_PATH "output/omp/result_omp.png"
#define MPI_RESULT_PATH "output/mpi/result_mpi.png"
#define FINAL_OUTPUT_PATH "output/result_all.png"

// Runtime files
#define SEQ_RUNTIME_PATH "output/seq/runtime_seq.txt"
#define OMP_RUNTIME_PATH "output/omp/runtime_omp.txt"
#define MPI_RUNTIME_PATH "output/mpi/runtime_mpi.txt"

// Divider settings
#define DIVIDER_PERCENT 0.005
#define DIVIDER_COLOR Scalar(0, 0, 255) // Red divider

// Font settings
#define FONT_FACE FONT_HERSHEY_SIMPLEX
#define FONT_SCALE 1.0
#define FONT_THICKNESS 2
#define TEXT_COLOR Scalar(0, 0, 0) // Black

// Utility to read runtime text
string readRuntime(const string &path)
{
  ifstream file(path);
  string line;
  if (file.is_open())
  {
    getline(file, line);
    file.close();
    return line;
  }
  return "Runtime: N/A";
}

// Utility to create a text image (header or runtime)
Mat createTextImage(const string &text, int width, int height)
{
  Mat textImg(height, width, CV_8UC3, Scalar(255, 255, 255)); // White background
  int baseline = 0;
  Size textSize = getTextSize(text, FONT_FACE, FONT_SCALE, FONT_THICKNESS, &baseline);

  Point textOrg((width - textSize.width) / 2, (height + textSize.height) / 2 - baseline);
  putText(textImg, text, textOrg, FONT_FACE, FONT_SCALE, TEXT_COLOR, FONT_THICKNESS);
  return textImg;
}

int main()
{
  // Read the 3 result images
  Mat seqResult = imread(SEQ_RESULT_PATH, IMREAD_COLOR);
  Mat ompResult = imread(OMP_RESULT_PATH, IMREAD_COLOR);
  Mat mpiResult = imread(MPI_RESULT_PATH, IMREAD_COLOR);

  if (seqResult.empty() || ompResult.empty() || mpiResult.empty())
  {
    cerr << "Error: Failed to load one or more result images." << endl;
    return -1;
  }

  // Split each result into before and after halves
  auto splitHalf = [](const Mat &img) -> pair<Mat, Mat>
  {
    int midY = img.rows / 2;
    Mat before = img(Rect(0, 0, img.cols, midY)).clone();
    Mat after = img(Rect(0, midY, img.cols, img.rows - midY)).clone();
    return {before, after};
  };

  auto [seqBefore, seqAfter] = splitHalf(seqResult);
  auto [ompBefore, ompAfter] = splitHalf(ompResult);
  auto [mpiBefore, mpiAfter] = splitHalf(mpiResult);

  // Determine cell size (uniform)
  int cellWidth = seqBefore.cols;
  int cellHeight = seqBefore.rows;

  // Create headers
  Mat headerSeq = createTextImage("Sequential", cellWidth, 60);
  Mat headerOmp = createTextImage("OpenMP", cellWidth, 60);
  Mat headerMpi = createTextImage("MPI", cellWidth, 60);

  // Create runtime rows
  string seqRuntime = readRuntime(SEQ_RUNTIME_PATH);
  string ompRuntime = readRuntime(OMP_RUNTIME_PATH);
  string mpiRuntime = readRuntime(MPI_RUNTIME_PATH);

  Mat runtimeSeq = createTextImage(seqRuntime, cellWidth, 60);
  Mat runtimeOmp = createTextImage(ompRuntime, cellWidth, 60);
  Mat runtimeMpi = createTextImage(mpiRuntime, cellWidth, 60);

  // Combine each row horizontally
  Mat headerRow;
  hconcat(vector<Mat>{headerSeq, headerOmp, headerMpi}, headerRow);

  Mat beforeRow;
  hconcat(vector<Mat>{seqBefore, ompBefore, mpiBefore}, beforeRow);

  Mat afterRow;
  hconcat(vector<Mat>{seqAfter, ompAfter, mpiAfter}, afterRow);

  Mat runtimeRow;
  hconcat(vector<Mat>{runtimeSeq, runtimeOmp, runtimeMpi}, runtimeRow);

  // Stack all rows vertically
  Mat grid;
  vconcat(vector<Mat>{headerRow, beforeRow, afterRow, runtimeRow}, grid);

  // Save final combined
  imwrite(FINAL_OUTPUT_PATH, grid);
  cout << "Saved final combined grid image to: " << FINAL_OUTPUT_PATH << endl;

  return 0;
}
