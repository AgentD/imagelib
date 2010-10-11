#ifndef __IMAGE_LIB_UTIL_H__
#define __IMAGE_LIB_UTIL_H__


#define WRITE_LITTLE_ENDIAN_32( value, array, index )\
	array[ index   ] =  value & 0x000000FF;\
	array[ index+1 ] = (value & 0x0000FF00)>>8;\
	array[ index+2 ] = (value & 0x00FF0000)>>16;\
	array[ index+3 ] = (value & 0xFF000000)>>24

#define WRITE_LITTLE_ENDIAN_16( value, array, index ) array[ index ] = value&0x00FF; array[ index+1 ] = (value&0xFF00)>>8

namespace util
{
	template<typename T>
	void xorSwap( T& x, T& y)
	{
		x ^= y;
        y ^= x;
		x ^= y;
	}
}



#endif /* __IMAGE_LIB_UTIL_H__ */

