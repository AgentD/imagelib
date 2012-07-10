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

    /* read the file into a buffer */
    fseek( file, 0, SEEK_END );
    length = ftell( file );
    fseek( file, 0, SEEK_SET );

    buffer = malloc( length );

    fread( buffer, 1, length, file );

    /********************** Allocate the image buffer **********************/

    /* determine the length of the longest line and number of lines */
    for( width = 0, height = 0, len=0, i=0; i<length; ++i )
    {
        /* if we find CRLF, skip CR. LF is handled below */
        if( (i+1)<length && buffer[i]=='\r' && buffer[i+1]=='\n' )
            ++i;

        /* if we find a single CR or a LF */
        if( buffer[i]=='\r' || buffer[i]=='\n' )
        {
            ++height;           /* increase line count */

            if( len > width )   /* if the line was longer than the longest */
                width = len;        /* store it's length as the longest */

            len = 0;            /* reset length counter and continue */
            continue;
        }

        ++len;                  /* increase line lenght counter */
    }

    if( !height )           /* if there is only one line, set manually */
        height = 1;

    width  *= CHAR_WIDTH;   /* determine actual image size in pixels */
    height *= CHAR_HEIGHT;

    /* allocate the buffer */
    image_allocate_buffer( img, width, height, ECT_GRAYSCALE8 );

    /************************* Generate the image *************************/

    /* clear the image to black */
    memset( img->image_buffer, 0, width*height );

    /* iterate over all characters in the buffer */
    for( x=0, y=0, i=0, dst=img->image_buffer; i < length; ++i )
    {
        /* if we find CRLF, skip CR. LF is handled below */
        if( (i+1)<length && buffer[i]=='\r' && buffer[i+1]=='\n' )
            ++i;

        /* if we find a single CR or a LF */
        if( buffer[i]=='\r' || buffer[i]=='\n' )
        {
            x  = 0;             /* move cursor to the beginning */
            y += CHAR_HEIGHT;   /* move cursor one line down and continue */
            continue;
        }

        /* render character and move cursor to next position */
        render_character( buffer[i], dst, x, y, width, height );

        x += CHAR_WIDTH;
    }

    /* cleanup and return */
    free( buffer );

    return ELR_SUCESS;
}



#endif

