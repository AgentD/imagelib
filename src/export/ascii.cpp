#ifdef IMAGE_SAVE_TXT



#include "image.h"
#include "util.h"

#include <vector>
#include <string>
#include <iostream>



/*
   Bitmap exporting facilities.

   What should work('-' means implemented but not tested, 'x' means tested and DOES work, 'o' means not implemented yet)
    x exporting EIT_GRAYSCALE8 images
    x exporting EIT_RGB8 images
    x exporting EIT_RGBA8 images
    x exporting EIT_BGR8 images
    x exporting EIT_BGRA8 images
*/



namespace
{
   const char* vt100palette[] =
   {
      "\033[22;30m",
      "\033[22;31m",
      "\033[22;32m",
      "\033[22;33m",
      "\033[22;34m",
      "\033[22;35m",
      "\033[22;36m",
      "\033[22;37m",
      "\033[01;30m",
      "\033[01;31m",
      "\033[01;32m",
      "\033[01;33m",
      "\033[01;34m",
      "\033[01;35m",
      "\033[01;36m",
      "\033[01;37m"
   };

   const unsigned char vt100paletteRGB[ 3*16 ] =
   {
        0,   0,   0,
      205,   0,   0,
        0, 205,   0,
      205, 205,   0,
        0,   0, 238,
      205,   0, 205,
        0, 205, 205,
      229, 229, 229,
      127, 127, 127,
      255,   0,   0,
        0, 255,   0,
      255, 255,   0,
       92,  92, 255,
      255,   0, 255,
        0, 255, 255,
      255, 255, 255
   };




   char getCharFromLuminance( float lum )
   {
      const char   chars[]  = " .,'~:;!+>=icopjtJY56SB8XDQKHNWM";
      const size_t numchars = 32;

      return chars[ (size_t)( lum*(numchars-1) ) ];
   }




   template<typename T, size_t channels, size_t maxChannelValue, size_t R, size_t G, size_t B>
   void toAscii( T* buffer, size_t x, size_t y, size_t width, size_t height, size_t charW, size_t charH, std::string& result, bool col )
   {
      T* src = buffer + (y*width + x)*channels;

      float lum = 0.0f;

      for( size_t Y=0; Y<charH; ++Y, src += width*channels )
         for( size_t X=0; X<charW*channels; X+=channels )
            lum += (float)util::luminance( src[ X+R ], src[ X+G ], src[ X+B ] );

      lum /= ( (float)charW*(float)charH*(float)maxChannelValue );



      if( col )
      {
         // Determine average color of block
         float r=0.0f, g=0.0f, b=0.0f;

         src -= width*channels*charH;

         for( size_t Y=0; Y<charH; ++Y, src += width*channels )
            for( size_t X=0; X<charW*channels; X+=channels )
            {
               r += (float)src[ X + R ];
               g += (float)src[ X + G ];
               b += (float)src[ X + B ];
            }

         r /= (float)charW*(float)charH*(float)maxChannelValue;
         g /= (float)charW*(float)charH*(float)maxChannelValue;
         b /= (float)charW*(float)charH*(float)maxChannelValue;

         // Get the nerest VT100 color index
         size_t i = util::getNearestPaletteIndex<unsigned char, 255>( vt100paletteRGB, 16, 255*r, 255*g, 255*b );

         // Write out the escape code
         result = vt100palette[ i ];
      }
      else
      {
         result = "";
      }

      // Covert the luminance to the character
      result += getCharFromLuminance( lum );
   }
}




void CImage::m_saveTxt( std::ostream& stream )
{
   // Get the number of columns and rows
   size_t cols = getHint<size_t>( IH_ASCII_EXPORT_COLUMNS );

   if( !cols ) cols = 80;

   size_t rows = cols * 0.5f * ( (float)m_height/(float)m_width );

   // Determine the size of the pixel block covered by one character
   size_t charWidth = m_width/cols, charHeight = m_height/rows;

   // Determine whether to generate a colored output
   bool color = getHint<bool>( IH_ASCII_EXPORT_VT100_COLORS );

   // Get the required converter
   void(* convert )( unsigned char*, size_t, size_t, size_t, size_t, size_t, size_t, std::string&, bool ) = NULL;

   switch( m_type )
   {
   case EIT_GRAYSCALE8: convert = toAscii<unsigned char, 1, 255, 0, 0, 0>; break;
   case EIT_RGB8:       convert = toAscii<unsigned char, 3, 255, 0, 1, 2>; break;
   case EIT_RGBA8:      convert = toAscii<unsigned char, 4, 255, 0, 1, 2>; break;
   case EIT_BGR8:       convert = toAscii<unsigned char, 3, 255, 2, 1, 0>; break;
   case EIT_BGRA8:      convert = toAscii<unsigned char, 4, 255, 2, 1, 0>; break;
   default:
      return;
   };

   // Generate the image
   if( color )
      stream<<"\033[0m"<<std::endl;  // Reset attributes to default

   for( size_t y=rows-1; y!=0; --y )
   {
      std::string scanline, result;

      for( size_t x=0; x<cols; ++x )
      {
         convert( (unsigned char*)m_imageBuffer, x*charWidth, y*charHeight, m_width, m_height, charWidth, charHeight, result, color );

         scanline += result;
      }

      stream<<scanline<<std::endl;
   }

   if( color )
      stream<<"\033[0m"<<std::endl;  // Reset attributes to default
}



#endif

