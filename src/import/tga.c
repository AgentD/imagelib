#ifdef IMAGE_LOAD_TGA



#include "image.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>



/*
    The TGA loading facilities.

    What should work:
      - Importing 8 bit per pixel gray scale images
      - Importing 24&32 bit per pixel RGB/RGBA images
      - Importing 24&32 bit per pixel RGB/RGBA images with RLE compression
      - Flip the image vertically during loading if needed to move the origin
        to the upper left corner

    What is implemented but has never ever been tested:

      - Importing color mapped 24/32 bit per pixel RGB/RGBA images
      - Importing 8 bit per pixel gray scale images with RLE compression
      - Importing color mapped 24/32 bit per pixel RGB/RGBA images with RLE
        compression
      - Flip the image horizontally during loading if needed to move the
        origin to the upper left corner

    TODO:
      - Convert BGR to RGB and BGRA to RGBA on loading
      - Clean this code up, it is filthy!
*/

enum
{
    COLOR_MAPPED = 1,
    RGB = 2,
    GRAYSCALE = 3,

    COLOR_MAPPED_RLE = 9,
    RGB_RLE = 10,
    GRAYSCALE_RLE = 11
};

typedef struct
{
    size_t width;               // The width of the image in pixels
    size_t height;               // The height of the image in pixels
    size_t bytePerPixel;         // The total number of bytes per pixel over all color channels
    size_t colorMapBytePerPixel;   // The total number of bytes per pixel in the color map over all color channels

    unsigned char* ptr;            // A pointer to the first pixel in the image buffer
    unsigned char* end;            // A pointer to the last pixel in the image buffer
    int xstep;               // How much to add to the pixel poiter to get to the next pixel
    int ystep;               // How much to add to the pixel pointer to get to the next pixel row
}
tgaInfo;

static void loadTgaColorMapped( FILE* file, tgaInfo* i,
                                unsigned char* colorMap )
{
    unsigned char* row;
    unsigned char* mapPtr;
    size_t j, c, k;

    for( ; i->ptr!=i->end; i->ptr+=i->ystep )
    {
        for( row=i->ptr, j=0; j<i->width; ++j, row+=i->xstep )
        {
            unsigned char c0[ 4 ] = { 0, 0, 0, 0 };

            fread( c0, 1, i->bytePerPixel, file );
            c = ((size_t)c0[0]) | ((size_t)c0[1])<<8 | ((size_t)c0[2])<<16 | ((size_t)c0[3])<<24;

            mapPtr = colorMap + c*i->colorMapBytePerPixel;

            for( k=0; k<i->colorMapBytePerPixel; ++k )
                row[k] = mapPtr[k];
        }
    }
}

static void loadTgaColorMappedRLE( FILE* file, tgaInfo* i,
                                   unsigned char* colorMap )
{
    unsigned char* t;
    unsigned char* row;
    unsigned char run=0, raw=0, packet=0, data[4] = {0,0,0,0};
    size_t c, j, k;

    for( ; i->ptr!=i->end; i->ptr+=i->ystep )
    {
        for( row=i->ptr, j=0; j<i->width; ++j, row+=i->xstep )
        {
            if( run )
            {
                --run;
            }
            else if( raw )
            {
                fread( data, 1, i->bytePerPixel, file );
                c = ((size_t)data[0]) | ((size_t)data[1])<<8 | ((size_t)data[2])<<16 | ((size_t)data[3])<<24;
                t = &colorMap[ c*i->colorMapBytePerPixel ];
                --raw;
            }
            else
            {
                fread( &packet, 1, 1, file );
                fread( data, 1, i->bytePerPixel, file );
                c = ((size_t)data[0]) | ((size_t)data[1])<<8 | ((size_t)data[2])<<16 | ((size_t)data[3])<<24;
                t = &colorMap[ c*i->colorMapBytePerPixel ];

                if( packet & 0x80 )
                    run = packet & 0x7F;
                else
                    raw = packet;
            }

            for( k=0; k<i->colorMapBytePerPixel; ++k )
                row[k] = t[k];
        }
    }
}

static void loadTgaRGB( FILE* file, tgaInfo* i )
{
    unsigned char* row;
    size_t j;

    for( ; i->ptr!=i->end; i->ptr+=i->ystep )
    {
        for( row=i->ptr, j=0; j<i->width; ++j, row+=i->xstep )
        {
            fread( row, 1, i->bytePerPixel, file );
        }
    }
}

static void loadTgaRGBRLE( FILE* file, tgaInfo* i )
{
    unsigned char run=0, raw=0, packet=0, data[ 4 ] = {0,0,0,0};
    unsigned char* row;
    size_t j, k;

    for( ; i->ptr!=i->end; i->ptr+=i->ystep )
    {
        for( row = i->ptr, j=0; j<i->width; ++j, row+=i->xstep )
        {
            if( run )
            {
                --run;
            }
            else if( raw )
            {
                fread( data, 1, i->bytePerPixel, file );
                --raw;
            }
            else
            {
                fread( &packet, 1, 1, file );
                fread( data, 1, i->bytePerPixel, file );

                if( packet & 0x80 )
                    run = packet & 0x7F;
                else
                    raw = packet;
            }

            for( k=0; k<i->bytePerPixel; ++k )
                row[k] = data[k];
        }
    }
}



E_LOAD_RESULT load_tga( SImage* img, FILE* file )
{
    /* read the TGA header */
    unsigned char header[ 18 ];

    size_t pictureType, bitsPerPixel;
    size_t attributeByte, colorMapOffset, colorMapLength;
    size_t colorMapSize;
    int colorMapPresent;
    unsigned char* colorMap = NULL;
    E_IMAGE_TYPE type = EIT_GRAYSCALE8;
    tgaInfo i;


    fread( header, 1, 18, file );
    fseek( file, header[0], SEEK_CUR );   // Skip the image ID field

    pictureType     = header[ 2 ];
    i.width         = ((size_t)header[ 12 ]) | (((size_t)header[ 13 ])<<8);
    i.height        = ((size_t)header[ 14 ]) | (((size_t)header[ 15 ])<<8);
    i.bytePerPixel  = header[ 16 ] / 8;
    attributeByte   = header[ 17 ];

    colorMapPresent = header[ 1 ];
    colorMapOffset  = ((size_t)header[ 3 ]) | (((size_t)header[ 4 ])<<8);
    colorMapLength  = ((size_t)header[ 5 ]) | (((size_t)header[ 6 ])<<8);
    i.colorMapBytePerPixel = header[ 7 ] / 8;

    /* sanity checks */
    if( pictureType<1 || pictureType>11 || (pictureType>3&&pictureType<9) )
        return ELR_FILE_CORRUPTED;

    if( !i.width || !i.height )
        return ELR_FILE_CORRUPTED;

    if( (pictureType==1||pictureType==9) &&
        (!colorMapPresent||colorMapOffset>=colorMapLength) )
        return ELR_FILE_CORRUPTED;

    if( i.colorMapBytePerPixel!=3 && i.colorMapBytePerPixel!=4 && colorMapPresent )
        return ELR_NOT_SUPPORTED;

    if( i.bytePerPixel!=1 && (pictureType==GRAYSCALE ||
                              pictureType==GRAYSCALE_RLE) )
        return ELR_NOT_SUPPORTED;

    if( i.bytePerPixel!=3 && i.bytePerPixel!=4 &&
        (pictureType==RGB || pictureType==RGB_RLE) )
        return ELR_NOT_SUPPORTED;

    /* Load the image from the file */
    colorMapSize = (colorMapLength-colorMapOffset)*i.colorMapBytePerPixel;

    if( pictureType==RGB || pictureType==RGB_RLE )
        type = (i.bytePerPixel==4) ? EIT_RGBA8 : EIT_RGB8;

    if( colorMapPresent )
    {
        type     = i.colorMapBytePerPixel==4 ? EIT_RGBA8 : EIT_RGB8;
        colorMap = malloc( colorMapSize );

        fseek( file, colorMapOffset, SEEK_CUR );
        fread( colorMap, 1, colorMapSize, file );
    }

    image_allocate_buffer( img, i.width, i.height, type );

    i.xstep                = (pictureType==1||pictureType==9 ? i.colorMapBytePerPixel : i.bytePerPixel);
    i.ystep                = i.xstep*i.width;

    if( attributeByte & 1<<5 )
    {
        i.ptr = img->image_buffer;
        i.end = i.ptr + i.height*i.ystep;
    }
    else
    {
        i.ptr = (unsigned char*)img->image_buffer + (i.height-1)*i.ystep;
        i.end = i.ptr - i.ystep;
        i.ystep*=-1;
    }

    if( attributeByte & 1<<4 )
        i.xstep*=-1;

    switch( pictureType )
    {
    case COLOR_MAPPED:
        loadTgaColorMapped( file, &i, colorMap );
        break;
    case COLOR_MAPPED_RLE:
        loadTgaColorMappedRLE( file, &i, colorMap );
        break;
    case RGB:
    case GRAYSCALE:
        loadTgaRGB( file, &i );
        break;
    case RGB_RLE:
    case GRAYSCALE_RLE:
        loadTgaRGBRLE( file, &i );
        break;
    };

    free( colorMap );

    return ELR_SUCESS;
}

#endif

