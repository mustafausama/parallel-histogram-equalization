#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <mpi.h>

using namespace cv;
using namespace std;

void computeLocalHistogram(const Mat& input, vector<int>& localHist, int startRow, int endRow) {
    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < input.cols; j++) {
            int pixelValue = input.at<uchar>(i, j);
            localHist[pixelValue]++;
        }
    }
}

void applyEqualization(Mat& partImage, const vector<uchar>& equalizedLUT) {
    for (int i = 0; i < partImage.rows; i++) {
        for (int j = 0; j < partImage.cols; j++) {
            partImage.at<uchar>(i, j) = equalizedLUT[partImage.at<uchar>(i, j)];
        }
    }
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            cout << "Usage: mpirun -np <num_processes> ./mpi <image_path>" << endl;
        }
        MPI_Finalize();
        return -1;
    }

    Mat image;
    int rows, cols;
    if (rank == 0) {
        image = imread(argv[1], IMREAD_UNCHANGED);
        if (image.empty()) {
            cout << "Could not open or find the image." << endl;
            MPI_Abort(MPI_COMM_WORLD, -1);
        }

        if (image.channels() != 1) {
            cout << "Input image is not grayscale. Converting to grayscale first." << endl;
            cvtColor(image, image, COLOR_BGR2GRAY);
        }

        rows = image.rows;
        cols = image.cols;
    }

    // Broadcast image size
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Scatter the rows of the image
    int localRows = rows / size;
    int remainder = rows % size;
    int myRows = (rank < remainder) ? localRows + 1 : localRows;
    Mat localImage(myRows, cols, CV_8UC1);

    // Scatterv setup
    vector<int> sendCounts(size), displs(size);
    int offset = 0;
    for (int i = 0; i < size; i++) {
        sendCounts[i] = (i < remainder) ? (localRows + 1) * cols : localRows * cols;
        displs[i] = offset;
        offset += sendCounts[i];
    }

    MPI_Scatterv(image.data, sendCounts.data(), displs.data(), MPI_UNSIGNED_CHAR,
                 localImage.data, myRows * cols, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Each process computes its local histogram
    vector<int> localHist(256, 0);
    computeLocalHistogram(localImage, localHist, 0, myRows);

    // Reduce histograms to get the global histogram at rank 0
    vector<int> globalHist(256, 0);
    MPI_Reduce(localHist.data(), globalHist.data(), 256, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Rank 0 computes CDF and equalized LUT
    vector<uchar> equalizedLUT(256, 0);
    if (rank == 0) {
        int totalPixels = rows * cols;
        vector<float> pdf(256, 0.0), cdf(256, 0.0);
        for (int i = 0; i < 256; i++) {
            pdf[i] = (float)globalHist[i] / totalPixels;
        }
        cdf[0] = pdf[0];
        for (int i = 1; i < 256; i++) {
            cdf[i] = cdf[i - 1] + pdf[i];
        }
        for (int i = 0; i < 256; i++) {
            equalizedLUT[i] = cvRound(cdf[i] * 255);
        }
    }

    // Broadcast the LUT to all processes
    MPI_Bcast(equalizedLUT.data(), 256, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Apply equalization to the local part of the image
    applyEqualization(localImage, equalizedLUT);

    // Gather the processed parts back to rank 0
    Mat equalizedImage;
    if (rank == 0) {
        equalizedImage = Mat(rows, cols, CV_8UC1);
    }

    MPI_Gatherv(localImage.data, myRows * cols, MPI_UNSIGNED_CHAR,
                equalizedImage.data, sendCounts.data(), displs.data(), MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    // Save the result at rank 0
    if (rank == 0) {
        imwrite("gray_image_mpi.png", image);
        imwrite("equalized_output_mpi.png", equalizedImage);
        cout << "Saved equalized_output_mpi.png successfully." << endl;
    }

    MPI_Finalize();
    return 0;
}
