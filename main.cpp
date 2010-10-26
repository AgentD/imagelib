#include "image.h"

#include <iostream>




int main( int argc, char** argv)
{
	CImage image;

	image.load( "test.jpg" );
	image.save( "test.png" );

	return 0;
}

