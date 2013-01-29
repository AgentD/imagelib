#include "image_edit.h"
#include "util.h"



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
    a = A * 255.0f;

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
                      float R, float G, float B, float A )
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
        *(dst++) = B * 255.0f;
        *dst     = A * 255.0f;
        return;
    };
}

