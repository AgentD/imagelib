#ifndef __IMAGE_H__
#define __IMAGE_H__



#include <string>
#include <fstream>




#define IMAGE_COMPILE_TGA
#define IMAGE_COMPILE_BMP




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
	EIF_AUTODETECT = 0,	///< Automatically detect the image file format

	EIF_TGA,			///< Truevision TARGA image file(*.tga)
	EIF_BMP				///< Microsoft Windows bitmap image file(*.bmp)
};



class CImage
{
public:

	CImage( );
	~CImage( );

	enum E_LOAD_RESULT
	{
		/// The image data has been loaded sucessfully
		ELR_SUCESS = 0,

		/// Error: Failed determining the file format or the loader has not been compiled in
		ELR_UNKNOWN_FILE_FORMAT,

		/// Error: Failed opening the file
		ELR_FILE_OPEN_FAILED,

		/// Error: The image file uses a feature that the underlying loading routine has not implemented
		ELR_NOT_SUPPORTED,

		/// Error: The image file contains information that makes no sense
		ELR_FILE_CORRUPTED
	};

	/**
	 * \brief Load an image from a file
	 *
	 * \param filename The path to the image that has to be loaded
	 * \param type     If EIF_AUTODETECT, the type will be determined using the last three characters
	 *                 in the filename. If that is no possible, loading failes. You can alternatively
	 *                 provide the file format if you know it.
	 * \return ELR_SUCESS(=0) on sucess, or a loading error otherwise.
	 */
	E_LOAD_RESULT load( const std::string& filename, E_IMAGE_FILE type=EIF_AUTODETECT );
	
	/**
	 * \brief Load an image from a stream
	 *
	 * \param stream A stream containing the image data encoded using an image file format.
	 * \param type   The image file format used for storing the image.
	 *
	 * If this method fails, the stream will be unaltered.
	 *
	 * \return ELR_SUCESS(=0) on sucess, or a loading error otherwise.
	 */
	E_LOAD_RESULT load( std::istream& stream, E_IMAGE_FILE type );

	/**
	 * \brief Store the contents of the image buffer to the given file
	 *
	 * \param filename The path to file that the image data should be stored to
	 * \param type     What image file format to use for storing the image file.
	 *                 If EIF_AUTODETECT, the type will be determined using the last three
	 *                 characters in the filename.
	 */
	void save( const std::string& filename, E_IMAGE_FILE type=EIF_AUTODETECT );

	/**
	 * \brief Write the contents of the image buffer to the given stream
	 *
	 * \param stream The stream to write the image data to.
	 * \param type   The image file format to use for writing the image data.
	 */
	void save( std::ostream& stream, E_IMAGE_FILE type );

	/**
	 * \brief Allocate an internal buffer for holding an image
	 *
	 * \param width  The width of the image to hold
	 * \param height The height of the image to hold
	 * \param depth  The depth of the image to hold. Note: This is not the number of bits
	 *               per pixel or something similar. This is the number of layers for e.g.
	 *               multilayer images or 3D images.
	 * \param type   The pixel format. This is used to determine the number of bits per pixel.
	 */
	void allocateBuffer( size_t width, size_t height, size_t depth, E_IMAGE_TYPE type );

	/// Get the pointer to the internal image buffer. \sa allocateBuffer
	void* getBuffer( ) const;


	/**
	 * \brief Returns true if the given image file format can be loaded
	 *
	 * Loaders for different image file formats can be optionally compiled in or not.
	 * With this method, you can check whether a specific loader has been compiled in.
	 *
	 * \param filetype The image file format to check for
	 *
	 * \return True if the loader for the given file format is compiled in
	 */
	bool isSupported( E_IMAGE_FILE filetype );

	/**
	 * \brief Guess the image file format from the file ending of a given string
	 *
	 * This takes the last three characters from a filename and tries to determine the
	 * image file format.
	 *
	 * \return An image file format enumerator, or EIF_AUTODETECT(=0) on failure
	 */
	E_IMAGE_FILE guessType( const std::string& filename );

	/// Get a string describing the given image file format.
	std::string getTypeName( E_IMAGE_FILE filetype );


	size_t getWidth( ) const;			///< Get the width of the internal image buffer
	size_t getHeight( ) const;			///< Get the height of the internal image buffer
	size_t getDepth( ) const;			///< Get the depth of the internal image buffer
	E_IMAGE_TYPE getImageType( ) const;	///< Get the color format of the internal image buffer

private:

	void* m_imageBuffer;

	size_t m_width, m_height, m_depth;

	E_IMAGE_TYPE m_type;


	#ifdef IMAGE_COMPILE_TGA
		E_LOAD_RESULT m_loadTga( std::istream& file );
		void m_saveTga( std::ostream& stream );
	#endif

	#ifdef IMAGE_COMPILE_BMP
		E_LOAD_RESULT m_loadBmp( std::istream& file );
		void m_saveBmp( std::ostream& stream );
	#endif
};



#endif /* __IMAGE_H__ */

