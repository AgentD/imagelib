#ifndef IMAGE_LIB_UTIL_H
#define IMAGE_LIB_UTIL_H



#define WRITE_LITTLE_ENDIAN_32( value, array, index )\
	array[ index   ] =  value      & 0xFF;\
	array[ index+1 ] = (value>>8 ) & 0xFF;\
	array[ index+2 ] = (value>>16) & 0xFF;\
	array[ index+3 ] = (value>>24) & 0xFF

#define WRITE_LITTLE_ENDIAN_16( value, array, index ) \
    array[ index   ] =  value     & 0xFF; \
    array[ index+1 ] = (value>>8) & 0xFF

#define READ_LITTLE_ENDIAN_32( array, index )\
    (((size_t)(array)[(index)  ])     | ((size_t)(array)[(index)+1])<<8 |\
     ((size_t)(array)[(index)+2])<<16 | ((size_t)(array)[(index)+3])<<24)

#define READ_LITTLE_ENDIAN_16( array, index )\
    (((size_t)(array)[(index)]) | ((size_t)(array)[(index)+1])<<8)



#define READ_BIG_ENDIAN_16( array, index )\
    (((size_t)(array)[(index)+1]) | ((size_t)(array)[(index)])<<8)



#define LUMINANCE( R, G, B ) (0.2126*(R) + 0.7152*(G) + 0.0722*(B))



#endif /* IMAGE_LIB_UTIL_H */

