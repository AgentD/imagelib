#ifndef __IMAGE_H__
#define __IMAGE_H__



#include <string>
#include <fstream>
#include <map>

#include "image_hints.h"



enum E_IMAGE_TYPE
{
   EIT_NONE = 0,

   EIT_GRAYSCALE8,   ///< Grayscale image, one color channel holding one byte
   EIT_RGB8,         ///< RGB image, three color channels holding one byte each
   EIT_RGBA8,        ///< RGBA image, three color channels and one alpha channel, holding one byte each
   EIT_BGR8,         ///< BGR image, three color channels holding one byte each
   EIT_BGRA8,        ///< BGRA image, three color channels and one alpha channel, holding one byte each
   EIT_RGBE8         ///< RGBE image. This is an HDR format. Three color channels and one shared exponent
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



class CImage
{
public:

   CImage( );
   ~CImage( );

   enum E_LOAD_RESULT
   {
      ELR_SUCESS = 0,            ///< The image data has been loaded sucessfully
      ELR_UNKNOWN_FILE_FORMAT,   ///< Error: Failed determining the file format or the loader has not been compiled in
      ELR_FILE_OPEN_FAILED,      ///< Error: Failed opening the file
      ELR_NOT_SUPPORTED,         ///< Error: The image file uses a feature that the underlying loading routine has not implemented
      ELR_FILE_CORRUPTED         ///< Error: The image file contains information that makes no sense
   };

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


   void* getBuffer( ) const;           ///< Get the pointer to the internal image buffer. \sa allocateBuffer
   size_t getWidth( ) const;           ///< Get the width of the internal image buffer
   size_t getHeight( ) const;          ///< Get the height of the internal image buffer
   size_t getDepth( ) const;           ///< Get the depth of the internal image buffer
   E_IMAGE_TYPE getImageType( ) const; ///< Get the color format of the internal image buffer


   /**
    * \copydoc image_hint
    *
    * This method is used to set the value of a hint.
    */
   template<typename T>
   void setHint( image_hint hint, T value )
   {
      union
      {
         T      tv;
         size_t sv;
      } u;

      u.tv = value;

      m_hints[ hint ] = u.sv;
   }

   /**
    * \copydoc image_hint
    *
    * This method is used to get the value of a hint.
    */
   template<typename T>
   T getHint( image_hint hint )
   {
      union
      {
         T      tv;
         size_t sv;
      } u;

      u.sv = m_hints[ hint ];

      return u.tv;
   }




   /**
    * \brief Guess the image file format from the file ending of a given string
    *
    * This takes the last three characters from a filename and tries to determine the
    * image file format.
    *
    * \return An image file format enumerator, or EIF_AUTODETECT(=0) on failure
    */
   static E_IMAGE_FILE guessType( const std::string& filename );




private:

   CImage( const CImage& );
   void operator= ( const CImage& );

   void* m_imageBuffer;

   size_t m_width, m_height, m_depth;

   E_IMAGE_TYPE m_type;

   std::map< image_hint, size_t > m_hints;

   #ifdef IMAGE_LOAD_TGA
      E_LOAD_RESULT m_loadTga( std::istream& file );
   #endif

   #ifdef IMAGE_SAVE_TGA
      void m_saveTga( std::ostream& stream );
   #endif


   #ifdef IMAGE_LOAD_JPG
      E_LOAD_RESULT m_loadJpg( std::istream& file );
   #endif

   #ifdef IMAGE_SAVE_JPG
      void m_saveJpg( std::ostream& file );
   #endif


   #ifdef IMAGE_LOAD_BMP
      E_LOAD_RESULT m_loadBmp( std::istream& file );
   #endif

   #ifdef IMAGE_SAVE_BMP
      void m_saveBmp( std::ostream& stream );
   #endif


   #ifdef IMAGE_LOAD_PNG
      E_LOAD_RESULT m_loadPng( std::istream& stream );
   #endif

   #ifdef IMAGE_SAVE_PNG
      void m_savePng( std::ostream& stream );
   #endif


   #ifdef IMAGE_LOAD_TXT
      E_LOAD_RESULT m_loadTxt( std::istream& stream );
   #endif

   #ifdef IMAGE_SAVE_TXT
      void m_saveTxt( std::ostream& stream );
   #endif
};



#endif /* __IMAGE_H__ */

