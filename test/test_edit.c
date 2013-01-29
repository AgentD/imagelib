#include "image.h"
#include "image_edit.h"



int main( int argc, char** argv )
{
    SImage img;
    size_t x, y;

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

    image_save( &img, "test_edit.png", EIF_AUTODETECT );

    image_deinit( &img );

    return 0;
}

