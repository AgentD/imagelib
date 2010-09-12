#ifndef __IMAGE_H__
#define __IMAGE_H__



#include <string>
#include <fstream>



enum E_IMAGE_TYPE
{
	EIT_NONE = 0,

	EIT_GRAYSCALE8,		///< Grayscale image, one color channel hodling one byte
	EIT_RGB8,			///< RGB image, three color channels holding one byte each
	EIT_RGBA8,			///< RGBA image, three color channels and one alpha channel, holding one byte each
	EIT_BGR8,			///< BGR image, three color channels holding one byte each
	EIT_BGRA8			///< BGRA image, three color channels and one alpha channel, holding one byte each
};



enum E_IMAGE_FILE
{
	EIF_AUTODETECT = 0,

	EIF_TGA,
	EIF_BMP
};



class CImage
{
public:

	CImage( );
	~CImage( );

	enum E_LOAD_RESULT
	{
		ELR_SUCESS = 0,
		ELR_UNKNOWN_FILE_FORMAT,
		ELR_FILE_OPEN_FAILED,
		ELR_NOT_SUPPORTED,
		ELR_FILE_CORRUPTED
	};

	E_LOAD_RESULT load( const std::string& filename, E_IMAGE_FILE type=EIF_AUTODETECT );
	E_LOAD_RESULT load( std::istream& stream, E_IMAGE_FILE type );

	void save( const std::string& filename, E_IMAGE_FILE type=EIF_AUTODETECT );
	void save( std::ostream& stream, E_IMAGE_FILE type );

	void allocateBuffer( size_t width, size_t height, size_t depth, E_IMAGE_TYPE type );

	void* getBuffer( ) const;

	size_t getWidth( ) const;
	size_t getHeight( ) const;
	size_t getDepth( ) const;
	size_t getImageType( ) const;

private:

	void* m_imageBuffer;

	size_t m_width, m_height, m_depth;

	E_IMAGE_TYPE m_type;


	E_LOAD_RESULT m_loadTga( std::istream& file );
	E_LOAD_RESULT m_loadBmp( std::istream& file );

	void m_saveBmp( std::ostream& stream );
	void m_saveTga( std::ostream& stream );
};



#endif /* __IMAGE_H__ */

