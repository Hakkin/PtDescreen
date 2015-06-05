#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <fftw3.h>
#include "descreen.h"

// Generates magnitude value from a real and imaginary value
double genMagnitude(double real, double imag);
// Finds peaks in magnitude by comparing all 4 pixels around it, if the specified
// pixel is a peak, it will return a non-zero value, otherwise it will return 0
int isPeak(unsigned int width, unsigned int height, fftw_complex *fft, unsigned int x, unsigned int y);
// Returns distance from (x1, y1) to (x2, y2)
double distanceFrom(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

int analyze(descreenConfig *config, unsigned int x, unsigned int y, unsigned int pow2)
{
    unsigned int analyzeSize = pow(2, pow2);

    // FFTW requires padding in order to perform in-place transforms of real data
    // http://www.fftw.org/doc/Multi_002dDimensional-DFTs-of-Real-Data.html
    int padding = (analyzeSize&1) ? 1 : 2;
    double *dInput  = fftw_alloc_real((analyzeSize+padding)*analyzeSize);
    // Casting double input to complex for output, this makes it easier to work with later
    fftw_complex *cOutput = (fftw_complex *)dInput;
    // This is an in-place transform, despite having difference input and output variables,
    // since they are just different casts of the same address
    fftw_plan plan = fftw_plan_dft_r2c_2d(analyzeSize, analyzeSize, dInput, cOutput, FFTW_ESTIMATE);

    // Processing loop
    for (int channel = 0; channel < 3; channel++)
    {
        // Initializing input array
        // Any out-of-bound pixels will be initialized as 0 (black)
        for (unsigned int row = 0; row < analyzeSize; row++)
        {
            for (unsigned int column = 0; column < analyzeSize; column++)
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

        unsigned int peakX = 0,
                     peakY = 0;
        double largestPeak = 0;
        double widthInches  = (double)analyzeSize/config->dpi,
               heightInches = (double)analyzeSize/config->dpi;
        unsigned int locateWidth  = (analyzeSize+padding)/2,
                     locateHeight = analyzeSize/2;
        // row (y) is only looped for analyzeSize/2 because the bottom half of the FFT
        // is mostly symmetrical, all information needed to detect screentones should exist
        // in the top half. column (x) is looped for (analyzeSize+padding)/2 because FFTW's r2c
        // function discards unneeded symmetrical data, the right horizontal half, in this case
        for (unsigned int row = 0; row < locateHeight; row++)
        {
            for (unsigned int column = 0; column < locateWidth; column++)
            {
                // This first checks if the point is within 15 pixels from the center, if it is then it discards it,
                // this is to make sure we are not getting false positives from the DC component or low frequencies,
                // it then checks if it the pixel is a peak value, if it is, it checks if it is larger than the
                // previous largest peak, if it is we will set it to the new largest peak
                if (distanceFrom(0, 0, column, row) > 15 &&
                    isPeak(locateWidth, locateHeight, cOutput, row, column) &&
                    genMagnitude(cOutput[row*locateWidth+column][0], cOutput[row*locateWidth+column][1]) > largestPeak)
                {
                    largestPeak = genMagnitude(cOutput[row*locateWidth+column][0], cOutput[row*locateWidth+column][1]);
                    peakX = column;
                    peakY = row;
                }
            }
        }
    }

    fftw_destroy_plan(plan);
    fftw_free(dInput);
    return 1;
}

int descreen(descreenConfig *config, unsigned int pow2)
{
    return 0;
}

double genMagnitude(double real, double imag)
{
    return sqrt((real*real)+(imag*imag));
}

int isPeak(unsigned int width, unsigned int height, fftw_complex *fft, unsigned int x, unsigned int y)
{
    double magValue = genMagnitude(fft[y*width+x][0], fft[y*width+x][1]);

    int isPeak = 1;
    // Checks each pixel around the specified one to check if it's brighter,
    // if it is, isPeak is set to 0 and we will break from the loop
    // TODO: There is probably a better way to do this, but this works for now
    for (unsigned int side = 0; side < 4; side++)
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

double distanceFrom(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}
