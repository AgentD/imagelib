#include "image.h"
#include "image_edit.h"



int main( int argc, char** argv )
{
    SImage img;
    size_t x, y, i;
    char c[33];

    image_init( &img );

    image_allocate_buffer( &img, 800, 600, ECT_RGBA8 );

    /* clear the image to semi transparent white */
    image_clear( &img, 1.0f, 1.0f, 1.0f, 0.5f );

    /* draw a multicoloured box */
    for( y=0; y<255; ++y )
    {
        for( x=0; x<255; ++x )
        {
            image_set_pixel( &img, 100+x, 100+y, ((float)x)/255.0f,
                                                 ((float)y)/255.0f,
                                                 1.0f - ((float)x)/255.0f,
                                                 1.0f );
        }
    }

    /* print a test string with line break */
    image_print_string( &img, 120, 300, 0.0f, 0.0f, 1.0f, 0.5f,
                        "Editing\nTest" );

    /* print character table */
    for( i=0, y=0; y<16; ++y )
    {
        for( x=0; x<32; ++x, ++i )
        {
            c[x  ] = (i=='\n') ? ' ' : i;
            c[++x] = ' ';
        }

        c[32] = '\0';

        image_print_string( &img, 400, 100+20*y, 0.0f, 0.0f, 1.0f, 1.0f, c );
    }

    image_save( &img, "test_edit.png", EIF_AUTODETECT );

    image_deinit( &img );

    return 0;
}

