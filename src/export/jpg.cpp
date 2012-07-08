#ifdef IMAGE_SAVE_JPG



#include "image.h"
#include "util.h"



/*
    The JPEG exporting facilities.

    What should work('-' means implemented but not tested,
                     'x' means tested and DOES work):
      x exporting EIT_GRAYSCALE8 images
      x exporting EIT_RGB8 images
      x exporting EIT_RGBA8 images
*/


#include <stdio.h>

extern "C"
{
    #include "jpeglib.h"
    #include "jerror.h"
}



namespace
{
    const size_t BUF_SIZE = 4096;

    struct m_mem_destination_mgr
    {
        jpeg_destination_mgr jdmgr;    // original jpeg_destination_mgr

        std::ostream* file;            // pointer to target file
        JOCTET buffer[ BUF_SIZE ];     // temporary writing buffer
    };


    void jpeg_init_destination( j_compress_ptr cinfo )
    {
        m_mem_destination_mgr* dst = (m_mem_destination_mgr*)cinfo->dest;

        dst->jdmgr.next_output_byte = dst->buffer;
        dst->jdmgr.free_in_buffer   = BUF_SIZE;
    }

    boolean jpeg_empty_output_buffer( j_compress_ptr cinfo )
    {
        m_mem_destination_mgr* dst = (m_mem_destination_mgr*)cinfo->dest;

        dst->file->write((char*)dst->buffer, BUF_SIZE);

        dst->jdmgr.next_output_byte = dst->buffer;
        dst->jdmgr.free_in_buffer   = BUF_SIZE;

        return TRUE;
    }

    void jpeg_term_destination( j_compress_ptr cinfo )
    {
        m_mem_destination_mgr* dst = (m_mem_destination_mgr*)cinfo->dest;

        const size_t datacount = (size_t)(BUF_SIZE - dst->jdmgr.free_in_buffer);

        dst->file->write((char*)dst->buffer, datacount);
    }

    void jpeg_file_dest( j_compress_ptr cinfo, std::ostream* file )
    {
        if( !cinfo->dest )
            cinfo->dest = (jpeg_destination_mgr*)(*cinfo->mem->alloc_small)( (j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(m_mem_destination_mgr) );

        m_mem_destination_mgr* dst = (m_mem_destination_mgr*)cinfo->dest;

        dst->jdmgr.init_destination    = jpeg_init_destination;
        dst->jdmgr.empty_output_buffer = jpeg_empty_output_buffer;
        dst->jdmgr.term_destination    = jpeg_term_destination;
        dst->file                      = file;
    }


    void RGBtoRGB( unsigned char* src, unsigned char* dst, size_t width )
    {
        for( size_t x=0; x<width*3; ++x )
            dst[ x ] = src[ x ];
    }

    void gray8ToRGB( unsigned char* src, unsigned char* dst, size_t width )
    {
        for( size_t x=0; x<width; ++x )
        {
            dst[ x*3     ] = src[ x ];
            dst[ x*3 + 1 ] = src[ x ];
            dst[ x*3 + 2 ] = src[ x ];
        }
    }

    void rgba8ToRGB( unsigned char* src, unsigned char* dst, size_t width )
    {
        for( size_t x=0; x<width; ++x )
        {
            dst[ x*3     ] = src[ x*4     ];
            dst[ x*3 + 1 ] = src[ x*4 + 1 ];
            dst[ x*3 + 2 ] = src[ x*4 + 2 ];
        }
    }
}



void CImage::m_saveJpg( std::ostream& file )
{
    void(*convert)(unsigned char*, unsigned char*, size_t) = NULL;

    size_t ystep = m_width;

    switch( m_type )
    {
    case EIT_GRAYSCALE8: convert = gray8ToRGB;           break;
    case EIT_RGB8:       convert = RGBtoRGB;   ystep*=3; break;
    case EIT_RGBA8:      convert = rgba8ToRGB; ystep*=4; break;
    };


    size_t quality = getHint<size_t>( IH_JPEG_EXPORT_QUALITY );

    if( quality<1 || quality>100 )
        quality = 75;

    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error( &jerr );

    jpeg_create_compress( &cinfo );
    jpeg_file_dest( &cinfo, &file );

    cinfo.image_width      = m_width;
    cinfo.image_height     = m_height;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults( &cinfo );

    jpeg_set_quality( &cinfo, quality, TRUE );
    jpeg_start_compress( &cinfo, TRUE );


    unsigned char* dst = new unsigned char[ m_width*3 ];

    if( dst )
    {
        JSAMPROW rowPtr[1];
        rowPtr[0] = dst;

        unsigned char* src = (unsigned char*)m_imageBuffer;

        while( cinfo.next_scanline < cinfo.image_height )
        {
            convert( src, dst, m_width );
            src += ystep;
            jpeg_write_scanlines( &cinfo, rowPtr, 1 );
        }
    }

    delete [] dst;

    jpeg_finish_compress( &cinfo );
    jpeg_destroy_compress( &cinfo );
}

#endif

