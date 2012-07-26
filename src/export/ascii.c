#ifdef IMAGE_SAVE_TXT



#include "image.h"
#include "util.h"



/*
   ASCII exporting facilities.

   What should work:
    - exporting ECT_GRAYSCALE8 images
    - exporting ECT_RGB8 images
    - exporting ECT_RGBA8 images
*/



static const char* vt100palette[] =
{
    "\033[22;30m",
    "\033[22;31m",
    "\033[22;32m",
    "\033[22;33m",
    "\033[22;34m",
    "\033[22;35m",
    "\033[22;36m",
    "\033[22;37m",
    "\033[01;30m",
    "\033[01;31m",
    "\033[01;32m",
    "\033[01;33m",
    "\033[01;34m",
    "\033[01;35m",
    "\033[01;36m",
    "\033[01;37m"
};

static const unsigned char vt100paletteRGB[ 3*16 ] =
{
      0,   0,   0,
    205,   0,   0,
      0, 205,   0,
    205, 205,   0,
      0,   0, 238,
    205,   0, 205,
      0, 205, 205,
    229, 229, 229,
    127, 127, 127,
    255,   0,   0,
      0, 255,   0,
    255, 255,   0,
     92,  92, 255,
    255,   0, 255,
      0, 255, 255,
    255, 255, 255
};



static char getCharFromLuminance( float lum )
{
    const char   chars[]  = " .,'~:;!+>=icopjtJY56SB8XDQKHNWM";
    const size_t numchars = 32;

    return chars[ (size_t)( lum*(numchars-1) ) ];
}

size_t getNearestPaletteIndex( const unsigned char* palette, size_t size,
                               unsigned char r, unsigned char g,
                               unsigned char b )
{
    float dist = 255.0f*255.0f*3.0f, dR, dG, dB, curDist;
    size_t best = 0, i;

    if( !palette )
        return 0;

    for( i=0; i<size; ++i, palette+=3 )
    {
        dR = palette[0] - r;
        dG = palette[1] - g;
        dB = palette[2] - b;

        curDist = dR*dR + dG*dG + dB*dB;

        if( curDist<dist )
        {
            best = i;
           
            dist = curDist;
        }
    }

    return best;
}

float average_block( unsigned char* buffer, size_t x, size_t y,
                     size_t w, size_t h, size_t scanline_length,
                     int channels, int R, int G, int B,
                     float* res_r, float* res_g, float* res_b )
{
    unsigned char* src = buffer + (y*scanline_length + x)*channels;
    unsigned char* row;
    size_t X, Y;
    float result = 0.0f;
    *res_r = 0.0f;
    *res_g = 0.0f;
    *res_b = 0.0f;

    for( Y=0; Y<h; ++Y, src+=scanline_length*channels )
    {
        for( row=src, X=0; X<w; ++X, row+=channels )
        {
            result += (float)( LUMINANCE( row[R], row[G], row[B] ) )/255.0f;

            *res_r += (float)(row[R])/255.0f;
            *res_g += (float)(row[G])/255.0f;
            *res_b += (float)(row[B])/255.0f;
        }
    }

    result /= (float)(w*h);
    *res_r /= (float)(w*h);
    *res_g /= (float)(w*h);
    *res_b /= (float)(w*h);

    return result;
}



void save_txt( SImage* img, void* file, const SFileIOInterface* io )
{
    size_t cols = 0, rows, charWidth, charHeight, Y, X, index;
    int color = 0, channels, R, G, B;
    char c;

    /* get the number of columns and rows */
    /*cols = getHint<size_t>( IH_ASCII_EXPORT_COLUMNS );*/
    if( !cols )
        cols = 80;

    rows = cols * 0.5f * ( (float)img->height/(float)img->width );

    /* determine the size of the pixel block covered by one character */
    charWidth  = img->width/cols;
    charHeight = img->height/rows;

    /* determine whether to generate a colored output */
    /*color = getHint<bool>( IH_ASCII_EXPORT_VT100_COLORS );*/

    /* get the required converter settings */
    switch( img->type )
    {
    case ECT_GRAYSCALE8: channels = 1; R = 0; G = 0; B = 0; break;
    case ECT_RGB8:       channels = 3; R = 0; G = 1; B = 2; break;
    case ECT_RGBA8:      channels = 4; R = 0; G = 1; B = 2; break;
    default:
        return;
    };

    /* generate the image */
    if( color )
        io->write( "\033[0m", 1, 4, file );  /* reset attributes to default */

    for( Y=0; Y<rows; ++Y )
    {
        for( X=0; X<cols; ++X )
        {
            float r, g, b, y;

            y = average_block( img->image_buffer, X*charWidth, Y*charHeight,
                               charWidth, charHeight, img->width, channels,
                               R, G, B, &r, &g, &b );

            c = getCharFromLuminance( y );

            if( color )
            {
                index = getNearestPaletteIndex( vt100paletteRGB, 16, r*255.0f,
                                                g*255.0f, b*255.0f );

                io->write( vt100palette[ index ], 1, 8, file );
            }

            io->write( &c, 1, 1, file );
        }

        c = '\n';

        io->write( &c, 1, 1, file );
    }

    if( color )
        io->write( "\033[0m", 1, 4, file );  /* reset attributes to default */
}

#endif

