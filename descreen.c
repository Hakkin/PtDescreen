#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#include <fftw3.h>
#include "descreen.h"

// Generates magnitude value from a real and imaginary value
static double genMagnitude(double real, double imag);
// Finds peaks in magnitude by comparing all 4 pixels around it, if the specified
// pixel is a peak, it will return a non-zero value, otherwise it will return 0
static int isPeak(int width, int height, fftw_complex *fft, int x, int y);
// Returns distance from (x1, y1) to (x2, y2)
static double distanceFrom(int x1, int y1, int x2, int y2);
// Calculates LPI
static int calcLPI(int width, int height, int dpi, int x, int y);
// Calculates angle
static int calcAngle(int x, int y);

int analyze(descreenConfig *config, int x, int y, int pow2)
{
    // TODO: This detects screentone frequencies and angle decently, but it also
    // has false positives on non-screentoned images. This can probably be fixed by
    // verifying that other peaks corresponding to screentone frequencies exist in the image.
    int analyzeSize = pow(2, pow2);

    // FFTW requires padding in order to perform in-place transforms of real data
    // http://www.fftw.org/doc/Multi_002dDimensional-DFTs-of-Real-Data.html
    int padding = (analyzeSize&1) ? 1 : 2;
    double *dInput  = fftw_alloc_real((analyzeSize+padding)*analyzeSize);
    // Casting double input to complex for output, this makes it easier to work with later
    fftw_complex *cOutput = (fftw_complex *)dInput;
    // This is an in-place transform, despite having difference input and output variables,
    // since they are just different casts of the same address
    fftw_plan plan = fftw_plan_dft_r2c_2d(analyzeSize, analyzeSize, dInput, cOutput, FFTW_ESTIMATE);

    int channelPeaksX[3] = {0},
        channelPeaksY[3] = {0};
    int channelLPI[3] = {0};
    // Processing loop
    for (int channel = 0; channel < 3; channel++)
    {
        // Initializing input array
        // Any out-of-bound pixels will be initialized as 0 (black)
        for (int row = 0; row < analyzeSize; row++)
        {
            for (int column = 0; column < analyzeSize; column++)
            {
                int rowOffset = row+y;
                int columnOffset = column+x;

                double pixel;
                if (rowOffset > config->height-1 || columnOffset > config->width-1)
                {
                    pixel = 0;
                } else
                {
                    pixel = config->pixels[(rowOffset*config->width+columnOffset)*3+channel];
                }
                dInput[row*(analyzeSize+padding)+column] = pixel;
            }
        }
        fftw_execute(plan);

        int peakX = 0,
            peakY = 0;
        double largestPeak = 0;
        int locateWidth  = (analyzeSize+padding)/2,
            locateHeight = analyzeSize/2;
        // row (y) is only looped for analyzeSize/2 because the bottom half of the FFT
        // is mostly symmetrical, all information needed to detect screentones should exist
        // in the top half. column (x) is looped for (analyzeSize+padding)/2 because FFTW's r2c
        // function discards unneeded symmetrical data, the right horizontal half, in this case
        for (int row = 0; row < locateHeight; row++)
        {
            for (int column = 0; column < locateWidth; column++)
            {
                // This first checks if the point is within 15 pixels from the center, if it is then it discards it,
                // this is to make sure we are not getting false positives from the DC component or low frequencies,
                // it then checks if it the pixel is a peak value, if it is, it checks if it is larger than the
                // previous largest peak, if it is we will set it to the new largest peak
                if (distanceFrom(0, 0, column, row) >  analyzeSize/8 &&
                    isPeak(locateWidth, locateHeight, cOutput, row, column) &&
                    genMagnitude(cOutput[row*locateWidth+column][0], cOutput[row*locateWidth+column][1]) > largestPeak)
                {
                    largestPeak = genMagnitude(cOutput[row*locateWidth+column][0], cOutput[row*locateWidth+column][1]);
                    peakX = column;
                    peakY = row;
                }
            }
        }
        channelPeaksX[channel] = peakX;
        channelPeaksY[channel] = peakY;
        channelLPI[channel] = calcLPI(analyzeSize, analyzeSize, config->dpi, peakX, peakY);
    }
    int peakFound = 0;
    // Checking if the detected peaks in each channel match, if 2 or more match, we will set lpi and angle in *config
    // to the detected values
    // TODO: This could probably be cleaned up
    if (channelLPI[0] == channelLPI[1] ||
        channelLPI[0] == channelLPI[2])
    {
        peakFound = 1;
        config->lpi = channelLPI[0];
        config->angle = calcAngle(channelPeaksX[0], channelPeaksY[0]);
    } else if (channelLPI[1] == channelLPI[2])
    {
        peakFound = 1;
        config->lpi = channelLPI[1];
        config->angle = calcAngle(channelPeaksX[1], channelPeaksY[1]);
    }

    fftw_destroy_plan(plan);
    fftw_free(dInput);
    return peakFound;
}

int descreen(descreenConfig *config, int pow2)
{
    return 0;
}

double genMagnitude(double real, double imag)
{
    return sqrt((real*real)+(imag*imag));
}

int isPeak(int width, int height, fftw_complex *fft, int x, int y)
{
    double magValue = genMagnitude(fft[y*width+x][0], fft[y*width+x][1]);

    int isPeak = 1;
    // Checks each pixel around the specified one to check if it's brighter,
    // if it is, isPeak is set to 0 and we will break from the loop
    // TODO: There is probably a better way to do this, but this works for now
    for (int side = 0; side < 4; side++)
    {
        switch (side)
        {
            case 0:
                if (y == 0)
                {
                    break;
                }
                if (magValue <= genMagnitude(fft[(y-1)*width+x][0], fft[(y-1)*width+x][1]))
                {
                    isPeak = 0;
                }
                break;
            case 1:
                if (x >= width-1)
                {
                    break;
                }
                if (magValue <= genMagnitude(fft[y*width+(x+1)][0], fft[y*width+(x+1)][1]))
                {
                    isPeak = 0;
                }
                break;
            case 2:
                if (y >= height-1)
                {
                    break;
                }
                if (magValue <= genMagnitude(fft[(y+1)*width+x][0], fft[(y+1)*width+x][1]))
                {
                    isPeak = 0;
                }
                break;
            case 3:
                if (x == 0)
                {
                    break;
                }
                if (magValue <= genMagnitude(fft[y*width+(x-1)][0], fft[y*width+(x-1)][1]))
                {
                    isPeak = 0;
                }
                break;
            default:
                // This should never happen
                return 0;
        }
        if (isPeak == 0)
        {
            break;
        }
    }

    return isPeak;
}

double distanceFrom(int x1, int y1, int x2, int y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

int calcLPI(int width, int height, int dpi, int x, int y)
{
    double widthInches  = (double)width/dpi,
           heightInches = (double)height/dpi;
    return round(distanceFrom(0, 0, round((double)x/widthInches), round((double)y/heightInches)));
}

int calcAngle(int x, int y)
{
    return round(fmod((atan2(x, y) * 180/M_PI), 30));
}
