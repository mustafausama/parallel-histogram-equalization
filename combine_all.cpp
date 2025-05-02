#include <opencv2/opencv.hpp>
#include <iostream>
#include "utils.hpp"

using namespace std;
using namespace cv;

// Define paths
#define SEQ_RESULT_PATH "output/seq/result_seq.png"
#define OMP_RESULT_PATH "output/omp/result_omp.png"
#define MPI_RESULT_PATH "output/mpi/result_mpi.png"
#define FINAL_OUTPUT_PATH "output/result_all.png"

// Divider settings
#define DIVIDER_PERCENT 0.005
#define DIVIDER_COLOR Scalar(0, 0, 255) // Red divider

int main()
{
  // Read the 3 result images
  Mat seqResult = imread(SEQ_RESULT_PATH, IMREAD_GRAYSCALE);
  Mat ompResult = imread(OMP_RESULT_PATH, IMREAD_GRAYSCALE);
  Mat mpiResult = imread(MPI_RESULT_PATH, IMREAD_GRAYSCALE);

  if (seqResult.empty())
  {
    cerr << "Failed to load: " << SEQ_RESULT_PATH << endl;
    return -1;
  }
  if (ompResult.empty())
  {
    cerr << "Failed to load: " << OMP_RESULT_PATH << endl;
    return -1;
  }
  if (mpiResult.empty())
  {
    cerr << "Failed to load: " << MPI_RESULT_PATH << endl;
    return -1;
  }

  // Step 1: Combine seq + omp
  Mat firstPairCombined;
  stackImages(seqResult, ompResult, firstPairCombined, true, DIVIDER_PERCENT, DIVIDER_COLOR);

  // Step 2: Combine the result with mpi
  Mat finalCombined;
  stackImages(firstPairCombined, mpiResult, finalCombined, true, DIVIDER_PERCENT, DIVIDER_COLOR);

  // Save final combined
  imwrite(FINAL_OUTPUT_PATH, finalCombined);
  cout << "Saved final combined image to: " << FINAL_OUTPUT_PATH << endl;

  return 0;
}
