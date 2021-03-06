#include "image.h"

/*
   The PNG exporting facilities.

   What should work:
    - exporting ECT_GRAYSCALE8 images
    - exporting ECT_RGB8 images
    - exporting ECT_RGBA8 images
*/

#ifdef IMAGE_SAVE_PNG
#include "lodepng.h"

#include <stdlib.h>

void save_png( const image_t* img, void* file, const image_io_t* io )
{
    unsigned char* buffer;
    size_t length;
    LodePNGColorType colortype;

    switch( img->type )
    {
    case ECT_GRAYSCALE8: colortype = LCT_GREY; break;
    case ECT_RGB8:       colortype = LCT_RGB;  break;
    case ECT_RGBA8:      colortype = LCT_RGBA; break;
    default:                                   return;
    };

    lodepng_encode_memory( &buffer, &length, img->image_buffer,
                           img->width, img->height, colortype, 8 );

    io->write( buffer, 1, length, file );

    free( buffer );
}
#endif

