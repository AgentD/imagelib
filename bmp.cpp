#include "image.h"
#include "util.h"

#include <cstring>




/*
	Bitmap loading facilities.

	What should work('-' means implemented but not tested, 'x' means tested and DOES work)
	  x Exporting 24 bit per pixel RGB images
	  x Exporting 8 bit per pixel gray scale images
 */




namespace
{
	enum
	{
		BI_RGB = 0,
		BI_RLE8 = 1,
		BI_RLE4 = 2,
		BI_BITFIELDS = 3
	};
}

CImage::E_LOAD_RESULT CImage::m_loadBmp( std::istream& file )
{
	char header[ 54 ];
}

void CImage::m_saveBmp( std::ostream& stream )
{
	const char zero[4] = { 0, 0, 0, 0 };
	size_t bytePerPixel, realBPP, padding = m_width%4;
	bool switchRB = false;

	//////////////// Compose us a nice BMP header ////////////////
	char header[ 54 ];

	memset( header, 0, 54 );

	switch( m_type )
	{
	case EIT_GRAYSCALE8: header[ 28 ] =  8;                  bytePerPixel = 1; realBPP = 1; break;
	case EIT_RGB8:       header[ 28 ] = 24; switchRB = true; bytePerPixel = 3; realBPP = 3; break;
	case EIT_RGBA8:      header[ 28 ] = 24; switchRB = true; bytePerPixel = 3; realBPP = 4; break;
	case EIT_BGR8:       header[ 28 ] = 24;                  bytePerPixel = 3; realBPP = 3; break;
	case EIT_BGRA8:      header[ 28 ] = 24;                  bytePerPixel = 3; realBPP = 4; break;
	};

	size_t size = m_width*m_height*bytePerPixel;

	header[  0 ] = 'B';
	header[  1 ] = 'M';
	header[ 10 ] = 54;
	header[ 14 ] = 40;
	header[ 26 ] = 1;

	WRITE_LITTLE_ENDIAN_32( size,     header,  2 );
	WRITE_LITTLE_ENDIAN_32( m_width,  header, 18 );
	WRITE_LITTLE_ENDIAN_32( m_height, header, 22 );

	if( m_type==EIT_GRAYSCALE8 )
	{
		WRITE_LITTLE_ENDIAN_32( 1078, header, 10 );
		WRITE_LITTLE_ENDIAN_32( 256,  header, 46 );
	}

	stream.write( header, 54 );

	///////// Write a dummy color map for grayscale images ////////
	if( m_type==EIT_GRAYSCALE8 )
	{
		for( size_t i=0; i<256; ++i )
		{
			unsigned char v[4] = { i, i, i, 0 };

			stream.write( (char*)v, 4 );
		}
	}

	////////////// Write the image data to the file //////////////
	size_t dy = m_width*realBPP;
	unsigned char* ptr = (unsigned char*)m_imageBuffer;
	unsigned char* end = (unsigned char*)m_imageBuffer + m_height*dy;

	if( switchRB )
	{
		for( ; ptr!=end; ptr+=dy )
		{
			for( size_t x=0; x<dy; x+=realBPP )
			{
				util::xorSwap( ptr[ x ], ptr[ x + 2 ] );

				stream.write( (char*)&ptr[ x ], bytePerPixel );

				util::xorSwap( ptr[ x ], ptr[ x + 2 ] );
			}

			stream.write( zero, padding );
		}
	}
	else
	{
		for( ; ptr!=end; ptr+=dy )
		{
			for( size_t x=0; x<dy; x+=realBPP )
				stream.write( (char*)&ptr[ x ], bytePerPixel );

			stream.write( zero, padding );
		}
	}
}

