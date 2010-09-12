#include "image.h"

#include <cstdlib>



CImage::CImage( ) : m_imageBuffer(NULL), m_width(0), m_height(0), m_depth(0), m_type(EIT_NONE) {                        }
CImage::~CImage( )                                                                             { free( m_imageBuffer ); }

void*  CImage::getBuffer   ( ) const { return m_imageBuffer; }
size_t CImage::getWidth    ( ) const { return m_width;       }
size_t CImage::getHeight   ( ) const { return m_height;      }
size_t CImage::getDepth    ( ) const { return m_depth;       }
size_t CImage::getImageType( ) const { return m_type;        }

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




CImage::E_LOAD_RESULT CImage::load( const std::string& filename, E_IMAGE_FILE type )
{
	std::ifstream file( filename.c_str(), std::ios_base::binary );

	if( !file )
		return ELR_FILE_OPEN_FAILED;

	if( type==EIF_AUTODETECT )
	{
		std::string extension = filename.substr( filename.size()-3 );

		if( extension=="tga" || extension=="TGA" ) type = EIF_TGA;
		if( extension=="bmp" || extension=="BMP" ) type = EIF_BMP;
	}

	return load( dynamic_cast<std::istream&>( file ), type );
}

void CImage::save( const std::string& filename, E_IMAGE_FILE type )
{
	std::ofstream file( filename.c_str(), std::ios_base::binary );

	if( type==EIF_AUTODETECT )
	{
		std::string extension = filename.substr( filename.size()-3 );

		if( extension=="tga" || extension=="TGA" ) type = EIF_TGA;
		if( extension=="bmp" || extension=="BMP" ) type = EIF_BMP;
	}

	save( dynamic_cast<std::ostream&>( file ), type );
}




CImage::E_LOAD_RESULT CImage::load( std::istream& stream, E_IMAGE_FILE type )
{
	switch( type )
	{
	case EIF_TGA: return m_loadTga( stream );
	case EIF_BMP: return m_loadBmp( stream );
	};

	return ELR_UNKNOWN_FILE_FORMAT;
}

void CImage::save( std::ostream& stream, E_IMAGE_FILE type )
{
	switch( type )
	{
	case EIF_TGA: m_saveTga( stream ); break;
	case EIF_BMP: m_saveBmp( stream ); break;
	};
}

