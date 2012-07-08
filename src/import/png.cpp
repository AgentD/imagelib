#ifdef IMAGE_LOAD_PNG



#include "image.h"



/*
    The PNG loading facilities.

    What should work('-' means implemented but not tested, 'x' means tested
                     and DOES work):
      x Importing PNG images using LodePNG and storing them as RGBA8
*/


#include "lodepng.h"

#include <iostream>
#include <cstdlib>



CImage::E_LOAD_RESULT CImage::m_loadPng( std::istream& stream )
{
    int bitDepth, colorType;
    unsigned int width, height, result;
    unsigned char* input;
    size_t cur, length;

    if( m_imageBuffer )
        free( m_imageBuffer );

    /* read the stream into a buffer */
    cur = stream.tellg( );
    stream.seekg( 0, std::ios_base::end );

    length = ((size_t)stream.tellg( )) - cur;
    stream.seekg( cur, std::ios_base::beg );

    input = (unsigned char*)malloc( length );

    stream.read( (char*)input, length );

    /* decode the image */
    result = lodepng_decode32( (unsigned char**)(&m_imageBuffer),
                               &width, &height, input, length );

    free( input );

    if( result != 0 )
    {
        free( m_imageBuffer );

        m_imageBuffer = NULL;
        m_width = m_height = m_depth = 0;

        return ELR_FILE_CORRUPTED;
    }

    /* store the image properties */
    m_width  = width;
    m_height = height;
    m_depth  = 1;
    m_type   = EIT_RGBA8;

    return ELR_SUCESS;
}

#endif

