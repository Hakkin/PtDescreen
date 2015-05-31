#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: %s [input] [output] [DPI]\n", argv[0]);
        return 0;
    }

    printf("Reading input image...");
    int width, height, comp;
    // The final argument to this specifies the number of components to return,
    // we will always be working with 3 (Red, Green and Blue)
    // comp will be the original number of components  in the image
    // It might be better to request 4 components here (RGBA) so we cant't throw away any information
    unsigned char *pixels = stbi_load(argv[1], &width, &height, &comp, 3);
    if (pixels == NULL)
    {
        printf("\nError reading image %s: %s", argv[1], stbi_failure_reason());
    }

    // Descreen image

    printf("\nWriting output image...");
    // We will always output 24bit PNG for now, regardless of output extension
    stbi_write_png(argv[2], width, height, 3, pixels, 0);

    return 0;
}
