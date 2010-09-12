#include "image.h"



int main( int argc, char** argv)
{
	CImage image;

	image.allocateBuffer( 800, 600, 1, EIT_GRAYSCALE8 );

	unsigned char* b = (unsigned char*)image.getBuffer( );

	for( size_t y=0; y<600; ++y )
		for( size_t x=0; x<800; ++x )
			b[ y*800 + x ] = (unsigned char)( 255.0f*((float)x)/800.0f );

	image.save( "test.bmp" );

	return 0;
}

