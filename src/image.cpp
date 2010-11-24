#include "image.h"

#include <cstdlib>
#include <algorithm>
#include <cctype>



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
// This method has to be altered if a new color type is implemented.
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

//-------------------------------------------------------------------------------
//
// If a new image file format is added, the following two methods and functions
// have to be altered.
//
//-------------------------------------------------------------------------------
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
   };
}

E_IMAGE_FILE guessType( const std::string& filename )
{
   // Get the last 3 characters
   std::string extension = filename.substr( filename.size()-3 );

   // Transform those 3 characters to uppercase
   std::transform( extension.begin(), extension.end(), extension.begin(), toupper );

   if( extension=="TGA" ) return EIF_TGA;
   if( extension=="BMP" ) return EIF_BMP;
   if( extension=="JPG" || extension=="JPEG" ) return EIF_JPG;
   if( extension=="PNG" ) return EIF_PNG;

   return EIF_AUTODETECT;
}

