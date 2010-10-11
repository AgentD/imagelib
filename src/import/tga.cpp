#include "image.h"
#include "util.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>



#ifdef IMAGE_LOAD_TGA

/*
	The TGA loading facilities.

	What should work('-' means implemented but not tested, 'x' means tested and DOES work):
	  x Importing 8 bit per pixel gray scale images
	  - Importing color mapped 24/32 bit per pixel RGB/RGBA images
	  x Importing 24&32 bit per pixel RGB/RGBA images
	  - Importing 8 bit per pixel gray scale images with RLE compression
	  - Importing color mapped 24/32 bit per pixel RGB/RGBA images with RLE compression
	  x Importing 24&32 bit per pixel RGB/RGBA images with RLE compression
	  x Flip the image vertically during loading if needed to move the origin to the lower left corner
	  - Flip the image horizontally during loading if needed to move the origin to the lower left corner
*/

namespace
{
	enum
	{
		COLOR_MAPPED = 1,
		RGB = 2,
		GRAYSCALE = 3,
	
		COLOR_MAPPED_RLE = 9,
		RGB_RLE = 10,
		GRAYSCALE_RLE = 11
	};

	struct tgaInfo
	{
		size_t width;					// The width of the image in pixels
		size_t height;					// The height of the image in pixels
		size_t bytePerPixel;			// The total number of bytes per pixel over all color channels
		size_t colorMapBytePerPixel;	// The total number of bytes per pixel in the color map over all color channels

		unsigned char* ptr;				// A pointer to the first pixel in the image buffer
		unsigned char* end;				// A pointer to the last pixel in the image buffer
		ssize_t xstep;					// How much to add to the pixel poiter to get to the next pixel
		ssize_t ystep;					// How much to add to the pixel pointer to get to the next pixel row
	};

	void loadTgaColorMapped( std::istream& file, tgaInfo& i, unsigned char* colorMap )
	{
		for( ; i.ptr!=i.end; i.ptr+=i.ystep )
		{
			unsigned char* row = i.ptr;

			for( size_t j=0; j<i.width; ++j, row+=i.xstep )
			{
				unsigned char c0[ 4 ] = { 0, 0, 0, 0 };
				file.read( (char*)c0, i.bytePerPixel );
				size_t c = ((size_t)c0[0]) | ((size_t)c0[1])<<8 | ((size_t)c0[2])<<16 | ((size_t)c0[3])<<24;

				unsigned char* mapPtr = colorMap + c*i.colorMapBytePerPixel;

				for( size_t k=0; k<i.colorMapBytePerPixel; ++k )
					row[k] = mapPtr[k];
			}
		}
	}

	void loadTgaColorMappedRLE( std::istream& file, tgaInfo& i, unsigned char* colorMap )
	{
		unsigned char *t, run=0, raw=0, packet=0, data[ 4 ] = {0,0,0,0};

		for( ; i.ptr!=i.end; i.ptr+=i.ystep )
		{
			unsigned char* row = i.ptr;

			for( size_t j=0; j<i.width; ++j, row+=i.xstep )
			{
				if( run )
				{
					--run;
				}
				else if( raw )
				{
					file.read( (char*)data, i.bytePerPixel );
					size_t c = ((size_t)data[0]) | ((size_t)data[1])<<8 | ((size_t)data[2])<<16 | ((size_t)data[3])<<24;
					t = &colorMap[ c*i.colorMapBytePerPixel ];
					--raw;
				}
				else
				{
					file.read( (char*)&packet, 1 );
					file.read( (char*)data, i.bytePerPixel );
					size_t c = ((size_t)data[0]) | ((size_t)data[1])<<8 | ((size_t)data[2])<<16 | ((size_t)data[3])<<24;
					t = &colorMap[ c*i.colorMapBytePerPixel ];

					if( packet & 0x80 ) run = packet & 0x7F;
					else                raw = packet;
				}

				for( size_t k=0; k<i.colorMapBytePerPixel; ++k )
					row[k] = t[k];
			}
		}
	}

	void loadTgaRGB( std::istream& file, tgaInfo& i )
	{
		for( ; i.ptr!=i.end; i.ptr+=i.ystep )
		{
			unsigned char* row = i.ptr;

			for( size_t j=0; j<i.width; ++j, row+=i.xstep )
			{
				file.read( (char*)row, i.bytePerPixel );
			}
		}
	}

	void loadTgaRGBRLE( std::istream& file, tgaInfo& i )
	{
		unsigned char run=0, raw=0, packet=0, data[ 4 ] = {0,0,0,0};

		for( ; i.ptr!=i.end; i.ptr+=i.ystep )
		{
			unsigned char* row = i.ptr;

			for( size_t j=0; j<i.width; ++j, row+=i.xstep )
			{
				if( run )
				{
					--run;
				}
				else if( raw )
				{
					file.read( (char*)data, i.bytePerPixel );
					--raw;
				}
				else
				{
					file.read( (char*)&packet, 1 );
					file.read( (char*)data, i.bytePerPixel );

					if( packet & 0x80 ) run = packet & 0x7F;
					else                raw = packet;
				}

				for( size_t k=0; k<i.bytePerPixel; ++k )
					row[k] = data[k];
			}
		}
	}
}



CImage::E_LOAD_RESULT CImage::m_loadTga( std::istream& file )
{
	////////////////////// Read the TGA File Header ////////////////////////
	char header[ 18 ];

	file.read( header, 18 );
	file.seekg( header[0], std::ios_base::cur );	// Skip the image ID field(after the header)

	///////// Extract all interesting information from the header /////////
	size_t pictureType     = header[ 2 ];
	size_t pictureWidth    = ((size_t)header[ 12 ]) | (((size_t)header[ 13 ])<<8);
	size_t pictureHeight   = ((size_t)header[ 14 ]) | (((size_t)header[ 15 ])<<8);
	size_t bitsPerPixel    = header[ 16 ];
	size_t attributeByte   = header[ 17 ];

	bool   colorMapPresent = header[ 1 ];
	size_t colorMapOffset  = ((size_t)header[ 3 ]) | (((size_t)header[ 4 ])<<8);
	size_t colorMapLength  = ((size_t)header[ 5 ]) | (((size_t)header[ 6 ])<<8);
	size_t colorMapBPP     = header[ 7 ];

	/////////////////// Check the header data for sanity //////////////////
	if( pictureType<1 || pictureType>11 || (pictureType>3&&pictureType<9)                      ) return ELR_FILE_CORRUPTED;
	if( !pictureWidth || !pictureHeight                                                        ) return ELR_FILE_CORRUPTED;
	if( (pictureType==1||pictureType==9) && (!colorMapPresent||colorMapOffset>=colorMapLength) ) return ELR_FILE_CORRUPTED;

	if( colorMapBPP !=24 && colorMapBPP !=32 && colorMapPresent                                        ) return ELR_NOT_SUPPORTED;
	if( bitsPerPixel!=8                      && (pictureType==GRAYSCALE || pictureType==GRAYSCALE_RLE) ) return ELR_NOT_SUPPORTED;
	if( bitsPerPixel!=24 && bitsPerPixel!=32 && (pictureType==RGB       || pictureType==RGB_RLE      ) ) return ELR_NOT_SUPPORTED;

	//////////////////// Load the image from the file /////////////////////
	size_t bytesPerPixel        = bitsPerPixel/8;
	size_t colorMapBytePerPixel = colorMapBPP/8;
	size_t colorMapSize         = (colorMapLength-colorMapOffset)*colorMapBytePerPixel;
	unsigned char* colorMap     = NULL;

	E_IMAGE_TYPE type = EIT_GRAYSCALE8;

	if( pictureType==RGB || pictureType==RGB_RLE )
		type = (bytesPerPixel==4) ? EIT_BGRA8 : EIT_BGR8;

	if( colorMapPresent )
	{
		type     = colorMapBytePerPixel==4 ? EIT_BGRA8 : EIT_BGR8;
		colorMap = (unsigned char*)malloc( colorMapSize );

		file.seekg( colorMapOffset, std::ios_base::cur );
		file.read( (char*)colorMap, colorMapSize );
	}

	allocateBuffer( pictureWidth, pictureHeight, 1, type );

	tgaInfo i;

	i.width                = pictureWidth;
	i.height               = pictureHeight;
	i.bytePerPixel         = bytesPerPixel;
	i.colorMapBytePerPixel = colorMapBytePerPixel;
	i.xstep                = (pictureType==1||pictureType==9 ? i.colorMapBytePerPixel : bytesPerPixel);
	i.ystep                = i.xstep*i.width;

	if( attributeByte & 1<<5 )
	{
		i.ptr = (unsigned char*)m_imageBuffer + (i.height-1)*i.ystep;
		i.end = (unsigned char*)m_imageBuffer - i.ystep;
		i.ystep*=-1;
	} else {
		i.ptr = (unsigned char*)m_imageBuffer;
		i.end = (unsigned char*)m_imageBuffer + i.height*i.ystep;
	}

	if( attributeByte & 1<<4 )
		i.xstep*=-1;

	switch( pictureType )
	{
	case COLOR_MAPPED:                loadTgaColorMapped   ( file, i, colorMap ); break;
	case COLOR_MAPPED_RLE:            loadTgaColorMappedRLE( file, i, colorMap ); break;
	case RGB:     case GRAYSCALE:     loadTgaRGB           ( file, i           ); break;
	case RGB_RLE: case GRAYSCALE_RLE: loadTgaRGBRLE        ( file, i           ); break;
	};

	free( colorMap );

	return ELR_SUCESS;
}

#endif

