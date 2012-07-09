#include "image_edit.h"

#include "util.h"
#include "bitmap_font.h"

#include <string.h>



void image_clear( SImage* img, float R, float G, float B, float A )
{
    unsigned char* dst = img->image_buffer;
    unsigned char r, g, b, a;
    size_t x, y;

    if( !dst )
        return;

    r = R * 255.0f;
    g = G * 255.0f;
    b = B * 255.0f;
    a = R * 255.0f;

    switch( img->type )
    {
    case ECT_GRAYSCALE8:
        r = LUMINANCE( R, G, B ) * 255.0f;

        for( y=0; y<img->height; ++y )
            for( x=0; x<img->width; ++x )
                *(dst++) = r;
        break;
    case ECT_RGBA8:
        for( y=0; y<img->height; ++y )
        {
            for( x=0; x<img->width; ++x )
            {
                *(dst++) = r;
                *(dst++) = g;
                *(dst++) = b;
                *(dst++) = a;
            }
        }
        break;
    case ECT_RGB8:
        for( y=0; y<img->height; ++y )
        {
            for( x=0; x<img->width; ++x )
            {
                *(dst++) = r;
                *(dst++) = g;
                *(dst++) = b;
            }
        }
        break;
    }
}

void image_set_pixel( SImage* img, size_t x, size_t y,
                      float R, float G, float B )
{
    unsigned char* dst = img->image_buffer;

    if( x>=img->width || y>=img->height || !dst )
        return;

    switch( img->type )
    {
    case ECT_GRAYSCALE8:
        dst += y*img->width + x;

        *dst = LUMINANCE( R, G, B ) * 255.0f;
        return;
    case ECT_RGB8:
        dst += 3*(y*img->width + x);

        *(dst++) = R * 255.0f;
        *(dst++) = G * 255.0f;
        *dst     = B * 255.0f;
        return;
    case ECT_RGBA8:
        dst += 4*(y*img->width + x);

        *(dst++) = R * 255.0f;
        *(dst++) = G * 255.0f;
        *dst     = B * 255.0f;
        return;
    };
}

void image_print_string( SImage* img, size_t x, size_t y,
                         float R, float G, float B,
                         const char* string )
{
#ifdef IMAGE_BITMAP_FONT
    unsigned char char_buffer[ CHAR_WIDTH*CHAR_HEIGHT ];
    unsigned char* src;

    size_t X = x, Y = y, i, j, k;

    memset( char_buffer, 0, CHAR_WIDTH*CHAR_HEIGHT );

    for( i=0; string[i]!='\0'; ++i )
    {
        if( string[i]=='\n' )
        {
            X  = x;
            Y += CHAR_HEIGHT;
            continue;
        }

        render_character( string[i], char_buffer, 0, 0,
                          CHAR_WIDTH, CHAR_HEIGHT );

        for( j=0; j<CHAR_HEIGHT; ++j )
        {
            src = char_buffer + j*CHAR_WIDTH;

            for( k=0; k<CHAR_WIDTH; ++k )
            {
                if( src[ k ] )
                    image_set_pixel( img, X + k, Y + j, R, G, B );
            }
        }

        X += CHAR_WIDTH;
    }
#endif
}


