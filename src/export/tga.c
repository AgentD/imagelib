#include "image.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>

/*
    The TGA exporting facilities.

    What should work:
      - exporting ECT_GRAYSCALE8 images
      - exporting ECT_RGB8 images
      - exporting ECT_RGBA8 images
*/

#ifdef IMAGE_SAVE_TGA
#define PIXBUFF 128

void save_tga( const image_t* img, void* file, const image_io_t* io )
{
    unsigned char header[ 18 ], temp[ PIXBUFF*4 ], *dst;
    const unsigned char *ptr, *end;
    size_t i = 0, bpp = 0;

    /* generate a TGA header */
    memset( header, 0, 18 );

    WRITE_LITTLE_ENDIAN_16( img->width,  header, 12 );
    WRITE_LITTLE_ENDIAN_16( img->height, header, 14 );

    switch( img->type )
    {
    case ECT_GRAYSCALE8: header[2] = 3; bpp = 1;                 break;
    case ECT_RGB8:       header[2] = 2; bpp = 3;                 break;
    case ECT_RGBA8:      header[2] = 2; bpp = 4; header[17] = 8; break;
    default:                                                     return;
    };

    header[16]  = bpp*8;
    header[17] |= 1<<5;     /* origin at the top */

    io->write( header, 1, 18, file );

    /* write image data */
    ptr = img->image_buffer;
    end = ptr + img->width*img->height*bpp;
    dst = temp;

    if( img->type == ECT_GRAYSCALE8 )
    {
        io->write( img->image_buffer, img->width, img->height, file );
    }
    else if( img->type == ECT_RGB8 )
    {
        while( ptr!=end )
        {
            dst[0] = ptr[2];
            dst[1] = ptr[1];
            dst[2] = ptr[0];
            ptr += 3;
            dst += 3;
            ++i;

            if( i >= PIXBUFF )
            {
                io->write( temp, 3, PIXBUFF, file );
                i = 0;
                dst = temp;
            }
        }
    }
    else if( img->type == ECT_RGBA8 )
    {
        while( ptr!=end )
        {
            dst[0] = ptr[2];
            dst[1] = ptr[1];
            dst[2] = ptr[0];
            dst[3] = ptr[3];
            ptr += 4;
            dst += 4;
            ++i;

            if( i >= PIXBUFF )
            {
                io->write( temp, 4, PIXBUFF, file );
                i = 0;
                dst = temp;
            }
        }
    }

    if( i )
        io->write( temp, 1, i*bpp, file );
}
#endif

