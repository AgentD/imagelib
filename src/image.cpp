#include "image.h"
#include "util.h"
#include "bitmap_font.h"

#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstring>



//----------------------------------------------------------------------------
//
// Basic functionality of the CImage class. This should always remain the
// same, no metter how many image loaders or color types are added or removed.
//
//----------------------------------------------------------------------------
CImage::CImage( ) : m_imageBuffer(NULL), m_width(0), m_height(0),
                    m_type(EIT_NONE)
{
}

CImage::~CImage( )
{
    free( m_imageBuffer );
}

void*        CImage::getBuffer   ( ) const { return m_imageBuffer; }
size_t       CImage::getWidth    ( ) const { return m_width;       }
size_t       CImage::getHeight   ( ) const { return m_height;      }
E_IMAGE_TYPE CImage::getImageType( ) const { return m_type;        }

//----------------------------------------------------------------------------
//
// Theese methods have to be altered if a new color type is implemented.
//
//----------------------------------------------------------------------------
void CImage::allocateBuffer( size_t width, size_t height, E_IMAGE_TYPE type )
{
    if( m_imageBuffer )
       free( m_imageBuffer );

    size_t byteperpixel = 0;

    switch( type )
    {
    case EIT_GRAYSCALE8: byteperpixel = 1; break;
    case EIT_RGB8:       byteperpixel = 3; break;
    case EIT_RGBA8:      byteperpixel = 4; break;
    };

    m_imageBuffer = malloc( width*height*byteperpixel );
    m_width       = width;
    m_height      = height;
    m_type        = type;
}

void CImage::setPixel( size_t x, size_t y, 
                       unsigned char R, unsigned char G, unsigned char B )
{
    if( x>=m_width || y>=m_height || !m_imageBuffer || m_type==EIT_NONE )
        return;

    unsigned char* dst = (unsigned char*)m_imageBuffer;

    switch( m_type )
    {
    case EIT_GRAYSCALE8:
        dst += y*m_width + x;

        *dst = LUMINANCE( R, G, B );
        break;
    case EIT_RGB8:
        dst += 3*(y*m_width + x);

        *(dst++) = R;
        *(dst++) = G;
        *dst     = B;
        break;
    case EIT_RGBA8:
        dst += 4*(y*m_width + x);

        *(dst++) = R;
        *(dst++) = G;
        *dst     = B;
        break;
    };
}

//----------------------------------------------------------------------------
//
// Image type and format independand editing methods.
//
//----------------------------------------------------------------------------
void CImage::print_string( size_t x, size_t y,
                           unsigned char R, unsigned char G, unsigned char B,
                           const char* string )
{
#ifdef IMAGE_BITMAP_FONT
    const size_t cw = getCharWidth( );
    const size_t ch = getCharHeight( );

    unsigned char* char_buffer = (unsigned char*)malloc( cw*ch );

    size_t X = x, Y = y;

    for( size_t i=0; string[i]!='\0'; ++i )
    {
        if( string[i]=='\n' )
        {
            X  = x;
            Y += ch;
            continue;
        }

        memset( char_buffer, 0, cw*ch );

        getCharacter( string[i], char_buffer, 0, 0, cw, ch );

        for( size_t j=0; j<ch; ++j )
        {
            unsigned char* src = char_buffer + j*cw;

            for( size_t k=0; k<cw; ++k )
            {
                if( src[ k ] )
                    setPixel( X + k, Y + j, R, G, B );
            }
        }

        X += cw;
    }

    // Cleanup
    free( char_buffer );
#endif
}

/****************************************************************************
 *                                                                          *
 * If a new image file format is added, the following methods have to be    *
 * altered.                                                                 *
 *                                                                          *
 ****************************************************************************/
E_LOAD_RESULT CImage::load( const std::string& filename, E_IMAGE_FILE type )
{
    FILE* f;
    E_LOAD_RESULT r = ELR_UNKNOWN_FILE_FORMAT;

    f = fopen( filename.c_str( ), "rb" );

    if( !f )
       return ELR_FILE_OPEN_FAILED;

    if( type==EIF_AUTODETECT )
       type = guessType( filename );

    switch( type )
    {
#ifdef IMAGE_LOAD_TGA
    case EIF_TGA: r = m_loadTga( f ); break;
#endif

#ifdef IMAGE_LOAD_BMP
    case EIF_BMP: r = m_loadBmp( f ); break;
#endif

#ifdef IMAGE_LOAD_JPG
    case EIF_JPG: r = m_loadJpg( f ); break;
#endif
   
#ifdef IMAGE_LOAD_PNG
    case EIF_PNG: r = m_loadPng( f ); break;
#endif

#ifdef IMAGE_LOAD_TXT
    case EIF_TXT: r = m_loadTxt( f ); break;
#endif
    };

    fclose( f );

    return r;
}

void CImage::save( const std::string& filename, E_IMAGE_FILE type )
{
    FILE* f;

    f = fopen( filename.c_str(), "wb" );

    if( !f )
        return;

    if( type==EIF_AUTODETECT )
       type = guessType( filename );

    switch( type )
    {
#ifdef IMAGE_SAVE_TGA
    case EIF_TGA: m_saveTga( f ); break;
#endif

#ifdef IMAGE_SAVE_BMP
    case EIF_BMP: m_saveBmp( f ); break;
#endif

#ifdef IMAGE_SAVE_JPG
    case EIF_JPG: m_saveJpg( f ); break;
#endif

#ifdef IMAGE_SAVE_PNG
    case EIF_PNG: m_savePng( f ); break;
#endif

#ifdef IMAGE_SAVE_TXT
    case EIF_TXT: m_saveTxt( f ); break;
#endif
    };

    fclose( f );
}

E_IMAGE_FILE CImage::guessType( const std::string& filename )
{
    // Get the last filename extension in uppercase
    std::string extension = filename.substr( filename.rfind( '.' )+1 );

    std::transform( extension.begin(), extension.end(), extension.begin(),
                    toupper );

    if( extension=="TGA" ) return EIF_TGA;
    if( extension=="BMP" ) return EIF_BMP;
    if( extension=="JPG" || extension=="JPEG" ) return EIF_JPG;
    if( extension=="PNG" ) return EIF_PNG;
    if( extension=="TXT" ) return EIF_TXT;

    return EIF_AUTODETECT;
}

