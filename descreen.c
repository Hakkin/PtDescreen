#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <fftw3.h>
#include "descreen.h"

int analyze(descreenConfig *config, unsigned int x, unsigned int y, unsigned int pow2)
{
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

    }

    fftw_destroy_plan(plan);
    fftw_free(dInput);
    return 1;
}

int descreen(descreenConfig *config, unsigned int pow2)
{
    return 0;
}
