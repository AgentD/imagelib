#ifdef IMAGE_SAVE_BMP



#include "image.h"
#include "util.h"

#include <cstring>



/*
    Bitmap exporting facilities.

    What should work
      - exporting EIT_GRAYSCALE8 images
      - exporting EIT_RGB8 images
      - exporting EIT_RGBA8 images
*/



void CImage::m_saveBmp( FILE* file )
{
    const char zero[4] = { 0, 0, 0, 0 };
    size_t bpp, realBPP;

    /********* generate BMP header *********/
    unsigned char header[ 54 ];

    memset( header, 0, 54 );

    switch( m_type )
    {
    case EIT_GRAYSCALE8: header[ 28 ] =  8; bpp = 1; realBPP = 1; break;
    case EIT_RGB8:       header[ 28 ] = 24; bpp = 3; realBPP = 3; break;
    case EIT_RGBA8:      header[ 28 ] = 24; bpp = 3; realBPP = 4; break;
    };

    size_t size = m_width*m_height*bpp;

    header[  0 ] = 'B';
    header[  1 ] = 'M';
    header[ 10 ] = 54;
    header[ 14 ] = 40;
    header[ 26 ] = 1;

    WRITE_LITTLE_ENDIAN_32(  size,     header,  2 );
    WRITE_LITTLE_ENDIAN_32(  m_width,  header, 18 );
    WRITE_LITTLE_ENDIAN_32( -m_height, header, 22 );

    if( m_type==EIT_GRAYSCALE8 )
    {
        WRITE_LITTLE_ENDIAN_32( 1078, header, 10 );
        WRITE_LITTLE_ENDIAN_32( 256,  header, 46 );
    }

    fwrite( header, 1, 54, file );

    /********* Write a dummy color map for grayscale images ********/
    if( m_type==EIT_GRAYSCALE8 )
    {
        for( size_t i=0; i<256; ++i )
        {
            unsigned char v[4] = { i, i, i, 0 };

            fwrite( v, 1, 4, file );
        }
    }

    /********* Write the image data to the file *********/
    size_t dy = m_width*realBPP, padding = (m_width*bpp)%4;
    unsigned char* ptr = (unsigned char*)m_imageBuffer;
    unsigned char* end = ptr + m_height*dy;

    if( m_type==EIT_GRAYSCALE8 )
    {
        for( ; ptr!=end; ptr+=dy )
        {
            fwrite( ptr,  1, dy*bpp,  file );
            fwrite( zero, 1, padding, file );
        }
    }
    else
    {
        for( ; ptr!=end; ptr+=dy )
        {
            for( size_t x=0; x<dy; x+=realBPP )
            {
                unsigned char temp;

                /* swap red and blue */
                temp     = ptr[x];
                ptr[x  ] = ptr[x+2];
                ptr[x+2] = temp;

                fwrite( &ptr[ x ], 1, bpp, file );

                /* swap red and blue */
                temp     = ptr[x];
                ptr[x  ] = ptr[x+2];
                ptr[x+2] = temp;
            }

            fwrite( zero, 1, padding, file );
        }
    }
}

#endif

