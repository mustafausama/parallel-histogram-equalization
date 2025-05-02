#include <mpi.h>
#include "utils.hpp"

using namespace cv;
using namespace std;

#define BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH "output/mpi/before/histogram_before_mpi.png"
#define AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH "output/mpi/after/histogram_after_mpi.png"
#define BEFORE_IMAGE_OUTPUT_PATH "output/mpi/before/image_before_mpi.png"
#define AFTER_IMAGE_OUTPUT_PATH "output/mpi/after/image_after_mpi.png"

void computeLocalHistogram(const Mat &input, vector<int> &localHist, int startRow, int endRow)
{
    for (int i = startRow; i < endRow; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            int pixelValue = input.at<uchar>(i, j);
            localHist[pixelValue]++;
        }
    }
}

void applyEqualization(Mat &partImage, const vector<uchar> &equalizedLUT)
{
    for (int i = 0; i < partImage.rows; i++)
    {
        for (int j = 0; j < partImage.cols; j++)
        {
            partImage.at<uchar>(i, j) = equalizedLUT[partImage.at<uchar>(i, j)];
        }
    }
}

void manualHistogramEqualization(const int rank, const int size, const Mat &image, vector<int> &histBefore, Mat &equalizedImage, vector<int> &histAfter)
{
    int rows = image.rows;
    int cols = image.cols;

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
    for (int i = 0; i < size; i++)
    {
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
    MPI_Reduce(localHist.data(), histBefore.data(), 256, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Rank 0 computes CDF and equalized LUT
    vector<uchar> equalizedLUT(256, 0);
    if (rank == 0)
    {
        int totalPixels = rows * cols;
        vector<float> pdf(256, 0.0), cdf(256, 0.0);
        for (int i = 0; i < 256; i++)
        {
            pdf[i] = (float)histBefore[i] / totalPixels;
        }
        cdf[0] = pdf[0];
        for (int i = 1; i < 256; i++)
        {
            cdf[i] = cdf[i - 1] + pdf[i];
        }
        for (int i = 0; i < 256; i++)
        {
            equalizedLUT[i] = cvRound(cdf[i] * 255);
        }
    }

    // Broadcast the LUT to all processes
    MPI_Bcast(equalizedLUT.data(), 256, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Apply equalization to the local part of the image
    applyEqualization(localImage, equalizedLUT);

    // Gather the processed parts back to rank 0
    if (rank == 0)
    {
        equalizedImage = Mat(rows, cols, CV_8UC1);
    }

    MPI_Gatherv(localImage.data, myRows * cols, MPI_UNSIGNED_CHAR,
                equalizedImage.data, sendCounts.data(), displs.data(), MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    // Save the result and histogram after equalization at rank 0
    if (rank == 0)
    {
        // Calculate histogram AFTER equalization
        for (int i = 0; i < equalizedImage.rows; i++)
        {
            for (int j = 0; j < equalizedImage.cols; j++)
            {
                int pixelValue = equalizedImage.at<uchar>(i, j);
                histAfter[pixelValue]++;
            }
        }
    }
}

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2 && argc != 3)
    {
        if (rank == 0)
        {
            cout << "Usage: mpirun -np <num_processes> ./mpi [--quiet|-q] <image_path>" << endl;
        }
        MPI_Finalize();
        return -1;
    }
    string filename = argv[1];
    bool quiet = false;
    if (argc == 3)
    {
        if (rank == 0)
        {
            if (string(argv[1]) == "--quiet" || string(argv[1]) == "-q")
            {
                quiet = true;
                filename = argv[2];
            }
            else
            {
                cout << "Usage: mpirun -np <num_processes> ./mpi [--quiet|-q] <image_path>" << endl;
                MPI_Finalize();
                return -1;
            }
        }
    }

    Mat image;
    int rows = 0, cols = 0;
    if (rank == 0)
    {
        try
        {
            readImage(filename, image);
        }
        catch (const std::exception &e)
        {
            MPI_Abort(MPI_COMM_WORLD, -1);
            throw e;
        }
    }

    Mat equalizedImage;
    vector<int> histBefore(256, 0);
    vector<int> histAfter(256, 0);

    double duration = measureRuntime(manualHistogramEqualization, rank, size, image, histBefore, equalizedImage, histAfter);

    if (rank == 0)
    {
        imwrite(BEFORE_IMAGE_OUTPUT_PATH, image);
        imwrite(AFTER_IMAGE_OUTPUT_PATH, equalizedImage);

        if (!quiet)
        {
            outputHistogram(histBefore, BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH, "Histogram BEFORE Equalization");
            outputHistogram(histAfter, AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH, "Histogram AFTER Equalization");
            cout << "\nSaved " << BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH << " and " << AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH << " successfully." << endl;
        }

        cout << "Runtime: " << duration << " ms" << endl;
    }

    MPI_Finalize();
    return 0;
}
