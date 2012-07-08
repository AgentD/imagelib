#ifndef __IMAGE_LIB_UTIL_H__
#define __IMAGE_LIB_UTIL_H__



#include <cmath>



#define WRITE_LITTLE_ENDIAN_32( value, array, index )\
	array[ index   ] =  value      & 0xFF;\
	array[ index+1 ] = (value>>8 ) & 0xFF;\
	array[ index+2 ] = (value>>16) & 0xFF;\
	array[ index+3 ] = (value>>24) & 0xFF

#define WRITE_LITTLE_ENDIAN_16( value, array, index ) \
    array[ index   ] =  value     & 0xFF; \
    array[ index+1 ] = (value>>8) & 0xFF

namespace util
{
    template<typename T, size_t maxColorValue>
    size_t getNearestPaletteIndex( const T* palette, size_t size,
                                   T r, T g, T b )
    {
        if( !palette )
            return 0;

        float  dist = maxColorValue*maxColorValue*3;
        size_t best = 0;

        for( size_t i=0; i<size; ++i, palette+=3 )
        {
            float dR = palette[0] - r;
            float dG = palette[1] - g;
            float dB = palette[2] - b;

            float curDist = (dR*dR) + (dG*dG) + (dB*dB);

            if( curDist<dist )
            {
                best = i;
                dist = curDist;
            }
        }

        return best;
    }

    template<typename T>
    T luminance( T R, T G, T B )
    {
        return 0.2126*R + 0.7152*G + 0.0722*B;
    }
}



#endif /* __IMAGE_LIB_UTIL_H__ */

