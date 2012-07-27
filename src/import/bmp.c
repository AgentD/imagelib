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
      - Clead the code a bit
*/



enum
{
    BI_RGB       = 0,
    BI_RLE8      = 1,
    BI_RLE4      = 2,
    BI_BITFIELDS = 3
};

static void loadBMPbitfields( void* src, const SFileIOInterface* io,
                              size_t w, size_t h, void* ptr, size_t bpp,
                              size_t B, size_t G, size_t R, int flip )
{
    int    ystep = flip ? -3*w : 3*w;
    size_t bytePerPixel = bpp/8;
    size_t padding = (w*bytePerPixel) % 2;
    size_t rshift=0, gshift=0, bshift=0;
    size_t scaleR, scaleG, scaleB, x, v;

    unsigned char* cur = (unsigned char*)ptr;
    unsigned char* end = cur + h*3*w;

    if( flip )
    {
        end  = cur - 3*w;
        cur += (h-1)*3*w;
    }

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
    for( ; cur!=end; cur+=ystep )
    {
        unsigned char* row = cur;

        /* for each pixel in a scanline */
        for( x=0; x<w; ++x, row+=3 )
        {
            unsigned char v0[4] = {0,0,0,0};

            /* Read the color value from the file */
            io->read( v0, 1, bytePerPixel, src );

            v = ((size_t)v0[0])     | ((size_t)v0[1])<<8 |
                ((size_t)v0[2])<<16 | ((size_t)v0[3])<<24;

            /* determine and store actual color values */
            row[2] = (unsigned char)( (v & B)>>bshift )*scaleB;
            row[1] = (unsigned char)( (v & G)>>gshift )*scaleG;
            row[0] = (unsigned char)( (v & R)>>rshift )*scaleR;
        }

        /* Skip the zeroes added for padding */
        io->seek( src, padding, SEEK_CUR );
    }
}

static void loadBMPcolormap( void* src, const SFileIOInterface* io,
                             size_t w, size_t h, void* ptr,
                             unsigned char* colorMap, int flip )
{
    unsigned char* cur = (unsigned char*)ptr;
    unsigned char* end = cur + h*3*w;
    size_t padding = w % 2;
    int ystep = flip ? -3*w : 3*w;
    size_t x;

    if( flip )
    {
        end  = cur - 3*w;
        cur += (h-1)*3*w;
    }

    /* for each scanline */
    for( ; cur!=end; cur+=ystep )
    {
        unsigned char* row = cur;

        /* for each pixel in a scanline */
        for( x=0; x<w; ++x, row+=3 )
        {
            char i;

            io->read( &i, 1, 1, src );      /* read color map index */

            row[0] = colorMap[ i*4     ];   /* store color from color map */
            row[1] = colorMap[ i*4 + 1 ];
            row[2] = colorMap[ i*4 + 2 ];
        }

        /* Skip the zeroes added for padding */
        io->seek( src, padding, SEEK_CUR );
    }
}

static void loadBMPrle( void* src, const SFileIOInterface* io,
                        size_t w, size_t h, void* ptr,
                        unsigned char* colorMap )
{
    unsigned char* beg = (unsigned char*)ptr;
    unsigned char* cur = beg;
    unsigned char* end = beg + 3*w*h;
    size_t i, j, index, padding;
    unsigned char v[2], c[2], R, G, B;

    while( cur!=end && !io->eof( src ) )
    {
        io->read( v, 1, 2, src );

        if( v[0] )
        {
            i = 4*((size_t)v[1]);

            B = colorMap[i  ];
            G = colorMap[i+1];
            R = colorMap[i+2];

            for( j=0; j<v[0] && cur!=end; ++j, cur+=3 )
            {
                cur[0] = R;
                cur[1] = G;
                cur[2] = B;
            }
        }
        else
        {
            if( v[1] == 2 )
            {
                io->read( c, 1, 2, src );

                cur += 3*  ((size_t)c[0]);
                cur += 3*w*((size_t)c[1]);
            }
            else if( v[1]>2 )
            {
                for( i=0; i<v[1] && cur!=end; ++i, cur+=3 )
                {
                    io->read( c, 1, 1, src );

                    index = 4*((size_t)c[0]);

                    cur[2] = colorMap[index  ];
                    cur[1] = colorMap[index+1];
                    cur[0] = colorMap[index+2];
                }

                padding = (v[1] % 2);

                io->seek( src, padding, SEEK_CUR );
            }
        }
    }
}



E_LOAD_RESULT load_bmp( SImage* img, void* file, const SFileIOInterface* io )
{
    unsigned char header[ 54 ];
    size_t bfOffBits, biWidth, biBitCount, biCompression, biClrUsed;
    size_t maskR=0, maskG=0, maskB=0, x;
    int biHeight, flipImage;
    unsigned char colorMask[12];
    unsigned char* colorMap = NULL;

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
                          biBitCount, maskR, maskG, maskB, flipImage );
        break;
    case BI_RGB:
        if( biBitCount==8 )
            loadBMPcolormap( file, io, biWidth, biHeight, img->image_buffer,
                             colorMap, flipImage );
        else if( biBitCount==16 )
            loadBMPbitfields( file, io, biWidth, biHeight, img->image_buffer,
                              biBitCount, 0x1F, 0x3E0, 0x7C00, flipImage );
        else
            loadBMPbitfields( file, io, biWidth, biHeight, img->image_buffer,
                              biBitCount, 0xFF, 0xFF00, 0xFF0000, flipImage );
        break;
    case BI_RLE8:
        loadBMPrle( file, io, biWidth, biHeight, img->image_buffer,
                    colorMap );
        break;
    };

    free( colorMap );
    return ELR_SUCESS;
}

#endif

