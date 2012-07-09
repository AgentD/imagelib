#ifndef IMAGE_H
#define IMAGE_H



#include <stdio.h>



#ifdef __cplusplus
extern "C"
{
#endif



typedef enum
{
    EIT_NONE = 0,

    EIT_GRAYSCALE8,   ///< Grayscale image, one color channel holding one byte
    EIT_RGB8,         ///< RGB image, three color channels holding one byte each
    EIT_RGBA8         ///< RGBA image, three color channels and one alpha channel, holding one byte each
}
E_IMAGE_TYPE;

typedef enum
{
    EIF_AUTODETECT = 0,  ///< Automatically detect the image file format

    EIF_TGA,             ///< Truevision TARGA image file(*.tga)
    EIF_BMP,             ///< Microsoft Windows bitmap image file(*.bmp)
    EIF_JPG,             ///< JPEG Interchange Format(*.jpg, *.jpeg)
    EIF_PNG,             ///< Portable Network Graphics("PNG's Not Gif") Format(*.png)
    EIF_TXT              ///< Textfile containing ASCII art(*.txt)
}
E_IMAGE_FILE;

typedef enum
{
    ELR_SUCESS = 0,            ///< The image data has been loaded sucessfully
    ELR_UNKNOWN_FILE_FORMAT,   ///< Error: Failed determining the file format or the loader has not been compiled in
    ELR_FILE_OPEN_FAILED,      ///< Error: Failed opening the file
    ELR_NOT_SUPPORTED,         ///< Error: The image file uses a feature that the underlying loading routine has not implemented
    ELR_FILE_CORRUPTED         ///< Error: The image file contains information that makes no sense
}
E_LOAD_RESULT;



typedef struct
{
    void* image_buffer;

    size_t width;
    size_t height;

    E_IMAGE_TYPE type;
}
SImage;



/**
 * \brief Initialise an image structure
 *
 * Call this before doing anything with an image.
 */
void image_init( SImage* img );

/**
 * \brief Uninitialise an image structure
 *
 * Call this once you are done with an image.
 */
void image_deinit( SImage* img );

/**
 * \brief Load an image from a file
 *
 * \param img      The image to load into
 * \param filename The path to the image that has to be loaded
 * \param type     If EIF_AUTODETECT, the type will be determined using the
 *                 last three characters in the filename. If that is no
 *                 possible, loading failes. You can alternatively
 *                 provide the file format if you know it.
 *
 * \return ELR_SUCESS(=0) on sucess, or a loading error otherwise.
 */
E_LOAD_RESULT image_load( SImage* img, const char* filename,
                          E_IMAGE_FILE type );

/**
 * \brief Store the contents of the image buffer to the given file
 *
 * \param img      The image to save
 * \param filename The path to file that the image data should be stored to
 * \param type     What image file format to use for storing the image file.
 *                 If EIF_AUTODETECT, the type will be determined using the
 *                 last three characters in the filename.
 */
void image_save( SImage* img, const char* filename, E_IMAGE_FILE type );

/**
 * \brief Allocate an internal buffer for holding an image
 *
 * \param width  The width of the image to hold
 * \param height The height of the image to hold
 * \param type   The pixel format. This is used to determine the number
 *               of bits per pixel.
 */
void image_allocate_buffer( SImage* img, size_t width, size_t height,
                            E_IMAGE_TYPE type );


/**
 * \brief Guess the image file format from the file ending of a given
 *        string
 *
 * This takes the last three characters from a filename and tries to
 * determine the image file format.
 *
 * \return An image file format enumerator, or EIF_AUTODETECT(=0) on failure
 */
E_IMAGE_FILE image_guess_type( const char* filename );



/**
 * \brief Place a pixel onto the image
 *
 * \param img The image to write to
 * \param x   The x coordinate of the pixel to place(0=left, width-1=right)
 * \param y   The y coordinate of the pixel to place(0=top, height-1=bottom)
 * \param R   The red component of the pixel olor
 * \param G   The green component of the pixel color
 * \param B   The blue component of the pixel color
 */
void image_set_pixel( SImage* img, size_t x, size_t y,
                      unsigned char R, unsigned char G, unsigned char B );

/**
 * \brief Print a text string into the image
 *
 * Print a text onto the image using a built in bitmap font.
 *
 * \param img          The image to write to
 * \param x            The x coordinate of the upper-left corner where the
 *                     text starts
 * \param y            The y coordinate of the upper-left corner where the
 *                     text starts
 * \param R            The red component of the pixel olor
 * \param G            The green component of the pixel color
 * \param B            The blue component of the pixel color
 * \param formatstring A printf format string
 */
void image_print_string( SImage* img, size_t x, size_t y,
                         unsigned char R, unsigned char G, unsigned char B,
                         const char* string );



#ifdef __cplusplus
}
#endif

#endif /* IMAGE_H */

