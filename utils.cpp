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

    void plotHistogramImage(const vector<int> &histogram, const string &filename, const bool quiet = false)
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
        if (!quiet)
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
            cout << "Warning: input image is not grayscale. Converting to grayscale first." << endl;
            cvtColor(image, grayImage, COLOR_BGR2GRAY);
        }
    }
}

void outputHistogram(const vector<int> &histogram, const string &filename, const string &title, const bool quiet = false)
{
    if (!quiet)
        printHistogram(histogram, title);
    plotHistogramImage(histogram, filename, quiet);
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

void stackImages(const cv::Mat &img1,
                 const cv::Mat &img2,
                 cv::Mat &output,
                 bool horizontal,
                 double dividerPercent,
                 const cv::Scalar &dividerColor)
{
    if (img1.empty() || img2.empty())
    {
        cerr << "One of the images is empty. Cannot stack." << endl;
        return;
    }

    // Clone to avoid modifying originals
    Mat img1_copy = img1.clone();
    Mat img2_resized;

    // Base thickness
    int dividerSize = 0;
    int gapSize = 0;
    Mat whiteGap, divider, secondWhiteGap;

    if (horizontal)
    {
        // Scale img2 to match img1 height
        double scale = static_cast<double>(img1.rows) / img2.rows;
        int newWidth = cvRound(img2.cols * scale);
        resize(img2, img2_resized, Size(newWidth, img1.rows));

        // Calculate sizes
        dividerSize = std::max(1, static_cast<int>(dividerPercent * img1.rows));
        gapSize = 4 * dividerSize;

        // Build the parts
        whiteGap = Mat(img1.rows, gapSize, img1.type(), Scalar(255, 255, 255));
        divider = Mat(img1.rows, dividerSize, img1.type(), dividerColor);
        secondWhiteGap = Mat(img1.rows, gapSize, img1.type(), Scalar(255, 255, 255));

        // Concatenate: img1 | whiteGap | divider | whiteGap | img2
        hconcat(vector<Mat>{img1_copy, whiteGap, divider, secondWhiteGap, img2_resized}, output);
    }
    else
    {
        // Scale img2 to match img1 width
        double scale = static_cast<double>(img1.cols) / img2.cols;
        int newHeight = cvRound(img2.rows * scale);
        resize(img2, img2_resized, Size(img1.cols, newHeight));

        // Calculate sizes
        dividerSize = std::max(1, static_cast<int>(dividerPercent * img1.cols));
        gapSize = 4 * dividerSize;

        // Build the parts
        whiteGap = Mat(gapSize, img1.cols, img1.type(), Scalar(255, 255, 255));
        divider = Mat(dividerSize, img1.cols, img1.type(), dividerColor);
        secondWhiteGap = Mat(gapSize, img1.cols, img1.type(), Scalar(255, 255, 255));

        // Concatenate: img1 / whiteGap / divider / whiteGap / img2
        vconcat(vector<Mat>{img1_copy, whiteGap, divider, secondWhiteGap, img2_resized}, output);
    }
}

void resizeIfTooLarge(cv::Mat &image, int maxDim = 1024)
{
    if (image.empty())
        return;

    int height = image.rows;
    int width = image.cols;

    if (height >= width && height > maxDim)
    {
        double scale = static_cast<double>(maxDim) / height;
        int newWidth = cvRound(width * scale);
        resize(image, image, Size(newWidth, maxDim));
    }
    else if (width > height && width > maxDim)
    {
        double scale = static_cast<double>(maxDim) / width;
        int newHeight = cvRound(height * scale);
        resize(image, image, Size(maxDim, newHeight));
    }
}

void generateCombinedOutputs(
    const cv::Mat &beforeImage,
    const cv::Mat &afterImage,
    const std::string &beforeHistPath,
    const std::string &afterHistPath,
    const std::string &beforeCombinedPath,
    const std::string &afterCombinedPath,
    const std::string &resultCombinedPath)
{
    Mat histBeforeImg = imread(beforeHistPath, IMREAD_GRAYSCALE);
    Mat histAfterImg = imread(afterHistPath, IMREAD_GRAYSCALE);

    if (histBeforeImg.empty())
    {
        cerr << "Failed to load histogram image: " << beforeHistPath << endl;
        return;
    }
    if (histAfterImg.empty())
    {
        cerr << "Failed to load histogram image: " << afterHistPath << endl;
        return;
    }

    // First combined: before image + histogram
    Mat combinedBefore;
    stackImages(beforeImage, histBeforeImg, combinedBefore, true);
    resizeIfTooLarge(combinedBefore);
    imwrite(beforeCombinedPath, combinedBefore);

    // Second combined: after image + histogram
    Mat combinedAfter;
    stackImages(afterImage, histAfterImg, combinedAfter, true);
    resizeIfTooLarge(combinedAfter);
    imwrite(afterCombinedPath, combinedAfter);

    // Final combined: stack the two above vertically
    Mat resultCombined;
    stackImages(combinedBefore, combinedAfter, resultCombined, false);
    resizeIfTooLarge(resultCombined);
    imwrite(resultCombinedPath, resultCombined);
}
