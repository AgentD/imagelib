#ifdef IMAGE_SAVE_BMP



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



void save_bmp( SImage* img, void* file, const SFileIOInterface* io )
{
    const char zero[4] = { 0, 0, 0, 0 };
    size_t bpp, realBPP, size, i, dy, padding, x;
    unsigned char header[ 54 ];
    unsigned char* ptr;
    unsigned char* end;

    /********* generate a BMP header *********/
    memset( header, 0, 54 );

    switch( img->type )
    {
    case ECT_GRAYSCALE8: header[28] =  8; bpp = 1; realBPP = 1; break;
    case ECT_RGB8:       header[28] = 24; bpp = 3; realBPP = 3; break;
    case ECT_RGBA8:      header[28] = 24; bpp = 3; realBPP = 4; break;
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
        for( i=0; i<256; ++i )
        {
            unsigned char v[4] = { i, i, i, 0 };

            io->write( v, 1, 4, file );
        }
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
            io->write( ptr,  1, dy*bpp,  file );
            io->write( zero, 1, padding, file );
        }
    }
    else
    {
        for( ; ptr!=end; ptr+=dy )
        {
            for( x=0; x<dy; x+=realBPP )
            {
                unsigned char temp;

                /* swap red and blue */
                temp     = ptr[x];
                ptr[x  ] = ptr[x+2];
                ptr[x+2] = temp;

                io->write( &ptr[ x ], 1, bpp, file );

                /* swap red and blue */
                temp     = ptr[x];
                ptr[x  ] = ptr[x+2];
                ptr[x+2] = temp;
            }

            io->write( zero, 1, padding, file );
        }
    }
}

#endif

