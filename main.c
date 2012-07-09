#include "image.h"



int isMandelbrot( float X, float Y, float* iter )
{
    const size_t max_iteration = 40;

    float x = 0, y = 0;
    size_t iteration = 0;

    while( (x*x + y*y<=4) && iteration<max_iteration )
    {
        float xtemp = x*x - y*y + X;
        y = 2*x*y + Y;

        x = xtemp;

        ++iteration;
    }

    *iter = ((float)iteration)/((float)max_iteration);

    return iteration == max_iteration;
}




int main( int argc, char** argv)
{
    SImage image;
    unsigned char* b;
    size_t x, y;

    image_init( &image );

    /*********************** generate RGB test images ***********************/
    image_allocate_buffer( &image, 800, 600, ECT_RGB8 );

    b = image.image_buffer;

    for( y=0; y<600; ++y )
    {
        for( x=0; x<800; ++x )
        {
            float i = 0.0f;
            unsigned char R=255, G=255, B=255;

            if( !isMandelbrot( 3*((float)x)/800.0f-2.0f,
                               2*((float)y)/600.0f - 1, &i ) )
            {
                R = G = 0;
                B = 255*i;
            }

            *(b++) = R;
            *(b++) = G;
            *(b++) = B;
        }
    }

    image_set_pixel( &image, 100, 500, 255, 0, 0 );
    image_print_string( &image, 100, 500, 255, 255, 255,
                        "Test\nLine wrap test\nAnother line" );

    image_save( &image, "rgb8/test.txt", EIF_AUTODETECT );
    image_save( &image, "rgb8/test.tga", EIF_AUTODETECT );
    image_save( &image, "rgb8/test.bmp", EIF_AUTODETECT );
    image_save( &image, "rgb8/test.jpg", EIF_AUTODETECT );
    image_save( &image, "rgb8/test.png", EIF_AUTODETECT );

    /********************** generate RGBA test images ***********************/
    image_allocate_buffer( &image, 800, 600, ECT_RGBA8 );

    b = image.image_buffer;

    for( y=0; y<600; ++y )
    {
        for( x=0; x<800; ++x )
        {
            float i = 0.0f;
            unsigned char R=255, G=255, B=255;

            if( !isMandelbrot( 3*((float)x)/800.0f-2.0f,
                               2*((float)y)/600.0f - 1, &i ) )
            {
                R = G = 0;
                B = 255*i;
            }

            *(b++) = R;
            *(b++) = G;
            *(b++) = B;
            *(b++) = 255.0f * (1.0f - ((float)x) / 800.0f);
        }
    }

    image_set_pixel( &image, 100, 500, 255, 0, 0 );
    image_print_string( &image, 100, 500, 255, 255, 255,
                        "Test\nLine wrap test\nAnother line" );

    image_save( &image, "rgba8/test.txt", EIF_AUTODETECT );
    image_save( &image, "rgba8/test.tga", EIF_AUTODETECT );
    image_save( &image, "rgba8/test.bmp", EIF_AUTODETECT );
    image_save( &image, "rgba8/test.jpg", EIF_AUTODETECT );
    image_save( &image, "rgba8/test.png", EIF_AUTODETECT );

    /******************** generate grayscale test images ********************/
    image_allocate_buffer( &image, 800, 600, ECT_GRAYSCALE8 );

    b = image.image_buffer;

    for( y=0; y<600; ++y )
        for( x=0; x<800; ++x )
            *(b++) = 255.0f*(((float)x) / 800.0f) * (((float)y) / 600.0f);

    image_set_pixel( &image, 100, 500, 255, 0, 0 );
    image_print_string( &image, 100, 500, 255, 255, 255, "Grayscale Test" );

    image_save( &image, "gray8/test.txt", EIF_AUTODETECT );
    image_save( &image, "gray8/test.tga", EIF_AUTODETECT );
    image_save( &image, "gray8/test.bmp", EIF_AUTODETECT );
    image_save( &image, "gray8/test.jpg", EIF_AUTODETECT );
    image_save( &image, "gray8/test.png", EIF_AUTODETECT );

    image_deinit( &image );

    return 0;
}

