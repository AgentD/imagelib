#include "image.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#ifdef IMAGE_LOAD_TGA
extern E_LOAD_RESULT load_tga( image_t* img, void* file,
                               const image_io_t* io );
#endif

#ifdef IMAGE_SAVE_TGA
extern void save_tga( const image_t* img, void* file, const image_io_t* io );
#endif


#ifdef IMAGE_LOAD_JPG
extern E_LOAD_RESULT load_jpg( image_t* img, void* file,
                               const image_io_t* io );
#endif

#ifdef IMAGE_SAVE_JPG
extern void save_jpg( const image_t* img, void* file, const image_io_t* io );
#endif


#ifdef IMAGE_LOAD_BMP
extern E_LOAD_RESULT load_bmp( image_t* img, void* file,
                               const image_io_t* io );
#endif

#ifdef IMAGE_SAVE_BMP
extern void save_bmp( const image_t* img, void* file, const image_io_t* io );
#endif


#ifdef IMAGE_LOAD_PNG
extern E_LOAD_RESULT load_png( image_t* img, void* file,
                               const image_io_t* io );
#endif

#ifdef IMAGE_SAVE_PNG
extern void save_png( const image_t* img, void* file, const image_io_t* io );
#endif


#ifdef IMAGE_LOAD_PBM
extern E_LOAD_RESULT load_pbm( image_t* img, void* file,
                               const image_io_t* io );
#endif

#ifdef IMAGE_SAVE_PBM
extern void save_pbm( const image_t* img, void* file, const image_io_t* io );
#endif



void image_init( image_t* img )
{
    memset( img, 0, sizeof(image_t) );
}

void image_deinit( image_t* img )
{
    free( img->image_buffer );
}

int image_allocate_buffer( image_t* img, size_t width, size_t height,
                           E_COLOR_TYPE type )
{
    size_t byteperpixel = 0;

    free( img->image_buffer );

    img->image_buffer = NULL;
    img->width        = 0;
    img->height       = 0;
    img->type         = ECT_NONE;

    if( !width || !height )
        return 0;

    switch( type )
    {
    case ECT_GRAYSCALE8: byteperpixel = 1; break;
    case ECT_RGB8:       byteperpixel = 3; break;
    case ECT_RGBA8:      byteperpixel = 4; break;
    default:
        return 0;
    };

    img->image_buffer = malloc( width*height*byteperpixel );

    if( !img->image_buffer )
        return 0;

    img->width  = width;
    img->height = height;
    img->type   = type;
    return 1;
}

void image_flip_v( image_t* img )
{
    size_t row_length, i, j;
    unsigned char *start_ptr, *end_ptr;
    unsigned char *a, *b;
    unsigned char temp;

    if( !img->width || !img->height || !img->image_buffer )
        return;

    switch( img->type )
    {
    case ECT_GRAYSCALE8: row_length =     img->width; break;
    case ECT_RGB8:       row_length = 3 * img->width; break;
    case ECT_RGBA8:      row_length = 4 * img->width; break;
    default:
        return;
    }

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

void image_flip_h( image_t* img )
{
    unsigned char *r0, *r1, *r2, *r3, temp;
    size_t i, j, row_length;

    if( !img->width || !img->height || !img->image_buffer )
        return;

    switch( img->type )
    {
    case ECT_GRAYSCALE8:
        r0 = img->image_buffer;

        for( i=0; i<img->height; ++i, r0+=img->width )
        {
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

        for( i=0; i<img->height; ++i )
        {
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

        for( i=0; i<img->height; ++i )
        {
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

void image_swap_channels( image_t* img, int c1, int c2 )
{
    unsigned char* ptr;
    unsigned char temp;
    size_t i, j;

    if( !img->width || !img->height || !img->image_buffer || c1==c2 )
        return;

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

E_LOAD_RESULT image_load( image_t* img, const char* filename,
                          E_IMAGE_FILE type )
{
    image_io_t stdio;
    E_LOAD_RESULT r;
    FILE* f;

    image_io_init_stdio( &stdio );

    f = fopen( filename, "rb" );

    if( !f )
       return ELR_FILE_OPEN_FAILED;

    if( type == EIF_AUTODETECT )
       type = image_guess_type( filename );

    r = image_load_custom( img, f, &stdio, type );

    fclose( f );
    return r;
}

E_LOAD_RESULT image_load_custom( image_t* img, void* file,
                                 const image_io_t* io, E_IMAGE_FILE type )
{
    E_LOAD_RESULT r = ELR_UNKNOWN_FILE_FORMAT;

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
    default:
        break;
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


void image_save( const image_t* img, const char* filename, E_IMAGE_FILE type )
{
    image_io_t stdio;
    FILE* f;

    image_io_init_stdio( &stdio );

    f = fopen( filename, "wb" );

    if( !f )
        return;

    if( type==EIF_AUTODETECT )
        type = image_guess_type( filename );

    image_save_custom( img, f, &stdio, type );

    fclose( f );
}

void image_save_custom( const image_t* img, void* file, const image_io_t* io,
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
    default:
        break;
    };
}

void image_set_hint( image_t* img, E_IMAGE_HINT hint, int value )
{
    if( hint < EIH_NUM_HINTS )
        img->hints[ hint ] = value;
}

int image_get_hint( const image_t* img, E_IMAGE_HINT hint )
{
    return hint < EIH_NUM_HINTS ? img->hints[ hint ] : 0;
}


E_IMAGE_FILE image_guess_type( const char* filename )
{
    const char* extension;
    char c[5];

    extension = strrchr( filename, '.' );

    if( !extension )
        return EIF_AUTODETECT;

    strncpy( c, extension+1, 4 );
    c[4] = '\0';

    c[0] = toupper( c[0] );
    c[1] = toupper( c[1] );
    c[2] = toupper( c[2] );
    c[3] = toupper( c[3] );

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

