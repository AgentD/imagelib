#include "image.h"



int main( )
{
    SImage img;

    image_init( &img );

    image_load( &img, "samples/lenna.txt", EIF_AUTODETECT );
    image_save( &img, "lenna.txt.png", EIF_AUTODETECT );

    image_load( &img, "samples/lenna.jpg", EIF_AUTODETECT );
    image_save( &img, "lenna.jpg.png", EIF_AUTODETECT );

    image_load( &img, "samples/lenna.png", EIF_AUTODETECT );
    image_save( &img, "lenna.png.png", EIF_AUTODETECT );

    image_load( &img, "samples/lenna32.bmp", EIF_AUTODETECT );
    image_save( &img, "lenna32.bmp.png", EIF_AUTODETECT );

    image_load( &img, "samples/lenna24.bmp", EIF_AUTODETECT );
    image_save( &img, "lenna24.bmp.png", EIF_AUTODETECT );

    image_deinit( &img );

    return 0;
}

