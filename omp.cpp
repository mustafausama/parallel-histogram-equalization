#include <omp.h>
#include "utils.hpp"
#include <fstream>

using namespace cv;
using namespace std;

#define BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH "output/omp/before/histogram_before_omp.png"
#define AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH "output/omp/after/histogram_after_omp.png"
#define BEFORE_IMAGE_OUTPUT_PATH "output/omp/before/image_before_omp.png"
#define AFTER_IMAGE_OUTPUT_PATH "output/omp/after/image_after_omp.png"
#define BEFORE_IMAGE_HISTOGRAM_COMBINED_PATH "output/omp/before/image_histo_before_omp.png"
#define AFTER_IMAGE_HISTOGRAM_COMBINED_PATH "output/omp/after/image_histo_after_omp.png"
#define BEFORE_AFTER_COMBINED_PATH "output/omp/result_omp.png"
#define RUNTIME_OUTPUT_PATH "output/omp/runtime_omp.txt"

void manualHistogramEqualization(const Mat &input, Mat &output, vector<int> &histBefore, vector<int> &histAfter)
{
    int histSize = 256;
    histBefore.assign(histSize, 0);
    histAfter.assign(histSize, 0);

// Parallel histogram calculation using per-thread local histograms + reduction
#pragma omp parallel
    {
        vector<int> localHist(histSize, 0);

#pragma omp for collapse(2)
        for (int i = 0; i < input.rows; i++)
        {
            for (int j = 0; j < input.cols; j++)
            {
                int pixelValue = input.at<uchar>(i, j);
                localHist[pixelValue]++;
            }
        }

// Merge local histograms into the global histogram
#pragma omp critical
        {
            for (int i = 0; i < histSize; i++)
            {
                histBefore[i] += localHist[i];
            }
        }
    }

    // Compute PDF
    vector<float> pdf(histSize, 0.0);
    int totalPixels = input.rows * input.cols;
#pragma omp parallel for
    for (int i = 0; i < histSize; i++)
    {
        pdf[i] = (float)histBefore[i] / totalPixels;
    }

    // Compute CDF (serial because it's tiny work)
    vector<float> cdf(histSize, 0.0);
    cdf[0] = pdf[0];
    for (int i = 1; i < histSize; i++)
    {
        cdf[i] = cdf[i - 1] + pdf[i];
    }

    // Prepare LUT
    vector<uchar> equalizedLUT(histSize, 0);
#pragma omp parallel for
    for (int i = 0; i < histSize; i++)
    {
        equalizedLUT[i] = cvRound(cdf[i] * 255);
    }

    // Apply LUT to get the equalized image
    output = input.clone();
#pragma omp parallel for collapse(2)
    for (int i = 0; i < input.rows; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            output.at<uchar>(i, j) = equalizedLUT[input.at<uchar>(i, j)];
        }
    }

// Histogram after equalization using per-thread local histograms + reduction
#pragma omp parallel
    {
        vector<int> localHist(histSize, 0);

#pragma omp for collapse(2)
        for (int i = 0; i < output.rows; i++)
        {
            for (int j = 0; j < output.cols; j++)
            {
                int pixelValue = output.at<uchar>(i, j);
                localHist[pixelValue]++;
            }
        }

// Merge local histograms into the global histogram
#pragma omp critical
        {
            for (int i = 0; i < histSize; i++)
            {
                histAfter[i] += localHist[i];
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        cout << "Usage: " << argv[0] << " [--quiet|-q] <image_path>" << endl;
        return -1;
    }
    string filename = argv[1];
    bool quiet = false;
    if (argc == 3)
    {
        if (string(argv[1]) == "--quiet" || string(argv[1]) == "-q")
        {
            quiet = true;
            filename = argv[2];
        }
        else
        {
            cout << "Usage: " << argv[0] << " [--quiet|-q] <image_path>" << endl;
            return -1;
        }
    }

    Mat image;
    readImage(filename, image);

    Mat equalizedImage;
    vector<int> histBefore, histAfter;

    double duration = measureRuntime(manualHistogramEqualization, image, equalizedImage, histBefore, histAfter);

    imwrite(BEFORE_IMAGE_OUTPUT_PATH, image);
    imwrite(AFTER_IMAGE_OUTPUT_PATH, equalizedImage);

    outputHistogram(histBefore, BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH, "Histogram BEFORE Equalization", quiet);
    outputHistogram(histAfter, AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH, "Histogram AFTER Equalization", quiet);

    generateCombinedOutputs(
        image,
        equalizedImage,
        BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH,
        AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH,
        BEFORE_IMAGE_HISTOGRAM_COMBINED_PATH,
        AFTER_IMAGE_HISTOGRAM_COMBINED_PATH,
        BEFORE_AFTER_COMBINED_PATH);

    if (!quiet)
        cout << "\nSaved " << BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH << " and " << AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH << " successfully." << endl;

    cout << "Runtime: " << duration << " ms" << endl;

    // Save runtime to file
    ofstream runtimeFile(RUNTIME_OUTPUT_PATH, ios::trunc);
    if (runtimeFile.is_open())
    {
        runtimeFile << duration << " ms" << endl;
        runtimeFile.close();
    }
    else
    {
        cerr << "Error opening file for writing: " << RUNTIME_OUTPUT_PATH << endl;
    }

    return 0;
}
