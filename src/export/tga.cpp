#include "image.h"
#include "util.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>



#ifdef IMAGE_SAVE_TGA

/*
	The TGA exporting facilities.

	What should work('-' means implemented but not tested, 'x' means tested and DOES work):
	 x exporting EIT_GRAYSCALE8 images
	 x exporting EIT_RGB8 images
	 x exporting EIT_RGBA8 images
	 x exporting EIT_BGR8 images
	 x exporting EIT_BGRA8 images
*/

void CImage::m_saveTga( std::ostream& stream )
{
	char header[ 18 ];
	bool switchRB = false;
	size_t bytePerPixel = 0;

	memset( header, 0, 18 );

	WRITE_LITTLE_ENDIAN_16( m_width,  header, 12 );
	WRITE_LITTLE_ENDIAN_16( m_height, header, 14 );

	switch( m_type )
	{
	case EIT_GRAYSCALE8: header[2] = 1; bytePerPixel = 1;                                  break;
	case EIT_RGB8:       header[2] = 2;       bytePerPixel = 3;                 switchRB = true; break;
	case EIT_RGBA8:      header[2] = 2;       bytePerPixel = 4; header[17] = 8; switchRB = true; break;
	case EIT_BGR8:       header[2] = 2;       bytePerPixel = 3;                                  break;
	case EIT_BGRA8:      header[2] = 2;       bytePerPixel = 4; header[17] = 8;                  break;
	};

	header[16] = bytePerPixel*8;

	stream.write( header, 18 );


	if( switchRB )
	{
		unsigned char* ptr = (unsigned char*)m_imageBuffer;
		unsigned char* end = ptr + m_width*m_height*bytePerPixel;

		for( ; ptr!=end; ptr+=bytePerPixel )
		{
			util::xorSwap( ptr[0], ptr[2] );

			stream.write( (char*)ptr, bytePerPixel );

			util::xorSwap( ptr[0], ptr[2] );
		}
	}
	else
	{
		stream.write( (char*)m_imageBuffer, m_width*m_height*bytePerPixel );
	}
}

#endif

