#ifdef IMAGE_LOAD_PNG



#include "image.h"



/*
    The PNG loading facilities.

    What should work:
      - Importing PNG images using LodePNG and storing them as RGBA8
*/


#include "lodepng.h"

#include <cstdlib>



E_LOAD_RESULT CImage::m_loadPng( FILE* file )
{
    int bitDepth, colorType;
    unsigned int width, height, result;
    unsigned char* input;
    size_t length;

    if( m_imageBuffer )
        free( m_imageBuffer );

    /* read the file into a buffer */
    fseek( file, 0, SEEK_END );
    length = ftell( file );
    fseek( file, 0, SEEK_SET );

    input = (unsigned char*)malloc( length );

    fread( input, 1, length, file );

    /* decode the image */
    result = lodepng_decode32( (unsigned char**)(&m_imageBuffer),
                               &width, &height, input, length );

    free( input );

    if( result != 0 )
    {
        free( m_imageBuffer );

        m_imageBuffer = NULL;
        m_width = m_height = 0;

        return ELR_FILE_CORRUPTED;
    }

    /* store the image properties */
    m_width  = width;
    m_height = height;
    m_type   = EIT_RGBA8;

    return ELR_SUCESS;
}

#endif

