#ifdef IMAGE_SAVE_TGA



#include "image.h"
#include "util.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>



/*
    The TGA exporting facilities.

    What should work:
      - exporting EIT_GRAYSCALE8 images
      - exporting EIT_RGB8 images
      - exporting EIT_RGBA8 images
*/

void CImage::m_saveTga( FILE* file )
{
    unsigned char header[ 18 ];
    size_t bpp = 0;
    unsigned char* ptr;
    unsigned char* end;

    /* generate TGA header */
    memset( header, 0, 18 );

    WRITE_LITTLE_ENDIAN_16( m_width,  header, 12 );
    WRITE_LITTLE_ENDIAN_16( m_height, header, 14 );

    switch( m_type )
    {
    case EIT_GRAYSCALE8: header[2] = 3; bpp = 1;                 break;
    case EIT_RGB8:       header[2] = 2; bpp = 3;                 break;
    case EIT_RGBA8:      header[2] = 2; bpp = 4; header[17] = 8; break;
    };

    header[16]  = bpp*8;
    header[17] |= 1<<5;     /* origin at the top */

    fwrite( header, 1, 18, file );

    /* write image data */
    ptr = (unsigned char*)m_imageBuffer;
    end = ptr + m_width*m_height*bpp;

    if( m_type == EIT_GRAYSCALE8 )
    {
        fwrite( m_imageBuffer, 1, m_width*m_height, file );
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

            fwrite( ptr, 1, bpp, file );

            /* swap red and blue back */
            temp   = ptr[0];
            ptr[0] = ptr[2];
            ptr[2] = temp;
        }
    }
}

#endif

