#ifdef IMAGE_LOAD_TXT



#include "image.h"
#include "bitmap_font.h"
#include "util.h"



#include <stdio.h>
#include <stdlib.h>
#include <string.h>



E_LOAD_RESULT load_txt( SImage* img, FILE* file )
{
    char* buffer;
    unsigned char* dst;
    size_t length, i, j, k, x, y, len, width, height;

    fseek( file, 0, SEEK_END );
    length = ftell( file );
    fseek( file, 0, SEEK_SET );

    buffer = malloc( length );

    fread( buffer, 1, length, file );

    ////////////////////// Allocate the image buffer ///////////////////////
    for( width = 0, height = 1, len=0, i=0; i<length; ++i )
    {
        if( (i+1)<length && buffer[i]=='\r' && buffer[i+1]=='\n' )
            ++i;

        if( buffer[i]=='\r' || buffer[i]=='\n' )
        {
            ++height;

            if( len > width )
                width = len;

            len = 0;
            continue;
        }

        ++len;
    }

    width  *= CHAR_WIDTH;
    height *= CHAR_HEIGHT;

    image_allocate_buffer( img, width, height, EIT_GRAYSCALE8 );

    ////////////////////////// Generate the image //////////////////////////
    memset( img->image_buffer, 0, width*height );

    for( x=0, y=0, i=0, dst=img->image_buffer; i < length; ++i )
    {
        if( (i+1)<length && buffer[i]=='\r' && buffer[i+1]=='\n' )
            ++i;

        if( buffer[i]=='\r' || buffer[i]=='\n' )
        {
            x  = 0;
            y += CHAR_HEIGHT;
            continue;
        }

        render_character( buffer[i], dst, x, y, width, height );

        x += CHAR_WIDTH;
    }

    // cleanup
    free( buffer );

    return ELR_SUCESS;
}



#endif

