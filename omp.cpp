#include <omp.h>
#include "utils.hpp"

using namespace cv;
using namespace std;

#define BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH "output/omp/before/histogram_before_omp.png"
#define AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH "output/omp/after/histogram_after_omp.png"
#define BEFORE_IMAGE_OUTPUT_PATH "output/omp/before/image_before_omp.png"
#define AFTER_IMAGE_OUTPUT_PATH "output/omp/after/image_after_omp.png"

void manualHistogramEqualization(const Mat& input, Mat& output, vector<int>& histBefore, vector<int>& histAfter) {
    int histSize = 256;
    histBefore.assign(histSize, 0);

    // Parallel histogram calculation with reduction
    #pragma omp parallel
    {
        vector<int> localHist(histSize, 0);

        #pragma omp for nowait
        for (int i = 0; i < input.rows; i++) {
            for (int j = 0; j < input.cols; j++) {
                int pixelValue = input.at<uchar>(i, j);
                localHist[pixelValue]++;
            }
        }

        // Combine local histograms
        #pragma omp critical
        {
            for (int i = 0; i < histSize; i++) {
                histBefore[i] += localHist[i];
            }
        }
    }

    vector<float> pdf(histSize, 0.0);
    int totalPixels = input.rows * input.cols;
    #pragma omp parallel for
    for (int i = 0; i < histSize; i++) {
        pdf[i] = (float)histBefore[i] / totalPixels;
    }

    vector<float> cdf(histSize, 0.0);
    cdf[0] = pdf[0];
    for (int i = 1; i < histSize; i++) {
        cdf[i] = cdf[i - 1] + pdf[i];
    }

    vector<uchar> equalizedLUT(histSize, 0);
    #pragma omp parallel for
    for (int i = 0; i < histSize; i++) {
        equalizedLUT[i] = cvRound(cdf[i] * 255);
    }

    output = input.clone();
    #pragma omp parallel for
    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            output.at<uchar>(i, j) = equalizedLUT[input.at<uchar>(i, j)];
        }
    }

    // Calculate histogram after equalization
    histAfter.assign(histSize, 0);
    #pragma omp parallel
    {
        vector<int> localHist(histSize, 0);
        #pragma omp for nowait
        for (int i = 0; i < output.rows; i++) {
            for (int j = 0; j < output.cols; j++) {
                int pixelValue = output.at<uchar>(i, j);
                localHist[pixelValue]++;
            }
        }
        #pragma omp critical
        {
            for (int i = 0; i < histSize; i++) {
                histAfter[i] += localHist[i];
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <image_path>" << endl;
        return -1;
    }

    Mat image;
    readImage(argv[1], image);

    Mat equalizedImage;
    vector<int> histBefore, histAfter;

    double duration = measureRuntime(manualHistogramEqualization, image, equalizedImage, histBefore, histAfter);

    outputHistogram(histBefore, BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH, "Histogram BEFORE Equalization");
    outputHistogram(histAfter, AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH, "Histogram AFTER Equalization");

    imwrite(BEFORE_IMAGE_OUTPUT_PATH, image);
    imwrite(AFTER_IMAGE_OUTPUT_PATH, equalizedImage);

    cout << "\nSaved input_gray_image_omp.png and equalized_output_omp.png successfully." << endl;

    cout << "Runtime: " << duration << " ms" << endl;

    return 0;
}
