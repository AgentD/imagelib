#include "image.h"

#include <iostream>




int main( int argc, char** argv)
{
	CImage image;

	image.load( "lena512.bmp" );
	image.save( "test.tga" );

	return 0;
}

