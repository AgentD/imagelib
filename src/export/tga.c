#ifdef IMAGE_SAVE_TGA



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

void save_tga( SImage* img, void* file, const SFileIOInterface* io )
{
    unsigned char header[ 18 ];
    size_t bpp = 0;
    unsigned char* ptr;
    unsigned char* end;

    /* generate a TGA header */
    memset( header, 0, 18 );

    WRITE_LITTLE_ENDIAN_16( img->width,  header, 12 );
    WRITE_LITTLE_ENDIAN_16( img->height, header, 14 );

    switch( img->type )
    {
    case ECT_GRAYSCALE8: header[2] = 3; bpp = 1;                 break;
    case ECT_RGB8:       header[2] = 2; bpp = 3;                 break;
    case ECT_RGBA8:      header[2] = 2; bpp = 4; header[17] = 8; break;
    };

    header[16]  = bpp*8;
    header[17] |= 1<<5;     /* origin at the top */

    io->write( header, 1, 18, file );

    /* write image data */
    ptr = img->image_buffer;
    end = ptr + img->width*img->height*bpp;

    if( img->type == ECT_GRAYSCALE8 )
    {
        io->write( img->image_buffer, 1, img->width*img->height, file );
    }
    else
    {
        for( ; ptr!=end; ptr+=bpp )
        {
            unsigned char temp;

            /* swap red and blue */
            temp   = ptr[0];
            ptr[0] = ptr[2];
            ptr[2] = temp;

            io->write( ptr, 1, bpp, file );

            /* swap red and blue back */
            temp   = ptr[0];
            ptr[0] = ptr[2];
            ptr[2] = temp;
        }
    }
}

#endif

