#include "image.h"
#include "util.h"

#include <cstring>



#ifdef IMAGE_LOAD_BMP

/*
	Bitmap loading facilities.

	What should work('-' means implemented but not tested, 'x' means tested and DOES work, 'o' means not implemented yet)
	  x Importing 8 bit per pixel color mapped images with RLE compression.
	  x Importing 8 bit per pixel color mapped images.
	  x Importing 16 bit per pixel images
	  x Importing 24 bit per pixel images
	  x Importing 32 bit per pixel images
	  x Importing generic bitfield encoded images
	  x Flipping images upside-down while loading if needed to move the origin to the lower left corner
*/



namespace
{
	enum
	{
		BI_RGB       = 0,
		BI_RLE8      = 1,
		BI_RLE4      = 2,
		BI_BITFIELDS = 3
	};

	void loadBMPbitfields( std::istream& src, size_t w, size_t h, void* ptr, size_t bpp, size_t R, size_t G, size_t B, bool flip )
	{
		ssize_t ystep = flip ? -3*w : 3*w;		// Add this to the image pointer to skip to the next row
		size_t bytePerPixel = bpp/8;
		size_t padding = (w*bytePerPixel) % 4;	// The number of bytes per row is always a multiple of four, if not, a padding is added

		unsigned char *cur = (unsigned char*)ptr, *end = (unsigned char*)ptr + h*3*w;

		if( flip )
		{
			end  = cur - 3*w;
			cur += (h-1)*3*w;
		}

		// We will read multibyte integers from the file and AND them with the color masks, to get the color values.
		// The ANDed values have to be shifted to the right to get the final color values. Here we work out how many bits we have to shift.
		size_t rshift=0, gshift=0, bshift=0;

		for( ; !((R>>rshift) & 1); ++rshift );
		for( ; !((G>>gshift) & 1); ++gshift );
		for( ; !((B>>bshift) & 1); ++bshift );


		size_t scaleR = 0xFF / (R>>rshift);
		size_t scaleG = 0xFF / (G>>gshift);
		size_t scaleB = 0xFF / (B>>bshift);


		for( ; cur!=end; cur+=ystep )	// Iterate over all pixel rows
		{
			unsigned char* row = cur;

			for( size_t x=0; x<w; ++x, row+=3 )	// Iterate over all pixels in the row
			{
				unsigned char v0[4] = {0,0,0,0};

				src.read( (char*)v0, bytePerPixel );	// Read the color value from the file

				size_t v = ((size_t)v0[0]) | ((size_t)v0[1])<<8 | ((size_t)v0[2])<<16 | ((size_t)v0[3])<<24;

				row[2] = (unsigned char)( (v & R)>>rshift )*scaleR;	// And it with the R mask and shift it right to get the red value
				row[1] = (unsigned char)( (v & G)>>gshift )*scaleG;	// And it with the G mask and shift it right to get the green value
				row[0] = (unsigned char)( (v & B)>>bshift )*scaleB;	// And it with the B mask and shift it right to get the blue value
			}

			src.seekg( padding, std::ios_base::cur );	// Skip the zeroes added for padding
		}
	}

	void loadBMPcolormap( std::istream& src, size_t w, size_t h, void* ptr, unsigned char* colorMap, bool flip )
	{
		ssize_t ystep = flip ? -3*w : 3*w;		// Add this to the image pointer to skip to the next row

		unsigned char *cur = (unsigned char*)ptr, *end = (unsigned char*)ptr + h*3*w;

		if( flip )
		{
			end  = cur - 3*w;
			cur += (h-1)*3*w;
		}

		size_t padding = w % 4;			// The number of bytes per row is always a multiple of four, if not, a padding is added

		for( ; cur!=end; cur+=ystep )	// Iterate over all pixel rows
		{
			unsigned char* row = cur;

			for( size_t x=0; x<w; ++x, row+=3 )	// Iterate over all pixels in the row
			{
				size_t i = src.get( );

				row[0] = colorMap[ i*4     ];
				row[1] = colorMap[ i*4 + 1 ];
				row[2] = colorMap[ i*4 + 2 ];
			}

			src.seekg( padding, std::ios_base::cur );	// Skip the zeroes added for padding
		}
	}

	void loadBMPtruecolor( std::istream& src, size_t w, size_t h, void* ptr, bool flip )
	{
		ssize_t ystep = flip ? -3*w : 3*w;		// Add this to the image pointer to skip to the next row
		size_t padding = w*3 % 4;				// The number of bytes per row is always a multiple of four, if not, a padding is added

		unsigned char *cur = (unsigned char*)ptr, *end = (unsigned char*)ptr + h*3*w;

		if( flip )
		{
			end  = cur - 3*w;
			cur += (h-1)*3*w;
		}

		for( ; cur!=end; cur+=ystep )	// Iterate over all pixel rows
		{
			src.read( (char*)cur, 3*w );				// Read the current pixel row into the image buffer
			src.seekg( padding, std::ios_base::cur );	// Skip the zeroes added for padding
		}
	}

	void loadBMPrle( std::istream& src, size_t w, size_t h, void* ptr, unsigned char* colorMap )
	{
		unsigned char* beg = (unsigned char*)ptr;
		unsigned char* cur = beg;
		unsigned char* end = beg + 3*w*h;

		while( cur!=end && !src.eof() )
		{
			unsigned char v[2];

			src.read( (char*)v, 2 );

			if( v[0] )
			{
				size_t i = 4*((size_t)v[1]);

				unsigned char B=colorMap[i], G=colorMap[i+1], R=colorMap[i+2];

				for( size_t j=0; j<v[0] && cur!=end; ++j, cur+=3 )
				{
					cur[0] = B;
					cur[1] = G;
					cur[2] = R;
				}
			}
			else
			{
				if( v[1] == 2 )
				{
					cur += 3*((size_t)src.get( ));
					cur += 3*w*((size_t)src.get( ));
				}
				else if( v[1]>2 )
				{
					for( size_t i=0; i<v[1] && cur!=end; ++i, cur+=3 )
					{
						size_t index = 4*((size_t)src.get( ));

						cur[0] = colorMap[index  ];
						cur[1] = colorMap[index+1];
						cur[2] = colorMap[index+2];
					}

					size_t padding = (v[1] % 2);	// TODO: Find an explanation why exactly this and nothing else works

					src.seekg( padding, std::ios_base::cur );
				}
			}
		}
	}
}



CImage::E_LOAD_RESULT CImage::m_loadBmp( std::istream& file )
{
	unsigned char header[ 54 ];

	size_t initialPosition = file.tellg( );

	file.read( (char*)header, 54 );


	//////////////////////// Extract all >interesting< information from the file header ////////////////////////
	size_t  bfOffBits     = ((size_t)header[10]) | ((size_t)header[11])<<8 | ((size_t)header[12])<<16 | ((size_t)header[13])<<24;
	size_t  biWidth       = ((size_t)header[18]) | ((size_t)header[19])<<8 | ((size_t)header[20])<<16 | ((size_t)header[21])<<24;
	ssize_t biHeight      = ((size_t)header[22]) | ((size_t)header[23])<<8 | ((size_t)header[24])<<16 | ((size_t)header[25])<<24;
	size_t  biBitCount    = ((size_t)header[28]) | ((size_t)header[29])<<8;
	size_t  biCompression = ((size_t)header[30]) | ((size_t)header[31])<<8 | ((size_t)header[32])<<16 | ((size_t)header[33])<<24;
	size_t  biClrUsed     = ((size_t)header[46]) | ((size_t)header[47])<<8 | ((size_t)header[48])<<16 | ((size_t)header[49])<<24;

	bool flipImage = (biHeight<0);

	biHeight = flipImage ? -biHeight : biHeight;
	biClrUsed = biClrUsed ? biClrUsed : 256;


	///////////////////////////////////// Check the header data for sanity /////////////////////////////////////
	if( header[0]!='B' || header[1]!='M' || biCompression>3                                         ) return ELR_FILE_CORRUPTED;
	if( biBitCount==1 || biBitCount==2 || biBitCount==4 || biCompression==BI_RLE4 || header[14]!=40 ) return ELR_NOT_SUPPORTED;
	if( biBitCount!=8 && biBitCount!=16 && biBitCount!=24 && biBitCount!=32                         ) return ELR_FILE_CORRUPTED;
	if( biCompression==BI_RLE8      && (biBitCount!=8 || flipImage)                                 ) return ELR_FILE_CORRUPTED;
	if( biCompression==BI_BITFIELDS && biBitCount!=16 && biBitCount!=32                             ) return ELR_FILE_CORRUPTED;


	////////////////////////////////////////////// Read color mask /////////////////////////////////////////////
	size_t maskR=0, maskG=0, maskB=0;

	if( biCompression==BI_BITFIELDS )
	{
		unsigned char colorMask[12];
		file.read( (char*)colorMask, 12 );

		maskR = ((size_t)colorMask[0]) | ((size_t)colorMask[1])<<8 | ((size_t)colorMask[ 2])<<16 | ((size_t)colorMask[ 3])<<24;
		maskG = ((size_t)colorMask[4]) | ((size_t)colorMask[5])<<8 | ((size_t)colorMask[ 6])<<16 | ((size_t)colorMask[ 7])<<24;
		maskB = ((size_t)colorMask[8]) | ((size_t)colorMask[9])<<8 | ((size_t)colorMask[10])<<16 | ((size_t)colorMask[11])<<24;

		size_t x = (maskR|maskG|maskB);		// ORing the color masks must result in a continous block of set bits

		if( !x || !maskR || !maskG || !maskB )
			return ELR_FILE_CORRUPTED;

		while( !(x & 1) )			// Shift the block to the right until the first bit is set.
			x>>=1;					// After that, x+1 MUST be some power of two if the block is continous.
									// If x+1 is a power of two, ((x+1) & x) must be 0

		// Check wether the block in x is not continous OR the bit masks overlap
		if( (x & (x+1)) || (maskR & maskG & maskB) )
			return ELR_FILE_CORRUPTED;
	}


	//////////////////////////////////////////// Read the color map ////////////////////////////////////////////
	unsigned char colorMap[ 4*256 ];

	if( biBitCount==8 )
		file.read( (char*)colorMap, 4*biClrUsed );


	////////////////////////////////////////// Create the image buffer /////////////////////////////////////////
	allocateBuffer( biWidth, biHeight, 1, EIT_BGR8 );


	//////////////////////////////////////////// Read the image data ///////////////////////////////////////////
	file.seekg( initialPosition + bfOffBits, std::ios_base::beg );

	switch( biCompression )
	{
	case BI_BITFIELDS:
		loadBMPbitfields( file, biWidth, biHeight, m_imageBuffer, biBitCount, maskR, maskG, maskB, flipImage );
		break;
	case BI_RGB:
		if( biBitCount==8 )
			loadBMPcolormap( file, biWidth, biHeight, m_imageBuffer, colorMap, flipImage );
		else if( biBitCount==16 )
			loadBMPbitfields( file, biWidth, biHeight, m_imageBuffer, biBitCount, 0x00007C00, 0x000003E0, 0x0000001F, flipImage );
		else if( biBitCount==24 )
			loadBMPtruecolor( file, biWidth, biHeight, m_imageBuffer, flipImage );
		else
			loadBMPbitfields( file, biWidth, biHeight, m_imageBuffer, biBitCount, 0x00FF0000, 0x0000FF00, 0x000000FF, flipImage );
		break;
	case BI_RLE8:
		loadBMPrle( file, biWidth, biHeight, m_imageBuffer, colorMap );
		break;
	};

	return ELR_SUCESS;
}

#endif

