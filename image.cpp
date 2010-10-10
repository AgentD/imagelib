#include "image.h"

#include <cstdlib>
#include <algorithm>
#include <cctype>



//-----------------------------------------------------------------------------
//
// Basic functionality of the CImage class. This should always remain the
// same, no metter how many image loaders are added or removed.
//
//-----------------------------------------------------------------------------
CImage::CImage( ) : m_imageBuffer(NULL), m_width(0), m_height(0), m_depth(0), m_type(EIT_NONE) {                        }
CImage::~CImage( )                                                                             { free( m_imageBuffer ); }

void*        CImage::getBuffer   ( ) const { return m_imageBuffer; }
size_t       CImage::getWidth    ( ) const { return m_width;       }
size_t       CImage::getHeight   ( ) const { return m_height;      }
size_t       CImage::getDepth    ( ) const { return m_depth;       }
E_IMAGE_TYPE CImage::getImageType( ) const { return m_type;        }

//-----------------------------------------------------------------------------
//
// Image buffer allocation method. This method has to be altered if a new
// image data type is implemented.
//
//-----------------------------------------------------------------------------
void CImage::allocateBuffer( size_t width, size_t height, size_t depth, E_IMAGE_TYPE type )
{
	if( m_imageBuffer )
		free( m_imageBuffer );

	size_t byteperpixel;

	switch( type )
	{
	case EIT_GRAYSCALE8:                 byteperpixel = 1; break;
	case EIT_RGB8:       case EIT_BGR8:  byteperpixel = 3; break;
	case EIT_RGBA8:      case EIT_BGRA8: byteperpixel = 4; break;
	};

	m_imageBuffer = malloc( width*height*depth*byteperpixel );
	m_width       = width;
	m_height      = height;
	m_depth       = depth;
	m_type        = type;
}

//-----------------------------------------------------------------------------
//
// Image format helper methods. The following methods can be used to:
//  - determine whether a loader for a given file format has been compiled in
//  - guess the image file format from the file extension
//  - get a string describing a given image format
//
// When adding a new loader, these methods have to be altered.
//
//-----------------------------------------------------------------------------
bool CImage::isSupported( E_IMAGE_FILE filetype )
{
	switch( filetype )
	{
	case EIF_AUTODETECT: return true;
	
	#ifdef IMAGE_COMPILE_TGA
		case EIF_TGA: return true;
	#endif

	#ifdef IMAGE_COMPILE_BMP
		case EIF_BMP: return true;
	#endif
	};

	return false;
}

E_IMAGE_FILE CImage::guessType( const std::string& filename )
{
	// Get the last 3 characters
	std::string extension = filename.substr( filename.size()-3 );

	// Transform those 3 characters to uppercase
	std::transform( extension.begin(), extension.end(), extension.begin(), toupper );

	if( extension=="TGA" ) return EIF_TGA;
	if( extension=="BMP" ) return EIF_BMP;

	return EIF_AUTODETECT;
}

std::string CImage::getTypeName( E_IMAGE_FILE filetype )
{
	switch( filetype )
	{
	case EIF_TGA: return "Truevision TARGA image file(*.tga)";
	case EIF_BMP: return "Microsoft Windows bitmap file(*.bmp)";
	};

	return "unknown image file format";
}



//-----------------------------------------------------------------------------
//
// The following methods are responsible for loading/saving images from/to
// files. They do not have to be altered even if a new image file format loader
// is implemented, because they only redirect the input file to the generic
// loading methods and use the guessType( ) method to touch the list of image
// file format types.
//
//-----------------------------------------------------------------------------
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
// The following methods are resposible for loading/saving image data encoded
// in the form of image file formats from/to stream sources.
// If a new image file format is added, the following two methods have to be
// altered.
//
//-----------------------------------------------------------------------------
CImage::E_LOAD_RESULT CImage::load( std::istream& stream, E_IMAGE_FILE type )
{
	// Get the current stream position to restore it on failure
	std::streampos initialPosition = stream.tellg( );

	E_LOAD_RESULT r = ELR_UNKNOWN_FILE_FORMAT;

	switch( type )
	{
	#ifdef IMAGE_COMPILE_TGA
		case EIF_TGA: r = m_loadTga( stream ); break;
	#endif

	#ifdef IMAGE_COMPILE_BMP
		case EIF_BMP: r = m_loadBmp( stream ); break;
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
	#ifdef IMAGE_COMPILE_TGA
		case EIF_TGA: m_saveTga( stream ); break;
	#endif

	#ifdef IMAGE_COMPILE_BMP
		case EIF_BMP: m_saveBmp( stream ); break;
	#endif
	};
}

