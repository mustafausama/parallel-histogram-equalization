#include "utils.hpp"

using namespace cv;
using namespace std;

#define BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH "output/seq/before/histogram_before_seq.png"
#define AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH "output/seq/after/histogram_after_seq.png"
#define BEFORE_IMAGE_OUTPUT_PATH "output/seq/before/image_before_seq.png"
#define AFTER_IMAGE_OUTPUT_PATH "output/seq/after/image_after_seq.png"

void manualHistogramEqualization(const Mat &input, Mat &output, vector<int> &histBefore, vector<int> &histAfter)
{
    int histSize = 256;
    histBefore.assign(histSize, 0);

    // Calculate histogram
    for (int i = 0; i < input.rows; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            int pixelValue = input.at<uchar>(i, j);
            histBefore[pixelValue]++;
        }
    }

    // PDF
    vector<float> pdf(histSize, 0.0);
    int totalPixels = input.rows * input.cols;
    for (int i = 0; i < histSize; i++)
    {
        pdf[i] = (float)histBefore[i] / totalPixels;
    }

    // CDF
    vector<float> cdf(histSize, 0.0);
    cdf[0] = pdf[0];
    for (int i = 1; i < histSize; i++)
    {
        cdf[i] = cdf[i - 1] + pdf[i];
    }

    // Equalization LUT
    vector<uchar> equalizedLUT(histSize, 0);
    for (int i = 0; i < histSize; i++)
    {
        equalizedLUT[i] = cvRound(cdf[i] * 255);
    }

    // Apply equalization
    output = input.clone();
    for (int i = 0; i < input.rows; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            output.at<uchar>(i, j) = equalizedLUT[input.at<uchar>(i, j)];
        }
    }

    // Calculate histogram after equalization
    histAfter.assign(histSize, 0);
    for (int i = 0; i < output.rows; i++)
    {
        for (int j = 0; j < output.cols; j++)
        {
            int pixelValue = output.at<uchar>(i, j);
            histAfter[pixelValue]++;
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

    if (!quiet)
    {
        outputHistogram(histBefore, BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH, "Histogram BEFORE Equalization");
        outputHistogram(histAfter, AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH, "Histogram AFTER Equalization");

        cout << "\nSaved " << BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH << " and " << AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH << " successfully." << endl;
    }

    cout << "Runtime: " << duration << " ms" << endl;

    return 0;
}
