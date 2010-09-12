#include "image.h"



int main( int argc, char** argv)
{
	CImage image;

	image.load( "test.bmp" );
	image.save( "test.tga" );

	return 0;
}

