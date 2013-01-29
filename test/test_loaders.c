#include "image.h"



int main( )
{
    SImage img;

    image_init( &img );

    image_load( &img, "samples/feep.pbm", EIF_AUTODETECT );
    image_save( &img, "feep.pbm.png", EIF_AUTODETECT );

    image_load( &img, "samples/feep_gray.pbm", EIF_AUTODETECT );
    image_save( &img, "feep_gray.pbm.png", EIF_AUTODETECT );

    image_load( &img, "samples/rgb.pbm", EIF_AUTODETECT );
    image_save( &img, "rgb.pbm.png", EIF_AUTODETECT );

    image_load( &img, "samples/feep_grey_bin.pbm", EIF_AUTODETECT );
    image_save( &img, "feep_grey_bin.pbm.png", EIF_AUTODETECT );

    image_load( &img, "samples/rgb_bin.pbm", EIF_AUTODETECT );
    image_save( &img, "rgb_bin.pbm.png", EIF_AUTODETECT );

    image_load( &img, "samples/lenna.jpg", EIF_AUTODETECT );
    image_save( &img, "lenna.jpg.png", EIF_AUTODETECT );

    image_load( &img, "samples/lenna.png", EIF_AUTODETECT );
    image_save( &img, "lenna.png.png", EIF_AUTODETECT );

    image_load( &img, "samples/lenna32.bmp", EIF_AUTODETECT );
    image_save( &img, "lenna32.bmp.png", EIF_AUTODETECT );

    image_load( &img, "samples/lenna24.bmp", EIF_AUTODETECT );
    image_save( &img, "lenna24.bmp.png", EIF_AUTODETECT );

    image_load( &img, "samples/lennaRGB.tga", EIF_AUTODETECT );
    image_save( &img, "lennaRGB.tga.png", EIF_AUTODETECT );

    image_load( &img, "samples/lennaRGBRLE.tga", EIF_AUTODETECT );
    image_save( &img, "lennaRGBRLE.tga.png", EIF_AUTODETECT );

    image_load( &img, "samples/grayscaleRLE.tga", EIF_AUTODETECT );
    image_save( &img, "grayscaleRLE.tga.png", EIF_AUTODETECT );

    image_deinit( &img );

    return 0;
}

