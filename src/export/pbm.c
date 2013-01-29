#ifdef IMAGE_SAVE_PBM

#include "image.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>



/*
    The PBM exporting facilities.

    What should work:
      - exporting ECT_GRAYSCALE8 images
      - exporting ECT_RGB8 images
      - exporting ECT_RGBA8 images
*/

void save_pbm( SImage* img, void* file, const SFileIOInterface* io )
{
    char text_buffer[ 40 ];
    unsigned char* ptr;
    size_t i, j;

    /* generate magic value */
    switch( img->type )
    {
    case ECT_GRAYSCALE8:
        io->write( "P5\n", 1, 2, file );    /* binary grayscale */
        break;
    case ECT_RGB8:
    case ECT_RGBA8:
        io->write( "P6\n", 1, 2, file );    /* binary RGB */
        break;
    };

    /* <width> <height>\n */
    sprintf( text_buffer, "%zu %zu\n", img->width, img->height );
    io->write( text_buffer, 1, strlen(text_buffer), file );

    /* <max color value>\n */
    io->write( "255\n", 1, 4, file );

    /* raw image data */
    if( img->type == ECT_GRAYSCALE8 )
    {
        io->write( img->image_buffer, 1, img->width*img->height, file );
    }
    else if( img->type == ECT_RGB8 )
    {
        io->write( img->image_buffer, 1, img->width*img->height*3, file );
    }
    else if( img->type == ECT_RGBA8 )
    {
        ptr = img->image_buffer;

        for( i=0; i<img->height; ++i )
        {
            for( j=0; j<img->width; ++j )
            {
                io->write( ptr, 1, 3, file );
                ptr += 4;
            }
        }
    }
}

#endif

