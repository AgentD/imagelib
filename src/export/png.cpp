#ifdef IMAGE_SAVE_PNG



#include "image.h"



/*
   The PNG exporting facilities.

   What should work('-' means implemented but not tested, 'x' means tested and DOES work):
    x exporting EIT_GRAYSCALE8 images
    x exporting EIT_RGB8 images
    x exporting EIT_RGBA8 images
*/


#include "lodepng.h"

#include <iostream>
#include <cstdlib>



void CImage::m_savePng( std::ostream& stream )
{
    unsigned char* buffer;
    size_t length;
    LodePNGColorType colortype;

    switch( m_type )
    {
    case EIT_GRAYSCALE8: colortype = LCT_GREY; break;
    case EIT_RGB8:       colortype = LCT_RGB;  break;
    case EIT_RGBA8:      colortype = LCT_RGBA; break;
    };

    lodepng_encode_memory( &buffer, &length, (unsigned char*)m_imageBuffer,
                           m_width, m_height, colortype, 8 );

    stream.write( (char*)buffer, length );

    free( buffer );
}

#endif

