#ifdef IMAGE_LOAD_PNG



#include "image.h"



/*
    The PNG loading facilities.

    What should work:
      - Importing PNG images using LodePNG and storing them as RGBA8
*/


#include "lodepng.h"

#include <stdlib.h>



E_LOAD_RESULT load_png( SImage* img, void* file, const SFileIOInterface* io )
{
    int bitDepth, colorType;
    unsigned int width, height, result;
    unsigned char* input;
    size_t length;

    if( img->image_buffer )
        free( img->image_buffer );

    /* read the file into a buffer */
    io->seek( file, 0, SEEK_END );
    length = io->tell( file );
    io->seek( file, 0, SEEK_SET );

    input = malloc( length );

    io->read( input, 1, length, file );

    /* decode the image */
    result = lodepng_decode32( (unsigned char**)&img->image_buffer,
                               &width, &height, input, length );

    free( input );

    if( result != 0 )
    {
        free( img->image_buffer );

        img->image_buffer = NULL;
        img->width        = 0;
        img->height       = 0;

        return ELR_FILE_CORRUPTED;
    }

    /* store the image properties */
    img->width  = width;
    img->height = height;
    img->type   = ECT_RGBA8;

    return ELR_SUCESS;
}

#endif

