#ifdef IMAGE_LOAD_TGA



#include "image.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>



/*
    The TGA loading facilities.

    What should work:
      - Importing 8 bit per pixel gray scale images
      - Importing 8 bit per pixel gray scale images with RLE compression
      - Importing 24 & 32 bit per pixel RGB/RGBA images
      - Importing 24 & 32 bit per pixel RGB/RGBA images with RLE compression
      - Flip the image vertically during loading if needed to move the origin
        to the upper left corner
      - Convert BGR to RGB and BGRA to RGBA on loading

    What is implemented but has never ever been tested:
      - Importing color mapped 24/32 bit per pixel RGB/RGBA images
      - Importing color mapped 24/32 bit per pixel RGB/RGBA images with RLE
        compression
      - Flip the image horizontally during loading if needed to move the
        origin to the upper left corner
*/

#define COLOR_MAPPED      1
#define RGB               2
#define GRAYSCALE         3

#define COLOR_MAPPED_RLE  9
#define RGB_RLE          10
#define GRAYSCALE_RLE    11

static void load_cmap( void* file, const SFileIOInterface* io,
                       SImage* img, unsigned char* cmap,
                       size_t bpp, size_t cmap_bpp )
{
    unsigned char *mapPtr, *ptr;
    unsigned char c0[ 4 ] = { 0, 0, 0, 0 }, temp;
    size_t j, i, c, k;

    ptr = img->image_buffer;

    for( i=0; i<img->height; ++i )
    {
        for( j=0; j<img->width; ++j, ptr+=cmap_bpp )
        {
            /* read little endian color map index */
            io->read( c0, 1, bpp, file );
            c = READ_LITTLE_ENDIAN_32( c0, 0 );

            /* get color map entry */
            mapPtr = cmap + c*cmap_bpp;

            /* copy color map entry to pixel */
            for( k=0; k<cmap_bpp; ++k )
                ptr[k] = mapPtr[k];
        }
    }
}

static void load_cmap_rle( void* file, const SFileIOInterface* io,
                           SImage* img, unsigned char* cmap,
                           size_t bpp, size_t cmap_bpp )
{
    unsigned char packet=0, data[4] = {0,0,0,0}, *t;
    unsigned char* ptr = img->image_buffer;
    unsigned char* end = ptr + img->width * img->height * cmap_bpp;
    size_t c, i, j;

    while( ptr<end )
    {
        io->read( &packet, 1, 1, file );

        /* determine whether it is a raw or a run packet */
        if( packet & 0x80 )
        {
            packet &= 0x7F;    /* decode packet */

            /* get color map entry */
            io->read( data, 1, bpp, file );
            c = READ_LITTLE_ENDIAN_32( data, 0 );
            t = cmap + c*cmap_bpp;

            /* repeate color value */
            for( i=0; i<=packet && ptr<end; ++i, ptr+=cmap_bpp )
            {
                for( j=0; j<cmap_bpp; ++j )
                    ptr[j] = t[j];
            }
        }
        else
        {
            packet += 1;    /* decode packet */

            /* read color values and copy */
            for( i=0; i<=packet && ptr<end; ++i, ptr+=cmap_bpp )
            {
                io->read( data, 1, bpp, file );
                c = READ_LITTLE_ENDIAN_32( data, 0 );
                t = cmap + c*cmap_bpp;

                for( j=0; j<cmap_bpp; ++j )
                    ptr[j] = t[j];
            }
        }
    }
}

static void load_rle( void* file, const SFileIOInterface* io,
                      size_t bpp, SImage* img )
{
    unsigned char packet = 0, data[ 4 ] = {0,0,0,0};
    unsigned char* ptr = img->image_buffer;
    unsigned char* end = ptr + img->width * img->height * bpp;
    size_t i, j;

    while( ptr<end )
    {
        io->read( &packet, 1, 1, file );    /* read a packet */

        if( packet & 0x80 )                 /* is it a run packet? */
        {
            io->read( data, 1, bpp, file );     /* read a color value */
            packet &= 0x7F;                     /* decode packet */

            for( i=0; i<=packet && ptr<end; ++i )/* repeate color value */
            {
                for( j=0; j<bpp; ++j )
                    *(ptr++) = data[j];
            }
        }
        else
        {
            packet += 1;                        /* decode packet */

            if( (ptr+packet*bpp) >= end )       /* check bounds */
                packet = (end - ptr)/bpp;

            io->read( ptr, bpp, packet, file ); /* read raw colors */
            ptr += bpp * packet;
        }
    }
}



E_LOAD_RESULT load_tga( SImage* img, void* file, const SFileIOInterface* io )
{
    unsigned char header[ 18 ];
    int width, height, bpp, cmap_bpp, have_cmap, pic_type;
    size_t cmap_offset, cmap_end, cmap_bytes;
    unsigned char* cmap = NULL;
    E_COLOR_TYPE type = ECT_GRAYSCALE8;

    /* read the TGA header */
    io->read( header, 1, 18, file );
    io->seek( file, header[0], SEEK_CUR );   /* Skip the image ID field */

    pic_type    = header[ 2 ];
    width       = READ_LITTLE_ENDIAN_16( header, 12 );
    height      = READ_LITTLE_ENDIAN_16( header, 14 );
    bpp         = header[ 16 ] / 8;

    have_cmap   = header[ 1 ];
    cmap_offset = READ_LITTLE_ENDIAN_16( header, 3 );
    cmap_end    = READ_LITTLE_ENDIAN_16( header, 5 );
    cmap_bpp    = header[ 7 ] / 8;

    cmap_bytes  = (cmap_end - cmap_offset)*cmap_bpp;

    /* sanity checks */
    if( pic_type<1 || pic_type>11 || (pic_type>3 && pic_type<9) )
        return ELR_FILE_CORRUPTED;

    if( !width || !height )
        return ELR_FILE_CORRUPTED;

    if( (pic_type==1||pic_type==9) && (!have_cmap||cmap_offset>=cmap_end) )
        return ELR_FILE_CORRUPTED;

    if( cmap_bpp!=3 && cmap_bpp!=4 && have_cmap )
        return ELR_NOT_SUPPORTED;

    if( bpp!=1 && (pic_type==GRAYSCALE || pic_type==GRAYSCALE_RLE) )
        return ELR_NOT_SUPPORTED;

    if( bpp!=3 && bpp!=4 && (pic_type==RGB || pic_type==RGB_RLE) )
        return ELR_NOT_SUPPORTED;

    /* Read the color map if present and allocate an image buffer */
    if( have_cmap )
    {
        type = cmap_bpp==4 ? ECT_RGBA8 : ECT_RGB8;
        cmap = malloc( cmap_bytes );

        io->seek( file, cmap_offset, SEEK_CUR );
        io->read( cmap, 1, cmap_bytes, file );
    }
    else if( pic_type==RGB || pic_type==RGB_RLE )
    {
        type = bpp==4 ? ECT_RGBA8 : ECT_RGB8;
    }

    image_allocate_buffer( img, width, height, type );

    /* Load the image */
    switch( pic_type )
    {
    case COLOR_MAPPED:
        load_cmap( file, io, img, cmap, bpp, cmap_bpp );
        break;
    case COLOR_MAPPED_RLE:
        load_cmap_rle( file, io, img, cmap, bpp, cmap_bpp );
        break;
    case RGB:
        io->read( img->image_buffer, bpp*width, height, file );
        break;
    case GRAYSCALE:
        io->read( img->image_buffer, width, height, file );
        break;
    case RGB_RLE:
        load_rle( file, io, bpp, img );
        break;
    case GRAYSCALE_RLE:
        load_rle( file, io, 1, img );
        break;
    };

    /* flip the origin to the right position */
    if( !(header[17] & 1<<5) )
        image_flip_v( img );

    if( header[17] & 1<<4 )
        image_flip_h( img );

    /* swap red and blue channels */
    image_swap_channels( img, 0, 2 );

    /* cleanup */
    free( cmap );

    return ELR_SUCESS;
}

#endif

