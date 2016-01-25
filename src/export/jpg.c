/*
    The JPEG exporting facilities.

    Based on "Tiny JPEG", a small, public domain JPEG encoder
    by Sergio Gonzalez.

    What should work:
      - exporting ECT_GRAYSCALE8 images
      - exporting ECT_RGB8 images
      - exporting ECT_RGBA8 image
*/

#include "image.h"
#include "util.h"

#ifdef IMAGE_SAVE_JPG
#include <string.h>
#include <math.h>

#define HUFF_DC 0
#define HUFF_AC 1

#define LUMA_DC 0
#define LUMA_AC 1
#define CHROMA_DC 2
#define CHROMA_AC 3

struct enc_state
{
    uint8_t ehuffsize[4][257];
    uint16_t ehuffcode[4][256];

    uint8_t* ht_bits[4];
    uint8_t* ht_vals[4];

    uint8_t qt_luma[64];
    uint8_t qt_chroma[64];

    const SFileIOInterface* io;
    void* fd;
};

static uint8_t default_qt_luma[64] =
{
    16,11,10,16, 24, 40, 51, 61,
    12,12,14,19, 26, 58, 60, 55,
    14,13,16,24, 40, 57, 69, 56,
    14,17,22,29, 51, 87, 80, 62,
    18,22,37,56, 68,109,103, 77,
    24,35,55,64, 81,104,113, 92,
    49,64,78,87,103,121,120,101,
    72,92,95,98,112,100,103, 99,
};

static uint8_t default_qt_chroma[64] =
{
    16,  12, 14,  14, 18, 24,  49,  72,
    11,  10, 16,  24, 40, 51,  61,  12,
    13,  17, 22,  35, 64, 92,  14,  16,
    22,  37, 55,  78, 95, 19,  24,  29,
    56,  64, 87,  98, 26, 40,  51,  68,
    81, 103, 112, 58, 57, 87,  109, 104,
    121,100, 60,  69, 80, 103, 113, 120,
    103, 55, 56,  62, 77, 92,  101, 99,
};

static uint8_t default_ht_luma_dc_len[16] =
{
    0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0
};

static uint8_t default_ht_luma_dc[12] =
{
    0,1,2,3,4,5,6,7,8,9,10,11
};

static uint8_t default_ht_chroma_dc_len[16] =
{
    0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0
};

static uint8_t default_ht_chroma_dc[12] =
{
    0,1,2,3,4,5,6,7,8,9,10,11
};

static uint8_t default_ht_luma_ac_len[16] =
{
    0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0x7d
};

static uint8_t default_ht_luma_ac[162] =
{
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06,
    0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08,
    0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72,
    0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75,
    0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3,
    0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
    0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9,
    0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2,
    0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4,
    0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA
};

static uint8_t default_ht_chroma_ac_len[16] =
{
    0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0x77
};

static uint8_t default_ht_chroma_ac[162] =
{
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41,
    0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1,
    0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26,
    0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A,
    0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
    0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
    0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4,
    0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA
};

static uint8_t zig_zag[64] =
{
   0,   1,  5,  6, 14, 15, 27, 28,
   2,   4,  7, 13, 16, 26, 29, 42,
   3,   8, 12, 17, 25, 30, 41, 43,
   9,  11, 18, 24, 31, 40, 44, 53,
   10, 19, 23, 32, 39, 45, 52, 54,
   20, 22, 33, 38, 46, 51, 55, 60,
   21, 34, 37, 47, 50, 56, 59, 61,
   35, 36, 48, 49, 57, 58, 62, 63,
};

static const float aan_scales[8] =
{
    1.0f, 1.387039845f, 1.306562965f, 1.175875602f,
    1.0f, 0.785694958f, 0.541196100f, 0.275899379f
};

static void write_DQT(struct enc_state* state, uint8_t* matrix, uint8_t id)
{
    unsigned char buffer[5];

    WRITE_BIG_ENDIAN_16( 0xFFDB, buffer, 0 );   /* DQT */
    WRITE_BIG_ENDIAN_16( 0x0043, buffer, 2 );   /* length */
    buffer[4] = id;                             /* quality id */

    state->io->write(buffer, 1,  5, state->fd);
    state->io->write(matrix, 1, 64, state->fd);
}

static void write_DHT(struct enc_state* state, uint8_t* matrix_len,
                      uint8_t* matrix_val, int ht_class, uint8_t id)
{
    int i, num_values = 0, len;
    unsigned char buffer[5];

    for(i=0; i<16; ++i)
        num_values += matrix_len[i];

    len = num_values + 19;

    WRITE_BIG_ENDIAN_16( 0xFFC4, buffer, 0 );   /* DHT */
    WRITE_BIG_ENDIAN_16( len,    buffer, 2 );   /* length */
    buffer[4] = (((ht_class & 0xFF) << 4) | id) & 0xFF;

    state->io->write(buffer,     1,          5, state->fd);
    state->io->write(matrix_len, 1,         16, state->fd);
    state->io->write(matrix_val, 1, num_values, state->fd);
}

/************************** Huffman deflation code **************************/
static void huff_get_code_lengths(uint8_t* huffsize, uint8_t* bits)
{
    int i, j, k = 0;
    for( i=0; i<16; ++i )
    {
        for( j=0; j<bits[i]; ++j )
            huffsize[k++] = i + 1;
        huffsize[k] = 0;
    }
}

static void huff_get_codes(uint16_t* codes, uint8_t* huffsize)
{
    uint8_t sz = huffsize[0];
    uint16_t code = 0;
    int k = 0;

    while(1)
    {
        do { codes[k++] = code++; } while(huffsize[k] == sz);

        if( huffsize[k] == 0 )
            return;

        do { code<<=1; ++sz; } while(huffsize[k] != sz);
    }
}

static void huff_get_extended(uint8_t* out_ehuffsize, uint16_t* out_ehuffcode,
                              uint8_t* huffval, uint8_t* huffsize,
                              uint16_t* huffcode, int64_t count)
{
    uint8_t val;
    int k = 0;

    do
    {
        val = huffval[k];
        out_ehuffcode[val] = huffcode[k];
        out_ehuffsize[val] = huffsize[k];
        ++k;
    }
    while(k < count);
}
/****************************************************************************/
static void calculate_variable_length_int(int value, uint16_t* out)
{
    int abs_val = value;

    if( value < 0 )
    {
        abs_val = -abs_val;
        --value;
    }

    out[1] = 1;

    while( (abs_val >>= 1) )
        out[1] += 1;

    out[0] = value & ((1 << out[1]) - 1);
}

static void write_bits(struct enc_state* state, uint32_t* bitbuffer,
                       uint32_t* location, uint16_t num_bits, uint16_t bits)
{
    uint32_t nloc = *location + num_bits;
    uint8_t c;

    *bitbuffer |= (uint32_t)(bits << (32 - nloc));

    for( *location=nloc; *location>=8; *location-=8 )
    {
        c = (*bitbuffer) >> 24;
        state->io->write(&c, 1, 1, state->fd);

        if( c==0xFF )
        {
            c = 0;
            state->io->write(&c, 1, 1, state->fd);
        }

        *bitbuffer <<= 8;
    }
}

/*
    DCT implementation by Thomas G. Lane.
    Obtained through NVIDIA
        http://developer.download.nvidia.com/SDK/9.5/
        Samples/vidimaging_samples.html#gpgpu_dct

    QUOTE:
      This implementation is based on Arai, Agui, and Nakajima's algorithm for
      scaled DCT.  Their original paper (Trans. IEICE E-71(11):1095) is in
      Japanese, but the algorithm is described in the Pennebaker & Mitchell
      JPEG textbook (see REFERENCES section in file README). The following
      code is based directly on figure 4-8 in P&M.
 */
static void nv_fdct(float* data)
{
    float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp10, tmp11;
    float tmp12, tmp13, z1, z2, z3, z4, z5, z11, z13, *dataptr;
    int ctr;

    /* Pass 1: process rows. */
    for( ctr=7, dataptr=data; ctr>=0; --ctr, dataptr+=8 )
    {
        tmp0 = dataptr[0] + dataptr[7];
        tmp7 = dataptr[0] - dataptr[7];
        tmp1 = dataptr[1] + dataptr[6];
        tmp6 = dataptr[1] - dataptr[6];
        tmp2 = dataptr[2] + dataptr[5];
        tmp5 = dataptr[2] - dataptr[5];
        tmp3 = dataptr[3] + dataptr[4];
        tmp4 = dataptr[3] - dataptr[4];

        /* Even part */
        tmp10 = tmp0 + tmp3;                    /* phase 2 */
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        dataptr[0] = tmp10 + tmp11;             /* phase 3 */
        dataptr[4] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13) * 0.707106781f;    /* c4 */
        dataptr[2] = tmp13 + z1;                /* phase 5 */
        dataptr[6] = tmp13 - z1;

        /* Odd part */
        tmp10 = tmp4 + tmp5;                    /* phase 2 */
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        /* The rotator is modified from fig 4-8 to avoid extra negations. */
        z5 = (tmp10 - tmp12) * 0.382683433f;    /* c6 */
        z2 = 0.541196100f * tmp10 + z5;         /* c2-c6 */
        z4 = 1.306562965f * tmp12 + z5;         /* c2+c6 */
        z3 = tmp11 * 0.707106781f;              /* c4 */

        z11 = tmp7 + z3;                        /* phase 5 */
        z13 = tmp7 - z3;

        dataptr[5] = z13 + z2;                  /* phase 6 */
        dataptr[3] = z13 - z2;
        dataptr[1] = z11 + z4;
        dataptr[7] = z11 - z4;
    }

    /* Pass 2: process columns. */
    for( ctr=8-1, dataptr=data; ctr>=0; --ctr, ++dataptr )
    {
        tmp0 = dataptr[8*0] + dataptr[8*7];
        tmp7 = dataptr[8*0] - dataptr[8*7];
        tmp1 = dataptr[8*1] + dataptr[8*6];
        tmp6 = dataptr[8*1] - dataptr[8*6];
        tmp2 = dataptr[8*2] + dataptr[8*5];
        tmp5 = dataptr[8*2] - dataptr[8*5];
        tmp3 = dataptr[8*3] + dataptr[8*4];
        tmp4 = dataptr[8*3] - dataptr[8*4];

        /* Even part */
        tmp10 = tmp0 + tmp3;                    /* phase 2 */
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        dataptr[8*0] = tmp10 + tmp11;           /* phase 3 */
        dataptr[8*4] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13) * 0.707106781f;    /* c4 */
        dataptr[8*2] = tmp13 + z1;              /* phase 5 */
        dataptr[8*6] = tmp13 - z1;

        /* Odd part */
        tmp10 = tmp4 + tmp5;                    /* phase 2 */
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        /* The rotator is modified from fig 4-8 to avoid extra negations. */
        z5 = (tmp10 - tmp12) * 0.382683433f;    /* c6 */
        z2 = 0.541196100f * tmp10 + z5;         /* c2-c6 */
        z4 = 1.306562965f * tmp12 + z5;         /* c2+c6 */
        z3 = tmp11 * 0.707106781f;              /* c4 */

        z11 = tmp7 + z3;                        /* phase 5 */
        z13 = tmp7 - z3;

        dataptr[8*5] = z13 + z2;                /* phase 6 */
        dataptr[8*3] = z13 - z2;
        dataptr[8*1] = z11 + z4;
        dataptr[8*7] = z11 - z4;
    }
}

static void encode_and_write_MCU(struct enc_state* state, float* mcu,
                                 float* qt, uint8_t* huff_dc_len,
                                 uint16_t* huff_dc_code, uint8_t* huff_ac_len,
                                 uint16_t* huff_ac_code, int* pred,
                                 uint32_t* bitbuffer, uint32_t* location)
{
    int i, val, diff, zero_count, last_non_zero_i = 0, du[64];
    float fval, dct_mcu[64];
    uint16_t sym1, vli[2];

    memcpy(dct_mcu, mcu, 64*sizeof(float));

    nv_fdct(dct_mcu);
    for(i=0; i<64; ++i)
    {
        fval = dct_mcu[i];
        fval *= qt[i];
        fval = floor(fval + 1024 + 0.5f);
        fval -= 1024;
        val = (int)fval;
        du[zig_zag[i]] = val;
    }

    diff = du[0] - *pred;
    *pred = du[0];
    if(diff != 0)
    {
        calculate_variable_length_int(diff, vli);
        write_bits(state, bitbuffer, location,
                   huff_dc_len[vli[1]], huff_dc_code[vli[1]]);
        write_bits(state, bitbuffer, location, vli[1], vli[0]);
    }
    else
    {
        write_bits(state,bitbuffer,location,huff_dc_len[0],huff_dc_code[0]);
    }

    for(i=63; i>0; --i)
    {
        if(du[i] != 0)
        {
            last_non_zero_i = i;
            break;
        }
    }

    for(i=1; i<=last_non_zero_i; ++i)
    {
        zero_count = 0;
        while(du[i] == 0)
        {
            ++zero_count;
            ++i;
            if(zero_count == 16)
            {
                write_bits(state, bitbuffer, location,
                           huff_ac_len[0xf0], huff_ac_code[0xf0]);
                zero_count = 0;
            }
        }
        calculate_variable_length_int(du[i], vli);
        sym1 = (uint16_t)((uint16_t)zero_count << 4) | vli[1];

        write_bits(state, bitbuffer, location,
                   huff_ac_len[sym1], huff_ac_code[sym1]);
        write_bits(state, bitbuffer, location, vli[1], vli[0]);
    }

    if( last_non_zero_i != 63 )
        write_bits(state,bitbuffer,location,huff_ac_len[0],huff_ac_code[0]);
}

static void huff_expand(struct enc_state* state)
{
    int32_t spec_tables_len[4] = { 0 };
    uint16_t huffcode[4][256];
    uint8_t huffsize[4][257];
    int64_t count;
    int i, k;

    state->ht_bits[LUMA_DC]   = default_ht_luma_dc_len;
    state->ht_bits[LUMA_AC]   = default_ht_luma_ac_len;
    state->ht_bits[CHROMA_DC] = default_ht_chroma_dc_len;
    state->ht_bits[CHROMA_AC] = default_ht_chroma_ac_len;
    state->ht_vals[LUMA_DC]   = default_ht_luma_dc;
    state->ht_vals[LUMA_AC]   = default_ht_luma_ac;
    state->ht_vals[CHROMA_DC] = default_ht_chroma_dc;
    state->ht_vals[CHROMA_AC] = default_ht_chroma_ac;

    for( i=0; i<4; ++i )
    {
        for( k=0; k<16; ++k )
            spec_tables_len[i] += state->ht_bits[i][k];
    }
    for( i=0; i<4; ++i )
    {
        huff_get_code_lengths(huffsize[i], state->ht_bits[i]);
        huff_get_codes(huffcode[i], huffsize[i]);
    }
    for( i=0; i<4; ++i )
    {
        count = spec_tables_len[i];
        huff_get_extended(state->ehuffsize[i], state->ehuffcode[i],
                          state->ht_vals[i], &huffsize[i][0],
                          &huffcode[i][0], count);
    }
}

static void encode_main(struct enc_state* state, const unsigned char* img,
                        int width, int height, int components)
{
    int x, y, i, off_y, off_x, col, row, block_index, src_index;
    float du_y[64], du_b[64], du_r[64], luma, cb, cr, r, g, b;
    int realcomponents = components>=3 ? 3 : 1;
    int pred_y = 0, pred_b = 0, pred_r = 0;
    uint32_t bitbuffer = 0, location = 0;
    float pqt_chroma[64], pqt_luma[64];
    unsigned char buffer[20];

    for(y=0; y<8; ++y)
    {
        for(x=0; x<8; ++x)
        {
            i = y*8 + x;
            cb = 8*aan_scales[x]*aan_scales[y]*state->qt_luma[zig_zag[i]];
            cr = 8*aan_scales[x]*aan_scales[y]*state->qt_chroma[zig_zag[i]];
            pqt_luma[y*8+x] = 1.0f / cb;
            pqt_chroma[y*8+x] = 1.0f / cr;
        }
    }

    /* write header */
    WRITE_BIG_ENDIAN_16( 0xFFD8, buffer,  0 );  /* SOI */
    WRITE_BIG_ENDIAN_16( 0xFFE0, buffer,  2 );  /* APP0 */
    WRITE_BIG_ENDIAN_16(     16, buffer,  4 );  /* length */
    WRITE_BIG_ENDIAN_16( 0x0102, buffer, 11 );  /* version */
    WRITE_BIG_ENDIAN_16( 0x0060, buffer, 14 );  /* 96 DPI in x direction */
    WRITE_BIG_ENDIAN_16( 0x0060, buffer, 16 );  /* 96 DPI in y direction */
    memcpy( buffer+6, "JFIF", 5 );
    buffer[13] = 0x01;                          /* units = DPI */
    buffer[18] = 0;                             /* thumbnail x size */
    buffer[19] = 0;                             /* thumbnail y size */

    state->io->write(buffer, 1, 20, state->fd);

    write_DQT(state, state->qt_luma, 0x00);
    write_DQT(state, state->qt_chroma, 0x01);

    WRITE_BIG_ENDIAN_16( 0xFFC0, buffer, 0 );           /* SOF */
    WRITE_BIG_ENDIAN_16( 8+3*realcomponents, buffer, 2 );
    WRITE_BIG_ENDIAN_16( height, buffer, 5 );
    WRITE_BIG_ENDIAN_16( width, buffer, 7 );
    buffer[4] = 8;                                      /* precision */
    buffer[9] = realcomponents;                         /* components */

    buffer[10] = 1;         /* ID of first component */
    buffer[11] = 0x11;      /* sampling factors */
    buffer[12] = 0;         /* quantiazation table selector */

    if( components >= 3 )
    {
        buffer[13] = 2;     /* second component */
        buffer[14] = 0x11;
        buffer[15] = 1;
        buffer[16] = 3;     /* third component */
        buffer[17] = 0x11;
        buffer[18] = 1;

        state->io->write(buffer, 1, 19, state->fd);
    }
    else
    {
        state->io->write(buffer, 1, 13, state->fd);
    }

    write_DHT(state, state->ht_bits[LUMA_DC],
              state->ht_vals[LUMA_DC], HUFF_DC, 0);
    write_DHT(state, state->ht_bits[LUMA_AC],
              state->ht_vals[LUMA_AC], HUFF_AC, 0);
    write_DHT(state, state->ht_bits[CHROMA_DC],
              state->ht_vals[CHROMA_DC], HUFF_DC, 1);
    write_DHT(state, state->ht_bits[CHROMA_AC],
              state->ht_vals[CHROMA_AC], HUFF_AC, 1);

    /* Write start of scan */
    WRITE_BIG_ENDIAN_16( 0xFFDA, buffer, 0 );           /* SOS */
    WRITE_BIG_ENDIAN_16( 6 + realcomponents*2, buffer, 2 );
    buffer[ 4] = realcomponents;
    buffer[ 5] = 1;                                     /* first component */
    buffer[ 6] = 0x00;                                  /* (dc|ac) */

    if( components >= 3 )
    {
        buffer[ 7] = 2;     /* second component */
        buffer[ 8] = 0x11;
        buffer[ 9] = 3;     /* third component */
        buffer[10] = 0x11;  /* (dc|ac) */
        buffer[11] = 0;     /* first */
        buffer[12] = 63;    /* last */
        buffer[13] = 0;     /* (ah|al) */

        state->io->write(buffer, 1, 14, state->fd);
    }
    else
    {
        buffer[7] = 0;     /* first */
        buffer[8] = 63;    /* last */
        buffer[9] = 0;     /* (ah|al) */

        state->io->write(buffer, 1, 10, state->fd);
    }

    for(y=0; y<height; y+=8)
    {
        for(x=0; x<width; x+=8)
        {
            for(off_y=0; off_y<8; ++off_y)
            {
                for(off_x=0; off_x<8; ++off_x)
                {
                    block_index = (off_y * 8 + off_x);
                    src_index = (((y+off_y)*width) + (x+off_x))*components;

                    col = x + off_x;
                    row = y + off_y;

                    if(row >= height)
                        src_index -= (width * (row - height + 1))*components;

                    if(col >= width)
                        src_index -= (col - width + 1) * components;

                    if( components < 3 )
                    {
                        du_y[block_index] = (int)img[src_index] - 128;
                    }
                    else
                    {
                        r = img[src_index    ];
                        g = img[src_index + 1];
                        b = img[src_index + 2];

                        luma =  0.2990f*r + 0.5870f*g + 0.1140f*b - 128;
                        cb   = -0.1687f*r - 0.3313f*g + 0.5000f*b;
                        cr   =  0.5000f*r - 0.4187f*g - 0.0813f*b;

                        du_y[block_index] = luma;
                        du_b[block_index] = cb;
                        du_r[block_index] = cr;
                    }
                }
            }

            encode_and_write_MCU(state, du_y, pqt_luma,
                                 state->ehuffsize[LUMA_DC],
                                 state->ehuffcode[LUMA_DC],
                                 state->ehuffsize[LUMA_AC],
                                 state->ehuffcode[LUMA_AC],
                                 &pred_y, &bitbuffer, &location);
            if( components>=3 )
            {
                encode_and_write_MCU(state, du_b, pqt_chroma,
                                     state->ehuffsize[CHROMA_DC],
                                     state->ehuffcode[CHROMA_DC],
                                     state->ehuffsize[CHROMA_AC],
                                     state->ehuffcode[CHROMA_AC],
                                     &pred_b, &bitbuffer, &location);
                encode_and_write_MCU(state, du_r, pqt_chroma,
                                     state->ehuffsize[CHROMA_DC],
                                     state->ehuffcode[CHROMA_DC],
                                     state->ehuffsize[CHROMA_AC],
                                     state->ehuffcode[CHROMA_AC],
                                     &pred_r, &bitbuffer, &location);
            }
        }
    }

    if( location > 0 && location < 8 )
        write_bits(state, &bitbuffer, &location, 8 - location, 0);

    WRITE_BIG_ENDIAN_16( 0xFFD9, buffer, 0 );   /* EOI */
    state->io->write(buffer, 1, 2, state->fd);
}

void save_jpg( SImage* img, void* file, const SFileIOInterface* io )
{
    struct enc_state state = { 0 };
    int i, components, quality;
    uint8_t qt_factor = 1;

    switch( img->type )
    {
    case ECT_GRAYSCALE8: components = 1; break;
    case ECT_RGB8:       components = 3; break;
    case ECT_RGBA8:      components = 4; break;
    default:             return;
    }

    if( img->width>0xFFFF || img->height>0xFFFF )
        return;

    quality = image_get_hint( img, EIH_JPEG_EXPORT_QUALITY );
    if( quality<1 || quality>3 )
        quality = 3;

    switch( quality )
    {
    case 3:
        for( i=0; i<64; ++i )
        {
            state.qt_luma[i]   = 1;
            state.qt_chroma[i] = 1;
        }
        break;
    case 2:
        qt_factor = 10;
    case 1:
        for( i=0; i<64; ++i )
        {
            state.qt_luma[i] = default_qt_luma[i] / qt_factor;
            if( state.qt_luma[i] == 0 )
                state.qt_luma[i] = 1;
            state.qt_chroma[i] = default_qt_chroma[i] / qt_factor;
            if( state.qt_chroma[i] == 0 )
                state.qt_chroma[i] = 1;
        }
        break;
    }

    state.fd = file;
    state.io = io;

    huff_expand(&state);
    encode_main(&state,img->image_buffer,img->width,img->height,components);
}

#endif

