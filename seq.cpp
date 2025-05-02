#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

// Function to print histogram as ASCII chart
void printHistogramASCII(const vector<int>& histogram, const string& title) {
    cout << "\n=== " << title << " ===" << endl;
    int maxCount = *max_element(histogram.begin(), histogram.end());
    int scale = maxCount / 50; // scale to 50 chars width max

    for (int i = 0; i < histogram.size(); i++) {
        if (histogram[i] > 0) {
            cout << "[" << setw(3) << i << "] ";
            int barLength = histogram[i] / (scale == 0 ? 1 : scale);
            for (int j = 0; j < barLength; j++) {
                cout << "#";
            }
            cout << " (" << histogram[i] << ")" << endl;
        }
    }
}

// Function to plot histogram and save as image
void plotHistogramImage(const vector<int>& histogram, const string& filename) {
    int histSize = histogram.size();
    int hist_w = 512; int hist_h = 400;
    int bin_w = cvRound((double) hist_w / histSize);

    Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(255));

    // Normalize histogram to fit image height
    int maxVal = *max_element(histogram.begin(), histogram.end());
    vector<int> normHist(histSize);
    for (int i = 0; i < histSize; i++) {
        normHist[i] = ((double)histogram[i] / maxVal) * histImage.rows;
    }

    for (int i = 0; i < histSize; i++) {
        rectangle(histImage, Point(i * bin_w, hist_h),
                  Point((i + 1) * bin_w, hist_h - normHist[i]),
                  Scalar(0), FILLED);
    }

    imwrite(filename, histImage);
    cout << "Saved histogram image: " << filename << endl;
}

void manualHistogramEqualization(const Mat& input, Mat& output, vector<int>& histBefore, vector<int>& histAfter) {
    int histSize = 256;
    histBefore.assign(histSize, 0);

    // Calculate histogram
    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            int pixelValue = input.at<uchar>(i, j);
            histBefore[pixelValue]++;
        }
    }

    // PDF
    vector<float> pdf(histSize, 0.0);
    int totalPixels = input.rows * input.cols;
    for (int i = 0; i < histSize; i++) {
        pdf[i] = (float)histBefore[i] / totalPixels;
    }

    // CDF
    vector<float> cdf(histSize, 0.0);
    cdf[0] = pdf[0];
    for (int i = 1; i < histSize; i++) {
        cdf[i] = cdf[i - 1] + pdf[i];
    }

    // Equalization LUT
    vector<uchar> equalizedLUT(histSize, 0);
    for (int i = 0; i < histSize; i++) {
        equalizedLUT[i] = cvRound(cdf[i] * 255);
    }

    // Apply equalization
    output = input.clone();
    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            output.at<uchar>(i, j) = equalizedLUT[input.at<uchar>(i, j)];
        }
    }

    // Calculate histogram after equalization
    histAfter.assign(histSize, 0);
    for (int i = 0; i < output.rows; i++) {
        for (int j = 0; j < output.cols; j++) {
            int pixelValue = output.at<uchar>(i, j);
            histAfter[pixelValue]++;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: ./seq <image_path>" << endl;
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
    vector<int> histBefore, histAfter;
    manualHistogramEqualization(grayImage, equalizedImage, histBefore, histAfter);

    // Print ASCII charts
    printHistogramASCII(histBefore, "Histogram BEFORE Equalization");
    printHistogramASCII(histAfter, "Histogram AFTER Equalization");

    // Save histogram images
    plotHistogramImage(histBefore, "histogram_before.png");
    plotHistogramImage(histAfter, "histogram_after.png");

    // Save images
    imwrite("gray_image.png", grayImage);
    imwrite("equalized_output.png", equalizedImage);

    cout << "\nSaved gray_image.png and equalized_output.png successfully." << endl;

    return 0;
}
