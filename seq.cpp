#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

void manualHistogramEqualization(const Mat& input, Mat& output) {
    int histSize = 256;
    vector<int> histogram(histSize, 0);

    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            int pixelValue = input.at<uchar>(i, j);
            histogram[pixelValue]++;
        }
    }

    vector<float> pdf(histSize, 0.0);
    int totalPixels = input.rows * input.cols;
    for (int i = 0; i < histSize; i++) {
        pdf[i] = (float)histogram[i] / totalPixels;
    }

    vector<float> cdf(histSize, 0.0);
    cdf[0] = pdf[0];
    for (int i = 1; i < histSize; i++) {
        cdf[i] = cdf[i - 1] + pdf[i];
    }

    vector<uchar> equalizedLUT(histSize, 0);
    for (int i = 0; i < histSize; i++) {
        equalizedLUT[i] = cvRound(cdf[i] * 255);
    }

    output = input.clone();
    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            output.at<uchar>(i, j) = equalizedLUT[input.at<uchar>(i, j)];
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: ./hist_equalization <image_path>" << endl;
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
    manualHistogramEqualization(grayImage, equalizedImage);

    // Save both images to disk
    imwrite("gray_image.png", grayImage);
    imwrite("equalized_output.png", equalizedImage);

    cout << "Saved gray_image.png and equalized_output.png successfully." << endl;

    return 0;
}
