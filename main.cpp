#include "image.h"

#include <iostream>



bool isMandelbrot( float X, float Y, float& iter )
{
   float x = 0;
   float y = 0;

   size_t iteration = 0;
   const size_t max_iteration = 40;

   while( (x*x + y*y<=4) && iteration<max_iteration )
   {
      float xtemp = x*x - y*y + X;
      y = 2*x*y + Y;

      x = xtemp;

      ++iteration;
   }

   iter = ((float)iteration)/((float)max_iteration);

   return iteration == max_iteration;
}




int main( int argc, char** argv)
{
	CImage image;

   image.allocateBuffer( 800, 600, 1, EIT_RGB8 );
   unsigned char* b = (unsigned char*)image.getBuffer( );

   for( size_t y=0; y<600; ++y )
   {
      for( size_t x=0; x<800; ++x )
      {
         float i = 0.0f;
         unsigned char R=255, G=255, B=255;

         if( !isMandelbrot( 3*((float)x)/800.0f-2.0f, 2*((float)y)/600.0f - 1, i ) )
         {
            R = G = 0;
            B = 255*i;
         }

         *(b++) = R;
         *(b++) = G;
         *(b++) = B;
      }
   }

    image.setPixel( 100, 500, 0, 255, 0, 0 );
    image.printf( 100, 500, 0, 255, 255, 255, "Test %d, '%s'\nLine wrap test\nAnother line", 1337, "a string" );

	image.save( "test.png" );

	return 0;
}

