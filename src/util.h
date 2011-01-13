#ifndef __IMAGE_LIB_UTIL_H__
#define __IMAGE_LIB_UTIL_H__



#include <cmath>



#define WRITE_LITTLE_ENDIAN_32( value, array, index )\
	array[ index   ] =  value & 0x000000FF;\
	array[ index+1 ] = (value & 0x0000FF00)>>8;\
	array[ index+2 ] = (value & 0x00FF0000)>>16;\
	array[ index+3 ] = (value & (size_t)0xFF000000)>>24

#define WRITE_LITTLE_ENDIAN_16( value, array, index ) array[ index ] = value&0x00FF; array[ index+1 ] = (value&0xFF00)>>8

namespace util
{
   inline void rgb2rgbe( unsigned char* rgbe, float r, float g, float b )
   {
      float v = r>g ? (r>b ? r : b) : (g>b ? g : b);

      if( v<1e-32 )
      {
         rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
      }
      else
      {
         int e;

         v = frexp( v, &e ) * 256.0/v;

         rgbe[0] = (unsigned char)(r * v);
         rgbe[1] = (unsigned char)(g * v);
         rgbe[2] = (unsigned char)(b * v);
         rgbe[3] = (unsigned char)(e + 128);
      }
   }

   inline void rgbe2rgb( unsigned char* rgbe, float& r, float& g, float& b )
   {
      if( rgbe[3] )
      {
         float f = ldexp( 1.0, rgbe[3]-(int)(128+8) );

         r = rgbe[0] * f;
         g = rgbe[1] * f;
         b = rgbe[2] * f;
      }
      else
         r = g = b = 0.0;
   }

   template<typename T, size_t maxColorValue>
   size_t getNearestPaletteIndex( const T* palette, size_t size, T r, T g, T b )
   {
      if( !palette ) return 0;

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
   void average( T R1, T G1, T B1, T R2, T G2, T B2, T& Rout, T& Gout, T& Bout )
   {
      Rout = (R1 + R2) >> 1;
      Gout = (G1 + G2) >> 1;
      Bout = (B1 + B2) >> 1;
   }

   template<typename T>
   T luminance( T R, T G, T B )
   {
      return 0.2126*R + 0.7152*G + 0.0722*B;
   }

   template<typename T>
   void xorSwap( T& x, T& y)
   {
      x ^= y;
      y ^= x;
      x ^= y;
   }
}



#endif /* __IMAGE_LIB_UTIL_H__ */

