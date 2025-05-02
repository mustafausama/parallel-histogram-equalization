#include "utils.hpp"

namespace
{
    void printHistogram(const vector<int> &histogram, const string &title)
    {
        cout << "\n=== " << title << " ===" << endl;
        int maxCount = *max_element(histogram.begin(), histogram.end());
        // scale to the width of the histogram
        int scale = maxCount / HIST_WIDTH;

        for (int i = 0; i < histogram.size(); i++)
        {
            if (histogram[i] > 0)
            {
                cout << "[" << setw(3) << i << "] ";
                int barLength = histogram[i] / (scale == 0 ? 1 : scale);
                for (int j = 0; j < barLength; j++)
                {
                    cout << "#";
                }
                cout << " (" << histogram[i] << ")" << endl;
            }
        }
    }

    void plotHistogramImage(const vector<int> &histogram, const string &filename)
    {
        int histSize = histogram.size();
        int bin_w = cvRound((double)HIST_IMG_W / histSize);

        Mat histImage(HIST_IMG_H, HIST_IMG_W, CV_8UC1, Scalar(255));

        // Normalize histogram to fit image height
        int maxVal = *max_element(histogram.begin(), histogram.end());
        vector<int> normHist(histSize);
        for (int i = 0; i < histSize; i++)
        {
            normHist[i] = ((double)histogram[i] / maxVal) * histImage.rows;
        }

        for (int i = 0; i < histSize; i++)
        {
            rectangle(histImage, Point(i * bin_w, HIST_IMG_H),
                      Point((i + 1) * bin_w, HIST_IMG_H - normHist[i]),
                      Scalar(0), FILLED);
        }

        imwrite(filename, histImage);
        cout << "Saved histogram image: " << filename << endl;
    }

    void grayScaleImage(const Mat &image, Mat &grayImage)
    {
        if (image.channels() == 1)
        {
            grayImage = image.clone();
        }
        else
        {
            cout << "Input image is not grayscale. Converting to grayscale first." << endl;
            cvtColor(image, grayImage, COLOR_BGR2GRAY);
        }
    }
}

void outputHistogram(const vector<int> &histogram, const string &filename, const string &title)
{
    printHistogram(histogram, title);
    plotHistogramImage(histogram, filename);
}

void readImage(const string &filename, Mat &image)
{
    Mat input_image;
    input_image = imread(filename, IMREAD_UNCHANGED);
    if (input_image.empty())
    {
        cerr << "Could not open or find the image: " << filename << endl;
        throw runtime_error("Image not found");
    }
    grayScaleImage(input_image, image);
}
