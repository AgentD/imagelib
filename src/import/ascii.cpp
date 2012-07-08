#ifdef IMAGE_LOAD_TXT

#include "image.h"
#include "bitmap_font.h"
#include "util.h"

#include <vector>
#include <string>

CImage::E_LOAD_RESULT CImage::m_loadTxt( std::istream& stream )
{
    /////////////////////////// Read in the Text ///////////////////////////
    std::vector< std::string > text; // holds the text lines

    text.push_back( "" ); // add the first line

    // Read all lines
    while( !stream.eof( ) )
    {
        char c = (char)stream.get( );         // get one character

        if( c=='\r' )                         // Windows newline
        {
            char c2 = (char)stream.get( );         // get another character

            text.push_back( "" );

            if( c2!='\n' )                         // add it if it wasn't LF
                text.back( ) += c2;

            continue;
        }

        if( c=='\n' )                         // Unix newline
        {
            text.push_back( "" );
            continue;
        }

        text.back( ) += c;
    }


    ////////////////////// Allocate the image buffer ///////////////////////
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

    ////////////////////////// Generate the image //////////////////////////

    // For each text line
    for( size_t i=0; i<text.size( ); ++i )
    {
        size_t y = charHeight*i;    // The y pixel position
        size_t x = 0;               // The x pixel position

        // For each character
        for( size_t j=0; j<text[i].size( ); ++j )
        {
            // Render the character
            getCharacter( text[i][j], (unsigned char*)m_imageBuffer,
                          x, y, m_width, m_height );

            // Advance to the next position
            x += charWidth;
        }
    }

    return ELR_SUCESS;
}



#endif

