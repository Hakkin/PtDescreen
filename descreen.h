#ifndef DESCREEN_H_INCLUDED
#define DESCREEN_H_INCLUDED

typedef struct
{
    unsigned char *pixels;
    int width;
    int height;
    int dpi;
    int lpi;
    int angle;

} descreenConfig;

// analyze() will analyze a 2^pow2 sized square at (x, y) in *pixels,
// if it detects a screentone, it will set lpi and angle in *config
// to the detected values and will return a non-zero value.
// If no screentone could be detected, it will return 0 and *config will be unmodified.
int analyze(descreenConfig *config, unsigned int x, unsigned int y, unsigned int pow2);

// descreen() will apply a descreen filter to *pixels using the
// parameters provided in *config, using a 2^pow2 sized square window.
int descreen(descreenConfig *config, unsigned int pow2);

#endif // DESCREEN_H_INCLUDED
