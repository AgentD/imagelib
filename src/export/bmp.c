#include "image.h"
#include "util.h"

#include <string.h>

/*
    Bitmap exporting facilities.

    What should work
      - exporting ECT_GRAYSCALE8 images
      - exporting ECT_RGB8 images
      - exporting ECT_RGBA8 images
*/

#ifdef IMAGE_SAVE_BMP
#define PIXBUFF 256

static const char zero[4] = { 0, 0, 0, 0 };

void save_bmp( const image_t* img, void* file, const image_io_t* io )
{
    unsigned char header[ 54 ], temp[PIXBUFF*3], *dst;
    size_t bpp, realBPP, size, i, dy, padding, x;
    const unsigned char *ptr, *end;

    /********* generate a BMP header *********/
    memset( header, 0, 54 );

    switch( img->type )
    {
    case ECT_GRAYSCALE8: header[28] =  8; bpp = 1; realBPP = 1; break;
    case ECT_RGB8:       header[28] = 24; bpp = 3; realBPP = 3; break;
    case ECT_RGBA8:      header[28] = 24; bpp = 3; realBPP = 4; break;
    default:                                                    return;
    };

    size = img->width * img->height*bpp;

    header[ 0] = 'B';
    header[ 1] = 'M';
    header[10] = 54;
    header[14] = 40;
    header[26] = 1;

    WRITE_LITTLE_ENDIAN_32(  size,        header,  2 );
    WRITE_LITTLE_ENDIAN_32(  img->width,  header, 18 );
    WRITE_LITTLE_ENDIAN_32( -img->height, header, 22 );

    if( img->type==ECT_GRAYSCALE8 )
    {
        WRITE_LITTLE_ENDIAN_32( 1078, header, 10 );
        WRITE_LITTLE_ENDIAN_32( 256,  header, 46 );
    }

    io->write( header, 1, 54, file );

    /********* Write a dummy color map for grayscale images ********/
    if( img->type==ECT_GRAYSCALE8 )
    {
        for( x=0, i=0; i<256; ++i )
        {
            temp[x++] = i;
            temp[x++] = i;
            temp[x++] = i;
            temp[x++] = 0;

            if( x >= sizeof(temp) )
            {
                io->write( temp, 1, x, file );
                x = 0;
            }
        }

        if( x )
            io->write( temp, 1, x, file );
    }

    /********* Write the image data to the file *********/
    dy = img->width * realBPP;
    padding = (img->width*bpp) % 4;
    ptr = img->image_buffer;
    end = ptr + img->height*dy;

    if( img->type==ECT_GRAYSCALE8 )
    {
        for( ; ptr!=end; ptr+=dy )
        {
            io->write( ptr,  1, dy,      file );
            io->write( zero, 1, padding, file );
        }
    }
    else
    {
        while( ptr!=end )
        {
            for( x=0; x<img->width; x+=size )
            {
                size = img->width - x;

                if( size > PIXBUFF )
                    size = PIXBUFF;

                for( dst=temp, i=0; i<size; ++i, dst+=3, ptr+=realBPP )
                {
                    dst[0] = ptr[2];
                    dst[1] = ptr[1];
                    dst[2] = ptr[0];
                }

                io->write( temp, 1, bpp * size, file );
            }

            io->write( zero, 1, padding, file );
        }
    }
}
#endif

