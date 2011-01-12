#ifdef IMAGE_LOAD_TXT



#include "image.h"
#include "bitmap_font.h"
#include "util.h"



#include <vector>
#include <string>



CImage::E_LOAD_RESULT CImage::m_loadTxt( std::istream& stream )
{
   ///////////////////////////// Read in the Text /////////////////////////////
   std::vector< std::string > text; // Holds the text lines

   text.push_back( "" ); // Add the first line

   // Read all lines
   while( !stream.eof( ) )
   {
      char c = (char)stream.get( );         // Get one character

      if( c=='\r' )                         // If the character is CR
      {
         char c2 = (char)stream.get( );         // Get another character

         if( c2=='\n' )                         // Check if the other is LF
         {
            text.push_back( "" );                   // A windows new line!
         }
         else                                   // Not a windows new line?
         {
            text.back( ) += c;                      // Add both
            text.back( ) += c2;
         }

         continue;                              // Go on
      }

      if( c=='\n' )                         // If the character is LF
      {
         text.push_back( "" );                  // It's a Unix new line!
         continue;                              // Go on
      }

      text.back( ) += c;                    // nothing special, add it
   }


   //////////////////////// Allocate the image buffer /////////////////////////
   size_t width = 0, height = text.size( );
   size_t charWidth = getCharWidth( ), charHeight = getCharHeight( );

   for( size_t i=0; i<text.size( ); ++i )
   {
      size_t n = text[ i ].size( );

      width = n>width ? n : width;
   }

   width  *= charWidth;
   height *= charHeight;

   allocateBuffer( width, height, 1, EIT_GRAYSCALE8 );

   //////////////////////////// Generate the image ////////////////////////////

   // For each text line
   for( size_t i=0; i<text.size( ); ++i )
   {
      size_t y = (m_height-1) - charHeight*i;    // The y pixel position
      size_t x = 0;                              // The x pixel position

      // For each character
      for( size_t j=0; j<text[i].size( ); ++j )
      {
         // Render the character
         getCharacter( text[i][j], (unsigned char*)m_imageBuffer, x, y, m_width, m_height );

         // Advance to the next position
         x += charWidth;
      }
   }

   return ELR_SUCESS;
}



#endif

