#include "image.h"

#ifdef _WIN32
    #include <direct.h>
    #define mkdir _mkdir
#else
    #include <sys/stat.h>
    #include <sys/types.h>
#endif


/****************************************************************************
 *                                                                          *
 * The following code tests the exporter modules by generating images in    *
 * all supported color modes and exporting them.                            *
 *                                                                          *
 * For RGB, a blueish mandelbrot set is generated, for RGBA, the same but   *
 * with an alpha ramp overlayed. For grayscale, a grayscale ramp along the  *
 * x axis is generated, where the color is then multiplyed with a ramp      *
 * along the y axis.                                                        *
 *                                                                          *
 ****************************************************************************/



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

#ifdef _WIN32
    mkdir( "rgb8"  );
    mkdir( "rgba8" );
    mkdir( "gray8" );
#else
    mkdir( "rgb8",  0777 );
    mkdir( "rgba8", 0777 );
    mkdir( "gray8", 0777 );
#endif

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

    image_save( &image, "rgb8/test.tga", EIF_AUTODETECT );
    image_save( &image, "rgb8/test.bmp", EIF_AUTODETECT );
    image_save( &image, "rgb8/test.jpg", EIF_AUTODETECT );
    image_save( &image, "rgb8/test.png", EIF_AUTODETECT );
    image_save( &image, "rgb8/test.pbm", EIF_AUTODETECT );

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

    image_save( &image, "rgba8/test.tga", EIF_AUTODETECT );
    image_save( &image, "rgba8/test.bmp", EIF_AUTODETECT );
    image_save( &image, "rgba8/test.jpg", EIF_AUTODETECT );
    image_save( &image, "rgba8/test.png", EIF_AUTODETECT );
    image_save( &image, "rgba8/test.pbm", EIF_AUTODETECT );

    /******************** generate grayscale test images ********************/
    image_allocate_buffer( &image, 800, 600, ECT_GRAYSCALE8 );

    b = image.image_buffer;

    for( y=0; y<600; ++y )
        for( x=0; x<800; ++x )
            *(b++) = 255.0f*(((float)x) / 800.0f) * (((float)y) / 600.0f);

    image_save( &image, "gray8/test.tga", EIF_AUTODETECT );
    image_save( &image, "gray8/test.bmp", EIF_AUTODETECT );
    image_save( &image, "gray8/test.jpg", EIF_AUTODETECT );
    image_save( &image, "gray8/test.png", EIF_AUTODETECT );
    image_save( &image, "gray8/test.pbm", EIF_AUTODETECT );

    image_deinit( &image );

    return 0;
}

