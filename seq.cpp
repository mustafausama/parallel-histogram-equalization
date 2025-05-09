#include "utils.hpp"
#include <cmath>

using namespace cv;
using namespace std;

#define BEFORE_HISTOGRAM_OUTPUT_IMAGE_PATH "output/seq/before/histogram_before_seq.png"
#define AFTER_HISTOGRAM_OUTPUT_IMAGE_PATH "output/seq/after/histogram_after_seq.png"
#define BEFORE_IMAGE_OUTPUT_PATH "output/seq/before/image_before_seq.png"
#define AFTER_IMAGE_OUTPUT_PATH "output/seq/after/image_after_seq.png"
#define BEFORE_IMAGE_HISTOGRAM_COMBINED_PATH "output/seq/before/image_histo_before_seq.png"
#define AFTER_IMAGE_HISTOGRAM_COMBINED_PATH "output/seq/after/image_histo_after_seq.png"
#define BEFORE_AFTER_COMBINED_PATH "output/seq/result_seq.png"
#define RUNTIME_OUTPUT_PATH "output/seq/runtime_seq.txt"

void histogramEqualization(const ImageType &input, ImageType &output, vector<int> &histBefore, vector<int> &histAfter)
{
    int histSize = 256;
    histBefore.assign(histSize, 0);

    // Calculate histogram
    for (int i = 0; i < input.rows(); i++)
    {
        for (int j = 0; j < input.cols(); j++)
        {
            int pixelValue = input.at(i, j);
            histBefore[pixelValue]++;
        }
    }

    // Probability Density Function
    vector<float> probDensityFunc(histSize, 0.0);
    int totalPixels = input.rows() * input.cols();
    for (int i = 0; i < histSize; i++)
    {
        probDensityFunc[i] = (float)histBefore[i] / totalPixels;
    }

    // Cumulative Distribution Function
    vector<float> cumulativeDistFunc(histSize, 0.0);
    cumulativeDistFunc[0] = probDensityFunc[0];
    for (int i = 1; i < histSize; i++)
    {
        cumulativeDistFunc[i] = cumulativeDistFunc[i - 1] + probDensityFunc[i];
    }

    // Prepare Lookup Table
    vector<uint8_t> eqLookupTable(histSize, 0);
    for (int i = 0; i < histSize; i++)
    {
        eqLookupTable[i] = static_cast<uint8_t>(round(cumulativeDistFunc[i] * 255));
    }

    // Use Lookup Table to equalize the image
    output = input;
    for (int i = 0; i < input.rows(); i++)
    {
        for (int j = 0; j < input.cols(); j++)
        {
            output.at(i, j) = eqLookupTable[input.at(i, j)];
        }
    }

    // Calculate histogram after equalization
    histAfter.assign(histSize, 0);
    for (int i = 0; i < output.rows(); i++)
    {
        for (int j = 0; j < output.cols(); j++)
        {
            int pixelValue = output.at(i, j);
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

    ImageType image;
    readImage(filename, image);

    ImageType equalizedImage;
    vector<int> histBefore, histAfter;

    double duration = measureRuntime(
        RUNTIME_OUTPUT_PATH,
        histogramEqualization, image, equalizedImage, histBefore, histAfter);

    writeImage(BEFORE_IMAGE_OUTPUT_PATH, image);
    writeImage(AFTER_IMAGE_OUTPUT_PATH, equalizedImage);

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

    return 0;
}
