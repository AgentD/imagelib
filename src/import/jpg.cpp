#ifdef IMAGE_LOAD_JPG



#include "image.h"



/*
    The JPEG loading facilities.

    What should work:

      - Importing JPEG images using libjpeg and storing them as RGB8 images
*/




#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

extern "C"
{
    #include "jpeglib.h"
}



// struct for handling jpeg errors
struct m_jpeg_error_mgr
{
    struct jpeg_error_mgr emgr;   // jpeg error information

    jmp_buf setjmp_buffer;        // for longjmp, to return to caller on a fatal error
};


// Override to get rid of exit behaviour
static void error_exit( j_common_ptr cinfo )
{
    m_jpeg_error_mgr* m = (m_jpeg_error_mgr*)cinfo->err;  // Retrieve custom jpeg error structure

    longjmp( m->setjmp_buffer, 1 );                       // libjpeg expects us to exit. Jump out to continue normal program flow
}

static void output_message( j_common_ptr cinfo )
{
}

static void skip_input_data( j_decompress_ptr cinfo, long count )
{
    jpeg_source_mgr* src = cinfo->src;

    if( count > 0 )
    {
        src->bytes_in_buffer -= count;
        src->next_input_byte += count;
    }
}

static void init_source( j_decompress_ptr cinfo )
{
}

static void term_source( j_decompress_ptr cinfo )
{
}

static boolean fill_input_buffer( j_decompress_ptr cinfo )
{
    return 1;
}



E_LOAD_RESULT CImage::m_loadJpg( FILE* file )
{
    // Work out how many bytes we have left to read
    fseek( file, 0, SEEK_END );
    size_t length = ftell( file );
    fseek( file, 0, SEEK_SET );

    // Declare our row pointer buffer and an input buffer where we read all remaining bytes of the file into
    unsigned char** rowPtr = NULL;
    unsigned char*  input  = new unsigned char [length];

    fread( input, 1, length, file );

    // Set up our jpeg info and jpeg error struct with our error routines
    jpeg_decompress_struct cinfo;
    m_jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr.emgr);
    cinfo.err->error_exit     = error_exit;
    cinfo.err->output_message = output_message;

    // In case of a fatal error, libjpeg calls a custom callback and expects us not ot return(i.e. to exit).
    // In order to continue normal program flow, we use longjump to jump back here.
    if( setjmp( jerr.setjmp_buffer ) )
    {
        jpeg_destroy_decompress( &cinfo );

        delete [] input;
        delete [] rowPtr;

        return ELR_FILE_CORRUPTED;
    }

    // Initialise decompression
    jpeg_source_mgr jsrc;

    jsrc.bytes_in_buffer   = length;                 // Give the library our file buffer
    jsrc.next_input_byte   = (JOCTET*)input;
    jsrc.init_source       = init_source;            // Register our callback functions
    jsrc.fill_input_buffer = fill_input_buffer;
    jsrc.skip_input_data   = skip_input_data;
    jsrc.resync_to_restart = jpeg_resync_to_restart;
    jsrc.term_source       = term_source;

    jpeg_create_decompress( &cinfo );

    cinfo.src = &jsrc;

    jpeg_read_header( &cinfo, TRUE );          // Read the jif header

    cinfo.out_color_space = JCS_RGB;           // We want to convert the image to RGB on loading
    cinfo.out_color_components = 3;
    cinfo.do_fancy_upsampling = FALSE;

    jpeg_start_decompress( &cinfo );

    // Read the image
    allocateBuffer( cinfo.image_width, cinfo.image_height, EIT_RGB8 );

    // The libjpeg wants an array of row pointers, generate one.
    rowPtr = new unsigned char* [ m_height ];

    size_t ystep = m_width*3;

    for( size_t i=0; i<m_height; ++i )
        rowPtr[ i ] = (unsigned char*)m_imageBuffer + i*ystep;

    // Read all scanlines from the file
    size_t rows = 0;

    while( cinfo.output_scanline < cinfo.output_height )
        rows += jpeg_read_scanlines( &cinfo, &rowPtr[ rows ], cinfo.output_height - rows );

    // Cleanup
    delete [] rowPtr;
    delete [] input;

    jpeg_finish_decompress ( &cinfo );
    jpeg_destroy_decompress( &cinfo );

    return ELR_SUCESS;
}

#endif

