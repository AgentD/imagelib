#include "image.h"
#include "util.h"

#include <cstring>




/*
	Bitmap loading facilities.

	What should work('-' means implemented but not tested, 'x' means tested and DOES work)
	  - Importing 1 bit per pixel black and white images
	  - Importing 4 bit per pixel color mapped images
	  - Importing 8 bit per pixel color mapped images
	  x Importing 24 bit per pixel images
	  - Importing 1 bit per pixel black and white images with RLE compresion
	  - Importing 4 bit per pixel color mapped images with RLE compresion
	  - Importing 8 bit per pixel color mapped images with RLE compresion
	  - Flipping images upside-down while loading if needed to move the origin to the lower left corner
	  x Exporting 24 bit per pixel RGB images
	  x Exporting 8 bit per pixel gray scale images
 */




CImage::E_LOAD_RESULT CImage::m_loadBmp( std::istream& file )
{
	// Read the file header and extract all interesting information //
	char header[ 54 ];

	file.read( header, 54 );

	char    bfType[3]     = { header[0], header[1], 0 };
	size_t  bfOffBits     = header[10] | ((size_t)header[11])<<8 | ((size_t)header[12])<<16 | ((size_t)header[13])<<24;

	size_t  biSize        = header[14] | ((size_t)header[15])<<8 | ((size_t)header[16])<<8 | ((size_t)header[17])<<8;
	ssize_t	biWidth       = header[18] | ((size_t)header[19])<<8 | ((size_t)header[20])<<8 | ((size_t)header[21])<<8;
	ssize_t biHeight      = header[22] | ((size_t)header[23])<<8 | ((size_t)header[24])<<8 | ((size_t)header[25])<<8;
	size_t  biBitCount    = header[28] | ((size_t)header[29])<<8;
	size_t  biCompression = header[30] | ((size_t)header[31])<<8 | ((size_t)header[32])<<8 | ((size_t)header[33])<<8;
	size_t  biClrUsed     = header[46] | ((size_t)header[47])<<8 | ((size_t)header[48])<<8 | ((size_t)header[49])<<8;

	size_t  biByteCount = biBitCount/8;

	// Check the header data for sanity //
	if( strcmp( bfType, "BM" ) || biSize!=40 || biWidth==0 || biHeight==0 || biCompression>3 ) return ELR_FILE_CORRUPTED;
	if( !(biBitCount==1 || biBitCount==4 || biBitCount==8 || biBitCount==24)                 ) return ELR_FILE_CORRUPTED;
	if( biCompression==1 && (biBitCount!=8  || biHeight<0)                                   ) return ELR_FILE_CORRUPTED;
	if( biCompression==2 && (biBitCount!=4  || biHeight<0)                                   ) return ELR_FILE_CORRUPTED;
	if( biCompression==3 || biBitCount==32 || biBitCount==16                                 ) return ELR_NOT_SUPPORTED;

	// Read the color map if necessary //
	unsigned char colorMap[ 4*265 ];
	bool haveColorMap = false;

	if( biClrUsed==0 )
	{
		if( biBitCount==1 || biBitCount==4 || biBitCount==8 )
		{
			file.read( (char*)colorMap, 4<<biBitCount );
			file.seekg( -4<<biBitCount, std::ios_base::cur );
			haveColorMap = true;
		}
	}
	else
	{
		file.read( (char*)colorMap, 4*biClrUsed );
		file.seekg( -4*biClrUsed, std::ios_base::cur );
		haveColorMap = true;
	}

	file.seekg( bfOffBits-54, std::ios_base::cur );

	// Read the image data //
	bool needVflip = biHeight<0;
	biHeight = (biHeight<0) ? -biHeight : biHeight;

	allocateBuffer( biWidth, biHeight, 1, biBitCount==1 ? EIT_GRAYSCALE8 : EIT_BGR8 );

	size_t padding = (m_width*biByteCount)%4;
	ssize_t ystep = 3*m_width;
	unsigned char *ptr, *end;

	if( needVflip )
	{
		ptr = (unsigned char*)m_imageBuffer + (m_height-1)*ystep;
		end = (unsigned char*)m_imageBuffer - ystep;
		ystep*=-1;
	} else {
		ptr = (unsigned char*)m_imageBuffer;
		end = (unsigned char*)m_imageBuffer + m_height*ystep;
	}


	if( biCompression==0 )
	{
		if( biBitCount==24 )
		{
			for( ; ptr!=end; ptr+=ystep )
			{
				file.read( (char*)ptr, ystep );
				file.seekg( padding, std::ios_base::cur );
			}
		}
		else if( biBitCount==1 )
		{
		}
		else
		{
			for( ; ptr!=end; ptr+=ystep )
			{
				for( size_t i=0; i<m_width; ++i )
				{
					ptr[ 3*i     ] = colorMap[ 4*c     ];
					ptr[ 3*i + 1 ] = colorMap[ 4*c + 1 ];
					ptr[ 3*i + 2 ] = colorMap[ 4*c + 2 ];
				}

				file.seekg( padding, std::ios_base::cur );
			}
		}
	}
	else if( biCompression==1 )
	{
	}
	if( biCompression==2 )
	{
	}
	
	return ELR_SUCESS;
}

void CImage::m_saveBmp( std::ostream& stream )
{
	const char zero[4] = { 0, 0, 0, 0 };
	size_t bytePerPixel, realBPP;
	bool switchRB = false;

	//////////////// Compose us a nice BMP header ////////////////
	char header[ 54 ];

	memset( header, 0, 54 );

	switch( m_type )
	{
	case EIT_GRAYSCALE8: header[ 28 ] =  8; bytePerPixel = 1; realBPP = 1;                  break;
	case EIT_RGB8:       header[ 28 ] = 24; bytePerPixel = 3; realBPP = 3; switchRB = true; break;
	case EIT_RGBA8:      header[ 28 ] = 24; bytePerPixel = 3; realBPP = 4; switchRB = true; break;
	case EIT_BGR8:       header[ 28 ] = 24; bytePerPixel = 3; realBPP = 3;                  break;
	case EIT_BGRA8:      header[ 28 ] = 24; bytePerPixel = 3; realBPP = 4;                  break;
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
	size_t dy = m_width*realBPP, padding = (m_width*bytePerPixel)%4
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

