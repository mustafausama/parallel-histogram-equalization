#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>

using namespace cv;
using namespace std;

void manualHistogramEqualizationOMP(const Mat& input, Mat& output) {
    int histSize = 256;
    vector<int> histogram(histSize, 0);

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
                histogram[i] += localHist[i];
            }
        }
    }

    vector<float> pdf(histSize, 0.0);
    int totalPixels = input.rows * input.cols;
    #pragma omp parallel for
    for (int i = 0; i < histSize; i++) {
        pdf[i] = (float)histogram[i] / totalPixels;
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
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: ./omp <image_path>" << endl;
        return -1;
    }

    Mat image = imread(argv[1], IMREAD_UNCHANGED);
    if (image.empty()) {
        cout << "Could not open or find the image." << endl;
        return -1;
    }

    Mat grayImage;
    if (image.channels() == 1) {
        grayImage = image.clone();
    } else {
        cout << "Input image is not grayscale. Converting to grayscale first." << endl;
        cvtColor(image, grayImage, COLOR_BGR2GRAY);
    }

    Mat equalizedImage;
    manualHistogramEqualizationOMP(grayImage, equalizedImage);

    // Save both images to disk
    imwrite("gray_image_omp.png", grayImage);
    imwrite("equalized_output_omp.png", equalizedImage);

    cout << "Saved gray_image_omp.png and equalized_output_omp.png successfully." << endl;

    return 0;
}
