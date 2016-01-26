#include "image.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
    The PBM exporting facilities.

    What should work:
      - exporting ECT_GRAYSCALE8 images
      - exporting ECT_RGB8 images
      - exporting ECT_RGBA8 images
*/

#ifdef IMAGE_SAVE_PBM
#define PIXBUFF 256

void save_pbm( const image_t* img, void* file, const image_io_t* io )
{
    unsigned char temp[ PIXBUFF*3 ], *dst;
    const unsigned char* ptr;
    char text_buffer[ 40 ];
    size_t i, j, k, len;

    switch( img->type )
    {
    case ECT_GRAYSCALE8:
        io->write( "P5\n", 1, 2, file );    /* binary grayscale */
        break;
    case ECT_RGB8:
    case ECT_RGBA8:
        io->write( "P6\n", 1, 2, file );    /* binary RGB */
        break;
    default:
        return;
    };

    sprintf( text_buffer, "%lu %lu\n", (unsigned long)img->width,
                                       (unsigned long)img->height );
    io->write( text_buffer, 1, strlen(text_buffer), file );
    io->write( "255\n", 1, 4, file );

    if( img->type == ECT_GRAYSCALE8 )
    {
        io->write( img->image_buffer, img->width, img->height, file );
    }
    else if( img->type == ECT_RGB8 )
    {
        io->write( img->image_buffer, img->width*3, img->height, file );
    }
    else if( img->type == ECT_RGBA8 )
    {
        ptr = img->image_buffer;

        for( i=0; i<img->height; ++i )
        {
            for( j=0; j<img->width; j+=len )
            {
                len = img->width - j;
                if( len > PIXBUFF )
                    len = PIXBUFF;

                for( dst=temp, k=0; k<len; ++k, dst+=3, ptr+=4 )
                {
                    dst[0] = ptr[0];
                    dst[1] = ptr[1];
                    dst[2] = ptr[2];
                }

                io->write( temp, 3, len, file );
            }
        }
    }
}
#endif

