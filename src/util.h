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

   template<typename T>
   void xorSwap( T& x, T& y)
   {
      x ^= y;
      y ^= x;
      x ^= y;
   }
}



#endif /* __IMAGE_LIB_UTIL_H__ */

