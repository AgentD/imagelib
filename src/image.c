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


#ifdef IMAGE_LOAD_PBM
extern E_LOAD_RESULT load_pbm( SImage* img, void* file,
                               const SFileIOInterface* io );
#endif

#ifdef IMAGE_SAVE_PBM
extern void save_pbm( SImage* img, void* file, const SFileIOInterface* io );
#endif




void image_init( SImage* img )
{
    memset( img, 0, sizeof(SImage) );
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

void image_flip_v( SImage* img )
{
    size_t row_length, i, j;
    unsigned char *start_ptr, *end_ptr;
    unsigned char *a, *b;
    unsigned char temp;

    /* sanity check */
    if( !img || !img->width || !img->height || !img->image_buffer )
        return;

    /* determine bytes per scanline */
    switch( img->type )
    {
    case ECT_GRAYSCALE8: row_length =     img->width; break;
    case ECT_RGB8:       row_length = 3 * img->width; break;
    case ECT_RGBA8:      row_length = 4 * img->width; break;
    default:
        return;
    }

    /* flip the image */
    start_ptr = img->image_buffer;
    end_ptr   = start_ptr + (img->height-1)*row_length;

    for( i=0; i<img->height/2; ++i )
    {
        a = start_ptr;
        b = end_ptr;
        
        for( j=0; j<row_length; ++j, ++a, ++b )
        {
            temp = *a;
            *a = *b;
            *b = temp;
        }

        start_ptr += row_length;
        end_ptr -= row_length;
    }
}

void image_flip_h( SImage* img )
{
    unsigned char *r0, *r1, *r2, *r3, temp;
    size_t i, j, row_length;

    /* sanity check */
    if( !img || !img->width || !img->height || !img->image_buffer )
        return;

    /* determine bytes per pixel */
    switch( img->type )
    {
    case ECT_GRAYSCALE8:
        r0 = img->image_buffer;

        /* for each scanline */
        for( i=0; i<img->height; ++i, r0+=img->width )
        {
            /* reverse the scanline */
            for( j=0; j<img->width/2; ++j )
            {
                temp = r0[ img->width-1-j ];
                r0[ img->width-1-j ] = r0[ j ];
                r0[ j ] = temp;
            }
        }
        break;
    case ECT_RGB8:
        r0 = img->image_buffer;
        r1 = r0+1;
        r2 = r1+1;
        row_length = 3*img->width;

        /* for each scanline */
        for( i=0; i<img->height; ++i )
        {
            /* reverse the scanline */
            for( j=0; j<row_length/2; j+=3 )
            {
                temp = r0[ row_length-3-j ];
                r0[ row_length-3-j ] = r0[ j ];
                r0[ j ] = temp;

                temp = r1[ row_length-3-j ];
                r1[ row_length-3-j ] = r1[ j ];
                r1[ j ] = temp;

                temp = r2[ row_length-3-j ];
                r2[ row_length-3-j ] = r2[ j ];
                r2[ j ] = temp;
            }

            r0 += row_length;
            r1 += row_length;
            r2 += row_length;
        }
        break;
    case ECT_RGBA8:
        r0 = img->image_buffer;
        r1 = r0+1;
        r2 = r1+1;
        r3 = r2+1;
        row_length = 4*img->width;

        /* for each scanline */
        for( i=0; i<img->height; ++i )
        {
            /* reverse the scanline */
            for( j=0; j<row_length/2; j+=4 )
            {
                temp = r0[ row_length-4-j ];
                r0[ row_length-4-j ] = r0[ j ];
                r0[ j ] = temp;

                temp = r1[ row_length-4-j ];
                r1[ row_length-4-j ] = r1[ j ];
                r1[ j ] = temp;

                temp = r2[ row_length-4-j ];
                r2[ row_length-4-j ] = r2[ j ];
                r2[ j ] = temp;
            }

            r0 += row_length;
            r1 += row_length;
            r2 += row_length;
            r3 += row_length;
        }
        break;
    default:
        break;
    }
}

void image_swap_channels( SImage* img, int c1, int c2 )
{
    unsigned char* ptr;
    unsigned char temp;
    size_t i, j;

    /* sanity check */
    if( !img || !img->width || !img->height || !img->image_buffer )
        return;

    if( c1==c2 )
        return;

    /* */
    switch( img->type )
    {
    case ECT_GRAYSCALE8:
        break;
    case ECT_RGB8:
        if( c1>2 || c2>2 )
            break;

        ptr = img->image_buffer;

        for( j=0; j<img->height; ++j )
        {
            for( i=0; i<img->width; ++i, ptr+=3 )
            {
                temp = ptr[ c1 ];
                ptr[ c1 ] = ptr[ c2 ];
                ptr[ c2 ] = temp;
            }
        }
        break;
    case ECT_RGBA8:
        if( c1>3 || c2>3 )
            break;

        ptr = img->image_buffer;

        for( j=0; j<img->height; ++j )
        {
            for( i=0; i<img->width; ++i, ptr+=4 )
            {
                temp = ptr[ c1 ];
                ptr[ c1 ] = ptr[ c2 ];
                ptr[ c2 ] = temp;
            }
        }
        break;
    default:
        break;
    }
}

/****************************************************************************/

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

#ifdef IMAGE_LOAD_PBM
    case EIF_PBM: r = load_pbm( img, file, io ); break;
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

#ifdef IMAGE_SAVE_PBM
    case EIF_PBM: save_pbm( img, file, io ); break;
#endif
    };
}

void image_set_hint( SImage* img, E_IMAGE_HINT hint, size_t value )
{
    if( img && hint<EIH_NUM_HINTS )
        img->hints[ hint ] = value;
}

size_t image_get_hint( SImage* img, E_IMAGE_HINT hint )
{
    if( img && hint<EIH_NUM_HINTS )
        return img->hints[ hint ];

    return 0;
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

    if( !strcmp( c, "JPG" ) || !strcmp( c, "JPEG" ) )
        return EIF_JPG;

    if( !strcmp( c, "PPM" ) || !strcmp( c, "PGM" ) || !strcmp( c, "PBM" ) ||
        !strcmp( c, "PNM" ) )
        return EIF_PBM;

    return EIF_AUTODETECT;
}

