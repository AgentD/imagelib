#include "image.h"
#include "util.h"
#include "bitmap_font.h"

#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstring>



//-----------------------------------------------------------------------------
//
// Basic functionality of the CImage class. This should always remain the
// same, no metter how many image loaders or color types are added or removed.
//
//-----------------------------------------------------------------------------
CImage::CImage( ) : m_imageBuffer(NULL), m_width(0), m_height(0), m_depth(0), m_type(EIT_NONE) {                        }
CImage::~CImage( )                                                                             { free( m_imageBuffer ); }

void*        CImage::getBuffer   ( ) const { return m_imageBuffer; }
size_t       CImage::getWidth    ( ) const { return m_width;       }
size_t       CImage::getHeight   ( ) const { return m_height;      }
size_t       CImage::getDepth    ( ) const { return m_depth;       }
E_IMAGE_TYPE CImage::getImageType( ) const { return m_type;        }

CImage::E_LOAD_RESULT CImage::load( const std::string& filename, E_IMAGE_FILE type )
{
   std::ifstream file( filename.c_str(), std::ios_base::binary );

   if( !file )
      return ELR_FILE_OPEN_FAILED;

   if( type==EIF_AUTODETECT )
      type = guessType( filename );

   return load( dynamic_cast<std::istream&>( file ), type );
}

void CImage::save( const std::string& filename, E_IMAGE_FILE type )
{
   std::ofstream file( filename.c_str(), std::ios_base::binary );

   if( type==EIF_AUTODETECT )
      type = guessType( filename );

   save( dynamic_cast<std::ostream&>( file ), type );
}

//-----------------------------------------------------------------------------
//
// Theese methods have to be altered if a new color type is implemented.
//
//-----------------------------------------------------------------------------
void CImage::allocateBuffer( size_t width, size_t height, size_t depth, E_IMAGE_TYPE type )
{
   if( m_imageBuffer )
      free( m_imageBuffer );

   size_t byteperpixel = 0;

   switch( type )
   {
   case EIT_GRAYSCALE8:                 byteperpixel = 1; break;
   case EIT_RGB8:       case EIT_BGR8:  byteperpixel = 3; break;
   case EIT_RGBA8:      case EIT_BGRA8: byteperpixel = 4; break;
   case EIT_RGBE8:                      byteperpixel = 4; break;
   };

   m_imageBuffer = malloc( width*height*depth*byteperpixel );
   m_width       = width;
   m_height      = height;
   m_depth       = depth;
   m_type        = type;
}

void CImage::setPixel( size_t x, size_t y, size_t z, unsigned char R, unsigned char G, unsigned char B )
{
   if( !(x<m_width && y<m_height && z<m_depth && m_imageBuffer && m_type!=EIT_NONE) )
      return;

   unsigned char* dst = (unsigned char*)m_imageBuffer;

   switch( m_type )
   {
      case EIT_GRAYSCALE8:
      {
         dst += m_width*m_height*z;
         dst += y*m_width + x;

         *dst = util::luminance<unsigned char>( R, G, B );

         break;
      }
      case EIT_RGB8:
      {
         dst += m_width*m_height*z;
         dst += 3*(y*m_width + x);

         *(dst++) = R;
         *(dst++) = G;
         *dst     = B;

         break;
      }
      case EIT_BGR8:
      {
         dst += m_width*m_height*z;
         dst += 3*(y*m_width + x);

         *(dst++) = B;
         *(dst++) = G;
         *dst     = R;

         break;
      }
      case EIT_RGBA8:
      {
         dst += m_width*m_height*z;
         dst += 4*(y*m_width + x);

         *(dst++) = R;
         *(dst++) = G;
         *dst     = B;

         break;
      }
      case EIT_BGRA8:
      {
         dst += m_width*m_height*z;
         dst += 4*(y*m_width + x);

         *(dst++) = B;
         *(dst++) = G;
         *dst     = R;

         break;
      }
      case EIT_RGBE8:
      {
         dst += m_width*m_height*z;
         dst += 4*(y*m_width + x);

         unsigned char rgbe[4];

         util::rgb2rgbe( rgbe, (float)R/255.0f, (float)G/255.0f, (float)B/255.0f );

         *(dst++) = rgbe[0];
         *(dst++) = rgbe[1];
         *(dst++) = rgbe[2];
         *dst     = rgbe[3];

         break;
      }
   };
}

//-----------------------------------------------------------------------------
//
// Image type and format independand editing methods.
//
//-----------------------------------------------------------------------------
void CImage::printf( size_t x, size_t y, size_t z, unsigned char R, unsigned char G, unsigned char B, const char* formatstring, ... )
{
   // sprintf the format string and arguments into a buffer
   char* buffer = (char*)malloc( 128 );

   va_list arg;
   va_start( arg, formatstring );

   size_t n = vsnprintf( buffer, 128, formatstring, arg );

   if( n>=128 )
   {
      free( buffer );
      buffer = (char*)malloc( n+1 );
      vsnprintf( buffer, n+1, formatstring, arg );
   }

   va_end( arg );

   // Render the characters
   const size_t cw = getCharWidth( );
   const size_t ch = getCharHeight( );

   unsigned char* char_buffer = (unsigned char*)malloc( cw*ch );

   size_t X = x, Y = y-ch;

   for( size_t i=0; buffer[i]!='\0'; ++i )
   {
      if( buffer[i]=='\n' )
      {
         X = x;
         Y -= ch;
         continue;
      }

      memset( char_buffer, 0, cw*ch );

      getCharacter( buffer[i], char_buffer, 0, ch-1, cw, ch );

      for( size_t j=0; j<ch; ++j )
      {
         unsigned char* src = char_buffer + j*cw;

         for( size_t k=0; k<cw; ++k )
         {
            if( src[ k ] )
               setPixel( X + k, Y + j, z, R, G, B );
         }
      }

      X+=cw;
   }

   // Cleanup
   free( char_buffer );
   free( buffer );
}

//---------------------------------------------------------------------------------
//
// If a new image file format is added, the following methods have to be altered.
//
//---------------------------------------------------------------------------------
CImage::E_LOAD_RESULT CImage::load( std::istream& stream, E_IMAGE_FILE type )
{
   // Get the current stream position to restore it on failure
   std::streampos initialPosition = stream.tellg( );

   E_LOAD_RESULT r = ELR_UNKNOWN_FILE_FORMAT;

   switch( type )
   {
   #ifdef IMAGE_LOAD_TGA
      case EIF_TGA: r = m_loadTga( stream ); break;
   #endif

   #ifdef IMAGE_LOAD_BMP
      case EIF_BMP: r = m_loadBmp( stream ); break;
   #endif

   #ifdef IMAGE_LOAD_JPG
      case EIF_JPG: r = m_loadJpg( stream ); break;
   #endif
   
   #ifdef IMAGE_LOAD_PNG
      case EIF_PNG: r = m_loadPng( stream ); break;
   #endif

   #ifdef IMAGE_LOAD_TXT
      case EIF_TXT: r = m_loadTxt( stream ); break;
   #endif
   };

   // In case we failed reading, restore the initial stream state
   if( r != ELR_SUCESS )
   {
      stream.clear( );
      stream.seekg( initialPosition );
   }

   return r;
}

void CImage::save( std::ostream& stream, E_IMAGE_FILE type )
{
   switch( type )
   {
   #ifdef IMAGE_SAVE_TGA
      case EIF_TGA: m_saveTga( stream ); break;
   #endif

   #ifdef IMAGE_SAVE_BMP
      case EIF_BMP: m_saveBmp( stream ); break;
   #endif

   #ifdef IMAGE_SAVE_JPG
      case EIF_JPG: m_saveJpg( stream ); break;
   #endif

   #ifdef IMAGE_SAVE_PNG
      case EIF_PNG: m_savePng( stream ); break;
   #endif

   #ifdef IMAGE_SAVE_TXT
      case EIF_TXT: m_saveTxt( stream ); break;
   #endif
   };
}

E_IMAGE_FILE CImage::guessType( const std::string& filename )
{
   // Get the last 3 characters
   std::string extension = filename.substr( filename.size()-3 );

   // Transform those 3 characters to uppercase
   std::transform( extension.begin(), extension.end(), extension.begin(), toupper );

   if( extension=="TGA" ) return EIF_TGA;
   if( extension=="BMP" ) return EIF_BMP;
   if( extension=="JPG" || extension=="JPEG" ) return EIF_JPG;
   if( extension=="PNG" ) return EIF_PNG;
   if( extension=="TXT" ) return EIF_TXT;

   return EIF_AUTODETECT;
}

