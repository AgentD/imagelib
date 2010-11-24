#ifdef IMAGE_LOAD_PNG



#include "image.h"



/*
   The PNG loading facilities.

   What should work('-' means implemented but not tested, 'x' means tested and DOES work):
     x Importing PNG images using libpng and storing them as RGB8, RGBA8 or GRAYSCALE8 images
     x Flipping images upside down while loading to keep the origin in the lower left corner
*/


#include "png.h"

#include <iostream>



namespace
{
   // custom callback for error handling
   void PNGAPI png_error_fun( png_structp png_ptr, png_const_charp msg )
   {
      std::clog<<"Fatal PNG error: "<<msg<<std::endl;

      longjmp( png_ptr->jmpbuf, 1 );
   }

   // custom callback for reading data from the file
   void PNGAPI png_read_fun( png_structp png_ptr, png_bytep data, png_size_t length )
   {
      std::istream* in = (std::istream*)png_ptr->io_ptr;

      in->read( (char*)data, length );
   }
}


CImage::E_LOAD_RESULT CImage::m_loadPng( std::istream& stream )
{
   // Declare all variables we need
   int bitDepth, colorType;
   png_uint_32 width, height;
   unsigned char** rowPtr = NULL;   // Holds pointers to the individual scanlines

   // Read and check the file signature
   png_byte buffer[8];

   stream.read( (char*)buffer, 8 );

   if( png_sig_cmp( buffer, 0, 8 ) )
      return ELR_FILE_CORRUPTED;

   // Set up structures for reading the image and information from the file.
   png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)png_error_fun, NULL );

   if( !png_ptr )
      return ELR_FILE_OPEN_FAILED;

   png_infop info_ptr = png_create_info_struct( png_ptr );

   if( !info_ptr )
   {
		png_destroy_read_struct( &png_ptr, NULL, NULL );
      return ELR_FILE_OPEN_FAILED;
   }

   // Set up a jump buffer for returning here in case of an error
   if( setjmp( png_jmpbuf(png_ptr) ) )
   {
      png_destroy_read_struct( &png_ptr, &info_ptr, NULL );

      delete [] rowPtr;

      return ELR_FILE_CORRUPTED;
   }

   // Register our custom read function and read the information from the file header
   png_set_read_fn( png_ptr, &stream, png_read_fun );
   png_set_sig_bytes( png_ptr, 8 );
   png_read_info( png_ptr, info_ptr );
   png_get_IHDR( png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, NULL, NULL, NULL );

   // We want true color images. If the image uses a color map, it has to be converted.
   if( colorType==PNG_COLOR_TYPE_PALETTE )
      png_set_palette_to_rgb( png_ptr );

   // We want at least 8 bit per color channel. If the image uses a lower bit count, it has to be converted.
	if( bitDepth<8 )
	{
      if( colorType==PNG_COLOR_TYPE_GRAY || colorType==PNG_COLOR_TYPE_GRAY_ALPHA )
         png_set_gray_1_2_4_to_8( png_ptr );
      else
         png_set_packing( png_ptr );
   }

   if( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) )
      png_set_tRNS_to_alpha( png_ptr );

   // We want a maximum of 8 bit per color channel. If the image has 16, it has to be converted.
   if( bitDepth == 16 )
      png_set_strip_16( png_ptr );

   // Convert gray scale with alpha to true color
   if( colorType==PNG_COLOR_TYPE_GRAY_ALPHA )
      png_set_gray_to_rgb( png_ptr );

   png_read_update_info( png_ptr, info_ptr );
   png_get_IHDR( png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, NULL, NULL, NULL );

	// Allocate the image buffer
	size_t ystep = width;

   if( colorType==PNG_COLOR_TYPE_GRAY )
   {
      allocateBuffer( width, height, 1, EIT_GRAYSCALE8 );
   }
   else if( colorType==PNG_COLOR_TYPE_RGB_ALPHA )
   {
      ystep *= 4;
      allocateBuffer( width, height, 1, EIT_RGBA8 );
   }
   else
   {
      ystep *= 3;
      allocateBuffer( width, height, 1, EIT_RGB8 );
   }

   // Libpng wants an array of scan line pointers. With the first at the top and the last at the bottom
   rowPtr = new png_bytep[ height ];

   unsigned char* ptr = (unsigned char*)m_imageBuffer;

   for( size_t i=0; i<height; ++i )
   {
      rowPtr[ height-1-i ] = ptr;
      ptr += ystep;
   }

   // Set up a jump buffer for returning here in case of an error
   if( setjmp( png_jmpbuf(png_ptr) ) )
   {
      png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
      delete [] rowPtr;
      return ELR_FILE_CORRUPTED;
   }

   // Read the image from the file
   png_read_image( png_ptr, rowPtr );
   png_read_end( png_ptr, NULL );

   // Clean up
   png_destroy_read_struct( &png_ptr, &info_ptr, 0 );
   delete [] rowPtr;

   return ELR_SUCESS;
}

#endif

