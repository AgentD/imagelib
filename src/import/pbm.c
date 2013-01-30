#ifdef IMAGE_LOAD_PBM

#include "image.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>



/*
    NETBPM loading facilities.

    What should work:
      - Importing P1 ASCII bitmap images
      - Importing P2 ASCII grayscale images
      - Importing P3 ASCII RGB images
      - Importing P5 binary grayscale images
      - Importing P6 binary RGB images

    What is implemented but has never been tested:
      - Importing P4 binary bitmap images
*/



static void read_next_value( void* file, const SFileIOInterface* io,
                             char* buffer, size_t size )
{
    size_t i;

    while( 1 )
    {
        /* skip space */
        while( io->read( buffer, 1, 1, file ) && isspace( buffer[0] ) );

        /* in case we hit a comment */
        if( buffer[0]=='#' )
        {
            /* skip comment */
            while( io->read( buffer, 1, 1, file ) && buffer[0]!='\r' &&
                                                     buffer[0]!='\n' );
        }
        else
        {
            /* read characters until we hit a space */
            for( i=1; i<size && io->read( buffer+i, 1, 1, file ) &&
                      !isspace( buffer[i] ); ++i );

            if( i<size )
                buffer[i] = '\0';
            else
                buffer[size-1] = '\0';

            return;
        }
    }
}



E_LOAD_RESULT load_pbm( SImage* img, void* file, const SFileIOInterface* io )
{
    char buffer[ 40 ], pbm_format;
    size_t width, height, i, j, size;
    unsigned char* ptr;
    int val, max_val;
    float scale;

    /* read format identifyer */
    io->read( buffer, 1, 3, file );

    if( buffer[0]!='P' || buffer[1]<'1' || buffer[1]>'6' )
        return ELR_FILE_CORRUPTED;

    if( !isspace( buffer[2] ) )
        return ELR_FILE_CORRUPTED;

    /* save the format specifyer from the file */
    pbm_format = buffer[1];

    /* read image size */
    read_next_value( file, io, buffer, sizeof(buffer) );
    width = strtol( buffer, NULL, 10 );

    read_next_value( file, io, buffer, sizeof(buffer) );
    height = strtol( buffer, NULL, 10 );

    /* read maximum color value if required */
    if( pbm_format!='1' && pbm_format!='4' )
    {
        read_next_value( file, io, buffer, sizeof(buffer) );
        max_val = strtol( buffer, NULL, 10 );

        /* BPM spec says, must be in [1,65535] range */
        if( max_val<1 || max_val>65535 )
            return ELR_FILE_CORRUPTED;

        /* compute scale factor for [0,255] range */
        scale = 255.0f/((float)max_val);
    }

    /* Allocate the image buffer */
    image_allocate_buffer( img, width, height,
                           (pbm_format=='3' || pbm_format=='6') ?
                           ECT_RGB8 : ECT_GRAYSCALE8 );

    /*
       P1, P2 and P3 are ascii formats, P4, P5 and P6 are binary
       P1 and P4 are bitmap, P2 and P5 are grayscale,
       P3 and P6 are RGB
     */
    ptr = img->image_buffer;
    size = width*height;

    if( pbm_format == '1' )
    {
        for( j=0; j<size; ++j )
        {
            read_next_value( file, io, buffer, sizeof(buffer) );

            *(ptr++) = buffer[0]=='1' ? 255 : 0;
        }
    }
    else if( pbm_format == '2' )
    {
        for( j=0; j<size; ++j )
        {
            read_next_value( file, io, buffer, sizeof(buffer) );
            val = strtol( buffer, NULL, 10 );
            *(ptr++) = val * scale;
        }
    }
    else if( pbm_format == '3' )
    {
        for( j=0; j<size; ++j )
        {
            /* red */
            read_next_value( file, io, buffer, sizeof(buffer) );
            val = strtol( buffer, NULL, 10 );
            *(ptr++) = val * scale;

            /* green */
            read_next_value( file, io, buffer, sizeof(buffer) );
            val = strtol( buffer, NULL, 10 );
            *(ptr++) = val * scale;

            /* blue */
            read_next_value( file, io, buffer, sizeof(buffer) );
            val = strtol( buffer, NULL, 10 );
            *(ptr++) = val * scale;
        }
    }
    else if( pbm_format == '4' )
    {
        for( j=0; j<size; ++j )
        {
            io->read( buffer, 1, 1, file );

            /* decode bits */
            for( i=0; i<8 && j<size; ++i, ++j )
            {
                *(ptr++) = (buffer[0] & (1<<(7-i))) ? 255 : 0;
            }
        }
    }
    else if( pbm_format == '5' )
    {
        if( max_val>255 )
        {
            for( j=0; j<size; ++j )
            {
                io->read( buffer, 1, 2, file );
                val = READ_BIG_ENDIAN_16( buffer, 0 );
                *(ptr++) = val * scale;
            }
        }
        else
        {
            io->read( ptr, 1, size, file );

            for( j=0; j<size; ++j )
            {
                *(ptr++) *= scale;
            }
        }
    }
    else if( pbm_format == '6' )
    {
        if( max_val>255 )
        {
            for( j=0; j<size; ++j )
            {
                /* red */
                io->read( buffer, 1, 2, file );
                val = READ_BIG_ENDIAN_16( buffer, 0 );
                *(ptr++) = val * scale;

                /* green */
                io->read( buffer, 1, 2, file );
                val = READ_BIG_ENDIAN_16( buffer, 0 );
                *(ptr++) = val * scale;

                /* blue */
                io->read( buffer, 1, 2, file );
                val = READ_BIG_ENDIAN_16( buffer, 0 );
                *(ptr++) = val * scale;
            }
        }
        else
        {
            io->read( ptr, 1, size*3, file );

            for( j=0; j<size; ++j )
            {
                *(ptr++) *= scale;  /* red */
                *(ptr++) *= scale;  /* green */
                *(ptr++) *= scale;  /* blue */
            }
        }
    }

    return ELR_SUCESS;
}

#endif

