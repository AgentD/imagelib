#ifdef IMAGE_SAVE_PNG



#include "image.h"



/*
   The PNG exporting facilities.

   What should work:
    - exporting EIT_GRAYSCALE8 images
    - exporting EIT_RGB8 images
    - exporting EIT_RGBA8 images
*/


#include "lodepng.h"

#include <stdlib.h>



void save_png( SImage* img, FILE* file )
{
    unsigned char* buffer;
    size_t length;
    LodePNGColorType colortype;

    switch( img->type )
    {
    case EIT_GRAYSCALE8: colortype = LCT_GREY; break;
    case EIT_RGB8:       colortype = LCT_RGB;  break;
    case EIT_RGBA8:      colortype = LCT_RGBA; break;
    };

    lodepng_encode_memory( &buffer, &length, img->image_buffer,
                           img->width, img->height, colortype, 8 );

    fwrite( buffer, 1, length, file );

    free( buffer );
}

#endif
