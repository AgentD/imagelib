#ifdef IMAGE_SAVE_JPG



#include "image.h"



/*
    The JPEG exporting facilities.

    What should work:
      - exporting ECT_GRAYSCALE8 images
      - exporting ECT_RGB8 images
      - exporting ECT_RGBA8 images
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jpeglib.h"
#include "jerror.h"



#define BUF_SIZE 4096



typedef struct
{
    struct jpeg_destination_mgr jdmgr;    /* original jpeg_destination_mgr */

    void* file;                    /* pointer to target file   */
    const SFileIOInterface* io;    /* file io interface */
    JOCTET buffer[ BUF_SIZE ];     /* temporary writing buffer */
}
m_mem_destination_mgr;


static void jpeg_init_destination( j_compress_ptr cinfo )
{
    m_mem_destination_mgr* dst = (m_mem_destination_mgr*)cinfo->dest;

    dst->jdmgr.next_output_byte = dst->buffer;
    dst->jdmgr.free_in_buffer   = BUF_SIZE;
}

static boolean jpeg_empty_output_buffer( j_compress_ptr cinfo )
{
    m_mem_destination_mgr* dst = (m_mem_destination_mgr*)cinfo->dest;

    dst->io->write( dst->buffer, 1, BUF_SIZE, dst->file );

    dst->jdmgr.next_output_byte = dst->buffer;
    dst->jdmgr.free_in_buffer   = BUF_SIZE;

    return TRUE;
}

static void jpeg_term_destination( j_compress_ptr cinfo )
{
    m_mem_destination_mgr* dst = (m_mem_destination_mgr*)cinfo->dest;

    const size_t datacount = (size_t)(BUF_SIZE - dst->jdmgr.free_in_buffer);

    dst->io->write( dst->buffer, 1, datacount, dst->file );
}

static void jpeg_file_dest( j_compress_ptr cinfo, void* file,
                            const SFileIOInterface* io )
{
    m_mem_destination_mgr* dst;

    if( !cinfo->dest )
        cinfo->dest = (struct jpeg_destination_mgr*)(*cinfo->mem->alloc_small)
                      ( (j_common_ptr) cinfo, JPOOL_PERMANENT,
                        sizeof(m_mem_destination_mgr) );

    dst = (m_mem_destination_mgr*)cinfo->dest;

    dst->jdmgr.init_destination    = jpeg_init_destination;
    dst->jdmgr.empty_output_buffer = jpeg_empty_output_buffer;
    dst->jdmgr.term_destination    = jpeg_term_destination;
    dst->file                      = file;
    dst->io                        = io;
}


static void RGBtoRGB( unsigned char* src, unsigned char* dst, size_t width )
{
    memcpy( dst, src, width*3 );
}

static void gray8ToRGB( unsigned char* src, unsigned char* dst, size_t width )
{
    size_t x;

    for( x=0; x<width; ++x )
    {
        dst[ x*3     ] = src[ x ];
        dst[ x*3 + 1 ] = src[ x ];
        dst[ x*3 + 2 ] = src[ x ];
    }
}

static void rgba8ToRGB( unsigned char* src, unsigned char* dst, size_t width )
{
    size_t x;

    for( x=0; x<width; ++x )
    {
        dst[ x*3     ] = src[ x*4     ];
        dst[ x*3 + 1 ] = src[ x*4 + 1 ];
        dst[ x*3 + 2 ] = src[ x*4 + 2 ];
    }
}



void save_jpg( SImage* img, void* file, const SFileIOInterface* io )
{
    void (* convert ) (unsigned char*, unsigned char*, size_t) = NULL;

    size_t ystep = img->width;
    size_t quality = 0;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char* dst;
    unsigned char* src;

    switch( img->type )
    {
    case ECT_GRAYSCALE8: convert = gray8ToRGB;           break;
    case ECT_RGB8:       convert = RGBtoRGB;   ystep*=3; break;
    case ECT_RGBA8:      convert = rgba8ToRGB; ystep*=4; break;
    };

    quality = image_get_hint( img, EIH_JPEG_EXPORT_QUALITY );

    if( quality<1 || quality>100 )
        quality = 75;

    cinfo.err = jpeg_std_error( &jerr );

    jpeg_create_compress( &cinfo );
    jpeg_file_dest( &cinfo, file, io );

    cinfo.image_width      = img->width;
    cinfo.image_height     = img->height;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults( &cinfo );

    jpeg_set_quality( &cinfo, quality, TRUE );
    jpeg_start_compress( &cinfo, TRUE );


    dst = malloc( img->width*3 );

    if( dst )
    {
        JSAMPROW rowPtr[1];
        rowPtr[0] = dst;

        src = img->image_buffer;

        while( cinfo.next_scanline < cinfo.image_height )
        {
            convert( src, dst, img->width );
            src += ystep;
            jpeg_write_scanlines( &cinfo, rowPtr, 1 );
        }
    }

    free( dst );

    jpeg_finish_compress( &cinfo );
    jpeg_destroy_compress( &cinfo );
}

#endif

