#ifndef IMAGE_H
#define IMAGE_H



#include <string>
#include <fstream>



enum E_IMAGE_TYPE
{
    EIT_NONE = 0,

    EIT_GRAYSCALE8,   ///< Grayscale image, one color channel holding one byte
    EIT_RGB8,         ///< RGB image, three color channels holding one byte each
    EIT_RGBA8         ///< RGBA image, three color channels and one alpha channel, holding one byte each
};

enum E_IMAGE_FILE
{
    EIF_AUTODETECT = 0,  ///< Automatically detect the image file format

    EIF_TGA,             ///< Truevision TARGA image file(*.tga)
    EIF_BMP,             ///< Microsoft Windows bitmap image file(*.bmp)
    EIF_JPG,             ///< JPEG Interchange Format(*.jpg, *.jpeg)
    EIF_PNG,             ///< Portable Network Graphics("PNG's Not Gif") Format(*.png)
    EIF_TXT              ///< Textfile containing ASCII art(*.txt)
};

enum E_LOAD_RESULT
{
    ELR_SUCESS = 0,            ///< The image data has been loaded sucessfully
    ELR_UNKNOWN_FILE_FORMAT,   ///< Error: Failed determining the file format or the loader has not been compiled in
    ELR_FILE_OPEN_FAILED,      ///< Error: Failed opening the file
    ELR_NOT_SUPPORTED,         ///< Error: The image file uses a feature that the underlying loading routine has not implemented
    ELR_FILE_CORRUPTED         ///< Error: The image file contains information that makes no sense
};



class CImage
{
public:

    CImage( );
    ~CImage( );

    /**
     * \brief Load an image from a file
     *
     * \param filename The path to the image that has to be loaded
     * \param type     If EIF_AUTODETECT, the type will be determined using the last three characters
     *                 in the filename. If that is no possible, loading failes. You can alternatively
     *                 provide the file format if you know it.
     *
     * \return ELR_SUCESS(=0) on sucess, or a loading error otherwise.
     */
    E_LOAD_RESULT load( const std::string& filename, E_IMAGE_FILE type=EIF_AUTODETECT );

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
     * \brief Allocate an internal buffer for holding an image
     *
     * \param width  The width of the image to hold
     * \param height The height of the image to hold
     * \param type   The pixel format. This is used to determine the number of bits per pixel.
     */
    void allocateBuffer( size_t width, size_t height, E_IMAGE_TYPE type );


    void* getBuffer( ) const;           ///< Get the pointer to the internal image buffer. \sa allocateBuffer
    size_t getWidth( ) const;           ///< Get the width of the internal image buffer
    size_t getHeight( ) const;          ///< Get the height of the internal image buffer
    E_IMAGE_TYPE getImageType( ) const; ///< Get the color format of the internal image buffer



    /**
     * \brief Guess the image file format from the file ending of a given
     *        string
     *
     * This takes the last three characters from a filename and tries to
     * determine the image file format.
     *
     * \return An image file format enumerator, or EIF_AUTODETECT(=0) on failure
     */
    static E_IMAGE_FILE guessType( const std::string& filename );



    /**
     * \brief Place a pixel onto the image
     *
     * \param x The x coordinate of the pixel to place(0=left, width-1=right)
     * \param y The y coordinate of the pixel to place(0=bottom, height-1=top)
     * \param R The red component of the pixel olor
     * \param G The green component of the pixel color
     * \param B The blue component of the pixel color
     */
    void setPixel( size_t x, size_t y, unsigned char R, unsigned char G, unsigned char B );

    /**
     * \brief Print a text string into the image
     *
     * Print a text onto the image using a built in bitmap font.
     *
     * \param x            The x coordinate of the upper-left corner where the text starts
     * \param y            The y coordinate of the upper-left corner where the text starts
     * \param R            The red component of the pixel olor
     * \param G            The green component of the pixel color
     * \param B            The blue component of the pixel color
     * \param formatstring A printf format string
     */
    void print_string( size_t x, size_t y, unsigned char R, unsigned char G, unsigned char B, const char* string );



private:

    CImage( const CImage& );
    void operator= ( const CImage& );

    void* m_imageBuffer;

    size_t m_width, m_height;

    E_IMAGE_TYPE m_type;

#ifdef IMAGE_LOAD_TGA
    E_LOAD_RESULT m_loadTga( FILE* file );
#endif

#ifdef IMAGE_SAVE_TGA
    void m_saveTga( FILE* file );
#endif


#ifdef IMAGE_LOAD_JPG
    E_LOAD_RESULT m_loadJpg( FILE* file );
#endif

#ifdef IMAGE_SAVE_JPG
    void m_saveJpg( FILE* file );
#endif


#ifdef IMAGE_LOAD_BMP
    E_LOAD_RESULT m_loadBmp( FILE* file );
#endif

#ifdef IMAGE_SAVE_BMP
    void m_saveBmp( FILE* file );
#endif


#ifdef IMAGE_LOAD_PNG
    E_LOAD_RESULT m_loadPng( FILE* file );
#endif

#ifdef IMAGE_SAVE_PNG
    void m_savePng( FILE* file );
#endif


#ifdef IMAGE_LOAD_TXT
    E_LOAD_RESULT m_loadTxt( FILE* file );
#endif

#ifdef IMAGE_SAVE_TXT
    void m_saveTxt( FILE* file );
#endif
};



#endif /* __IMAGE_H__ */

