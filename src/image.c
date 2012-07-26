#include "image.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>




#ifdef IMAGE_LOAD_TGA
extern E_LOAD_RESULT load_tga( SImage* img, void* file,
                               const SFileIOInterface* io );
#endif

#ifdef IMAGE_SAVE_TGA
extern void save_tga( SImage* img, void* file, const SFileIOInterface* io );
#endif


#ifdef IMAGE_LOAD_JPG
extern E_LOAD_RESULT load_jpg( SImage* img, void* file,
                               const SFileIOInterface* io );
#endif

#ifdef IMAGE_SAVE_JPG
extern void save_jpg( SImage* img, void* file, const SFileIOInterface* io );
#endif


#ifdef IMAGE_LOAD_BMP
extern E_LOAD_RESULT load_bmp( SImage* img, void* file,
                               const SFileIOInterface* io );
#endif

#ifdef IMAGE_SAVE_BMP
extern void save_bmp( SImage* img, void* file, const SFileIOInterface* io );
#endif


#ifdef IMAGE_LOAD_PNG
extern E_LOAD_RESULT load_png( SImage* img, void* file,
                               const SFileIOInterface* io );
#endif

#ifdef IMAGE_SAVE_PNG
extern void save_png( SImage* img, void* file, const SFileIOInterface* io );
#endif


#ifdef IMAGE_LOAD_TXT
extern E_LOAD_RESULT load_txt( SImage* img, void* file,
                               const SFileIOInterface* io );
#endif

#ifdef IMAGE_SAVE_TXT
extern void save_txt( SImage* img, void* file, const SFileIOInterface* io );
#endif



void image_init( SImage* img )
{
    img->image_buffer = NULL;
    img->width        = 0;
    img->height       = 0;
    img->type         = ECT_NONE;
}

void image_deinit( SImage* img )
{
    free( img->image_buffer );
}

void image_allocate_buffer( SImage* img, size_t width, size_t height,
                            E_COLOR_TYPE type )
{
    size_t byteperpixel = 0;

    /* free any existing buffer and reset attributes concerning the buffer */
    if( img->image_buffer )
        free( img->image_buffer );

    img->image_buffer = NULL;
    img->width        = 0;
    img->height       = 0;
    img->type         = ECT_NONE;

    /* sanity check */
    if( !width || !height )
        return;

    /* determine number of bytes per pixel */
    switch( type )
    {
    case ECT_GRAYSCALE8: byteperpixel = 1; break;
    case ECT_RGB8:       byteperpixel = 3; break;
    case ECT_RGBA8:      byteperpixel = 4; break;
    default:
        return;
    };

    /* allocate the buffer */
    img->image_buffer = malloc( width*height*byteperpixel );

    if( !img->image_buffer )
        return;

    /* on success, set the attributers concerning the buffer */
    img->width  = width;
    img->height = height;
    img->type   = type;
}

E_LOAD_RESULT image_load( SImage* img, const char* filename,
                          E_IMAGE_FILE type )
{
    SFileIOInterface stdio;
    E_LOAD_RESULT r;
    FILE* f;

    image_io_init_stdio( &stdio );

    /* try to open the file */
    f = fopen( filename, "rb" );

    if( !f )
       return ELR_FILE_OPEN_FAILED;

    /* try to determine the format from the extension if required */
    if( type == EIF_AUTODETECT )
       type = image_guess_type( filename );

    r = image_load_custom( img, f, &stdio, type );

    /* cleanup and return */
    fclose( f );

    return r;
}

E_LOAD_RESULT image_load_custom( SImage* img, void* file,
                                 const SFileIOInterface* io,
                                 E_IMAGE_FILE type )
{
    E_LOAD_RESULT r = ELR_UNKNOWN_FILE_FORMAT;

    /* call the coresponding loader routine */
    switch( type )
    {
#ifdef IMAGE_LOAD_TGA
    case EIF_TGA: r = load_tga( img, file, io ); break;
#endif

#ifdef IMAGE_LOAD_BMP
    case EIF_BMP: r = load_bmp( img, file, io ); break;
#endif

#ifdef IMAGE_LOAD_JPG
    case EIF_JPG: r = load_jpg( img, file, io ); break;
#endif
   
#ifdef IMAGE_LOAD_PNG
    case EIF_PNG: r = load_png( img, file, io ); break;
#endif

#ifdef IMAGE_LOAD_TXT
    case EIF_TXT: r = load_txt( img, file, io ); break;
#endif
    };

    if( r!=ELR_SUCESS )
    {
        free( img->image_buffer );

        img->image_buffer = NULL;
        img->width        = 0;
        img->height       = 0;
        img->type         = ECT_NONE;
    }

    return r;
}


void image_save( SImage* img, const char* filename, E_IMAGE_FILE type )
{
    SFileIOInterface stdio;
    FILE* f;

    image_io_init_stdio( &stdio );

    /* try to open the file */
    f = fopen( filename, "wb" );

    if( !f )
        return;

    /* try to determine the format from the extension if required */
    if( type==EIF_AUTODETECT )
        type = image_guess_type( filename );

    /* exporter the image */
    image_save_custom( img, f, &stdio, type );

    /* cleanup and return */
    fclose( f );
}

void image_save_custom( SImage* img, void* file, const SFileIOInterface* io,
                        E_IMAGE_FILE type )
{
    switch( type )
    {
#ifdef IMAGE_SAVE_TGA
    case EIF_TGA: save_tga( img, file, io ); break;
#endif

#ifdef IMAGE_SAVE_BMP
    case EIF_BMP: save_bmp( img, file, io ); break;
#endif

#ifdef IMAGE_SAVE_JPG
    case EIF_JPG: save_jpg( img, file, io ); break;
#endif

#ifdef IMAGE_SAVE_PNG
    case EIF_PNG: save_png( img, file, io ); break;
#endif

#ifdef IMAGE_SAVE_TXT
    case EIF_TXT: save_txt( img, file, io ); break;
#endif
    };
}


E_IMAGE_FILE image_guess_type( const char* filename )
{
    char c[5];

    /* Get the filename extension in uppercase */
    const char* extension = strrchr( filename, '.' );

    if( !extension )
        return EIF_AUTODETECT;

    strncpy( c, extension+1, 4 );
    c[4] = '\0';

    c[0] = toupper( c[0] );
    c[1] = toupper( c[1] );
    c[2] = toupper( c[2] );
    c[3] = toupper( c[3] );

    /* determine the type from the extension */
    if( !strcmp( c, "TGA" ) )
        return EIF_TGA;

    if( !strcmp( c, "BMP" ) )
        return EIF_BMP;

    if( !strcmp( c, "PNG" ) )
        return EIF_PNG;

    if( !strcmp( c, "TXT" ) )
        return EIF_TXT;

    if( !strcmp( c, "JPG" ) || !strcmp( c, "JPEG" ) )
        return EIF_JPG;

    return EIF_AUTODETECT;
}

