#ifdef IMAGE_SAVE_PNG



#include "image.h"



/*
   The PNG exporting facilities.

   What should work('-' means implemented but not tested, 'x' means tested and DOES work):
    x exporting EIT_GRAYSCALE8 images
    x exporting EIT_RGB8 images
    x exporting EIT_RGBA8 images
    x exporting EIT_BGR8 images
    x exporting EIT_BGRA8 images
*/


#include "png.h"

#include <iostream>



namespace
{
   void PNGAPI png_error_fun( png_structp png_ptr, png_const_charp msg )
   {
      std::clog<<"Fatal PNG error: "<<msg<<std::endl;

      longjmp( png_ptr->jmpbuf, 1 );
   }

   void PNGAPI png_write_fun( png_structp png_ptr, png_bytep data, png_size_t length )
   {
      std::ostream* out = (std::ostream*)png_ptr->io_ptr;

      out->write( (const char*)data, length );
   }
}

void CImage::m_savePng( std::ostream& stream )
{
   // Set up a png write and info structure
   png_structp png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)png_error_fun, NULL );

   if( !png_ptr )
      return;

   png_infop info_ptr = png_create_info_struct( png_ptr );

   if( !info_ptr )
   {
      png_destroy_write_struct( &png_ptr, NULL );
      return;
   }

   // Set up a jump buffer for returning here in case of an error
   if( setjmp( png_jmpbuf(png_ptr) ) )
   {
      png_destroy_write_struct( &png_ptr, &info_ptr );
      return;
   }

   // Register our write callback
   png_set_write_fn( png_ptr, &stream, png_write_fun, NULL );

   // Collect information needed to write out the image
   int colortype = 0, transform = PNG_TRANSFORM_IDENTITY;
   size_t ystep = m_width;

   switch( m_type )
   {
   case EIT_GRAYSCALE8: colortype = PNG_COLOR_TYPE_GRAY;                                                 break;
   case EIT_RGB8:       colortype = PNG_COLOR_TYPE_RGB;       ystep *= 3;                                break;
   case EIT_RGBA8:      colortype = PNG_COLOR_TYPE_RGB_ALPHA; ystep *= 4;                                break;
   case EIT_BGR8:       colortype = PNG_COLOR_TYPE_RGB;       ystep *= 3; transform = PNG_TRANSFORM_BGR; break;
   case EIT_BGRA8:      colortype = PNG_COLOR_TYPE_RGB_ALPHA; ystep *= 4; transform = PNG_TRANSFORM_BGR; break;
   };

   png_set_IHDR( png_ptr, info_ptr, m_width, m_height, 8, colortype, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );

   // Libpng wants an array of scan line pointers. With the first at the top and the last at the bottom
   unsigned char** rowPtr = new png_bytep[ m_height ];
   unsigned char* data = (unsigned char*)m_imageBuffer;

   for( size_t i=0; i<m_height; ++i )
   {
      rowPtr[ m_height-1-i ] = data;
      data += ystep;
   }

   // Set up a jump buffer for returning here in case of an error
   if( setjmp( png_jmpbuf(png_ptr) ) )
   {
      png_destroy_write_struct( &png_ptr, &info_ptr );
      delete [] rowPtr;
      return;
   }

   // Write the image
   png_set_rows( png_ptr, info_ptr, rowPtr );
   png_write_png( png_ptr, info_ptr, transform, NULL );

   // Clean up
   delete [] rowPtr;

   png_destroy_write_struct( &png_ptr, &info_ptr );
}

#endif

