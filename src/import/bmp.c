#ifdef IMAGE_LOAD_BMP



#include "image.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>



/*
    Bitmap loading facilities.

    What should work:
      - Importing 24 bit per pixel images
      - Importing 32 bit per pixel images
      - Flipping images upside-down while loading if needed to move the origin
        to the upper left corner

    What is implemented but has never been tested:
      - Importing 16 bit per pixel images
      - Importing 8 bit per pixel color mapped images with RLE compression
      - Importing 8 bit per pixel color mapped images
      - Importing generic bitfield encoded images

    TODO:
      - Optimize loading, we read one pixel a time from the file!!
      - Clean the code a bit
*/



#define BI_RGB       0
#define BI_RLE8      1
#define BI_RLE4      2
#define BI_BITFIELDS 3

static void loadBMPbitfields( void* src, const SFileIOInterface* io,
                              size_t w, size_t h, unsigned char* ptr,
                              int bpp, int B, int G, int R )
{
    int rshift=0, gshift=0, bshift=0, scaleR, scaleG, scaleB, v;
    size_t bytePerPixel = bpp/8;
    size_t padding = (w*bytePerPixel) % 2, x;
    unsigned char v0[4] = {0,0,0,0};
    unsigned char* end = ptr + h*3*w;

    /*
        We will read multibyte integers from the file and AND them with the
        color masks, to get the color values. The ANDed values have to be
        shifted to the right to get the final color values. Here we work
        out how many bits we have to shift.
     */
    for( ; !((R>>rshift) & 1); ++rshift );
    for( ; !((G>>gshift) & 1); ++gshift );
    for( ; !((B>>bshift) & 1); ++bshift );

    scaleR = 0xFF / (R>>rshift);
    scaleG = 0xFF / (G>>gshift);
    scaleB = 0xFF / (B>>bshift);

    /* for each scanline */
    while( ptr<end )
    {
        /* for each pixel in a scanline */
        for( x=0; x<w; ++x, ptr+=3 )
        {
            /* Read the color value from the file */
            io->read( v0, 1, bytePerPixel, src );
            v = READ_LITTLE_ENDIAN_32( v0, 0 );

            /* determine and store actual color values */
            ptr[0] = ((v & R)>>rshift)*scaleR;
            ptr[1] = ((v & G)>>gshift)*scaleG;
            ptr[2] = ((v & B)>>bshift)*scaleB;
        }

        /* Skip the zeroes added for padding */
        io->seek( src, padding, SEEK_CUR );
    }
}

static void loadBMPcolormap( void* src, const SFileIOInterface* io,
                             size_t w, size_t h, unsigned char* ptr,
                             unsigned char* colorMap )
{
    unsigned char i, *end = ptr + h*3*w;
    size_t padding = w % 2, x;

    /* for each scanline */
    while( ptr<end )
    {
        /* for each pixel in a scanline */
        for( x=0; x<w; ++x, ptr+=3 )
        {
            io->read( &i, 1, 1, src );      /* read color map index */

            ptr[0] = colorMap[ i*4     ];   /* store color from color map */
            ptr[1] = colorMap[ i*4 + 1 ];
            ptr[2] = colorMap[ i*4 + 2 ];
        }

        /* Skip the zeroes added for padding */
        io->seek( src, padding, SEEK_CUR );
    }
}

static void loadBMPrle( void* src, const SFileIOInterface* io,
                        size_t w, size_t h, unsigned char* ptr,
                        unsigned char* colorMap )
{
    unsigned char* end = ptr + 3*w*h;
    size_t i, j, padding;
    unsigned char v[2], c[2], R, G, B;

    while( ptr<end )
    {
        io->read( v, 1, 2, src );          /* 2 byte command */

        if( v[0] )                         /* v[0]>0: same color repeated */
        {
            i = 4*v[1];                        /* v[1] = color map index */

            B = colorMap[i  ];
            G = colorMap[i+1];
            R = colorMap[i+2];

            for( j=0; j<v[0] && ptr<end; ++j ) /* repeate color v[0] times */
            {
                *(ptr++) = R;
                *(ptr++) = G;
                *(ptr++) = B;
            }
        }
        else if( v[1] == 2 )              /* v[1]==2: skip forward */
        {
            io->read( c, 1, 2, src );
            ptr += 3*(c[1]*w + c[0]);
        }
        else if( v[1]>2 )                 /* read v[1] raw pixels */
        {
            for( i=0; i<v[1] && ptr<end; ++i )
            {
                io->read( c, 1, 1, src );
                i = 4*c[0];

                *(ptr++) = colorMap[i+2];
                *(ptr++) = colorMap[i+1];
                *(ptr++) = colorMap[i  ];
            }

            padding = (v[1] % 2);
            io->seek( src, padding, SEEK_CUR );
        }
    }
}



E_LOAD_RESULT load_bmp( SImage* img, void* file, const SFileIOInterface* io )
{
    unsigned char header[ 54 ];
    unsigned char colorMask[ 12 ];
    unsigned char* colorMap = NULL;
    int bfOffBits, biWidth, biBitCount, biCompression, biClrUsed;
    int maskR=0, maskG=0, maskB=0, x, biHeight, flipImage;

    /* read the header */
    io->read( header, 1, 54, file );

    bfOffBits     = READ_LITTLE_ENDIAN_32( header, 10 );
    biWidth       = READ_LITTLE_ENDIAN_32( header, 18 );
    biHeight      = READ_LITTLE_ENDIAN_32( header, 22 );
    biBitCount    = READ_LITTLE_ENDIAN_16( header, 28 );
    biCompression = READ_LITTLE_ENDIAN_32( header, 30 );
    biClrUsed     = READ_LITTLE_ENDIAN_32( header, 46 );

    flipImage = (biHeight>0);
    biHeight  = (biHeight<0) ? -biHeight : biHeight;
    biClrUsed = biClrUsed ? biClrUsed : 256;

    /* sanity check */
    if( header[0]!='B' || header[1]!='M' || biCompression>3 )
        return ELR_FILE_CORRUPTED;

    if( biBitCount!=8 && biBitCount!=16 && biBitCount!=24 && biBitCount!=32 )
        return ELR_FILE_CORRUPTED;

    if( biCompression==BI_RLE8 && (biBitCount!=8 || flipImage) )
        return ELR_FILE_CORRUPTED;

    if( biCompression==BI_BITFIELDS && biBitCount!=16 && biBitCount!=32 )
        return ELR_FILE_CORRUPTED;

    if( biBitCount==1 || biBitCount==2 || biBitCount==4 ||
        biCompression==BI_RLE4 || header[14]!=40 )
        return ELR_NOT_SUPPORTED;

    /* Read color mask */
    if( biCompression==BI_BITFIELDS )
    {
        io->read( colorMask, 1, 12, file );

        maskR = READ_LITTLE_ENDIAN_32( colorMask, 0 );
        maskG = READ_LITTLE_ENDIAN_32( colorMask, 4 );
        maskB = READ_LITTLE_ENDIAN_32( colorMask, 8 );

        /*
            ORing the color masks must result in a continous block of
            set bits. None of the masks must be zero and they must not
            overlap.
         */
        x = maskR | maskG | maskB;

        if( !x || (maskR & maskG & maskB) )
            return ELR_FILE_CORRUPTED;

        /*
            Shift the block to the right until the first bit is set.
            After that, x+1 MUST be some power of two if the block is
            continous. If x+1 is a power of two, ((x+1) & x) must be 0
         */
        while( !(x & 1) )
            x>>=1;

        if( x & (x+1) )
            return ELR_FILE_CORRUPTED;
    }

    /* Read the color map */
    if( biBitCount==8 )
    {
        colorMap = malloc( 4*biClrUsed );
        io->read( colorMap, 1, 4*biClrUsed, file );
    }

    /* Create the image buffer */
    image_allocate_buffer( img, biWidth, biHeight, ECT_RGB8 );

    /* Read the image data */
    io->seek( file, bfOffBits, SEEK_SET );

    switch( biCompression )
    {
    case BI_BITFIELDS:
        loadBMPbitfields( file, io, biWidth, biHeight, img->image_buffer,
                          biBitCount, maskR, maskG, maskB );
        break;
    case BI_RGB:
        if( biBitCount==8 )
            loadBMPcolormap( file, io, biWidth, biHeight, img->image_buffer,
                             colorMap );
        else if( biBitCount==16 )
            loadBMPbitfields( file, io, biWidth, biHeight, img->image_buffer,
                              biBitCount, 0x1F, 0x3E0, 0x7C00 );
        else
            loadBMPbitfields( file, io, biWidth, biHeight, img->image_buffer,
                              biBitCount, 0xFF, 0xFF00, 0xFF0000 );
        break;
    case BI_RLE8:
        loadBMPrle( file, io, biWidth, biHeight, img->image_buffer,
                    colorMap );
        break;
    };

    /* flip the origin to the right position */
    if( flipImage )
        image_flip_v( img );

    /* cleanup */
    free( colorMap );
    return ELR_SUCESS;
}

#endif

