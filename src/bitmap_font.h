#ifndef __IMAGE_BITMAP_FONT_H__
#define __IMAGE_BITMAP_FONT_H__



/// Get the width of a single character in pixels
unsigned int getCharWidth( );

/// Get the height of a single character in pixels
unsigned int getCharHeight( );

/**
 * \brief Render a given ASCII character into an buffer
 *
 * \param c    The character to render
 * \param dst  The destination buffer to render to
 * \param x    The x coordinate of the upper left corner of the character
 *             pixmap relative to the lower left corner of the darget buffer.
 * \param y    The y coordinate of the upper left corner of the character
 *             pixmap relative to the lower left corner of the darget buffer.
 * \param dstW The width of the target buffer in pixels
 * \param dstH The height of the target buffer in pixels
 */
void getCharacter( char c, unsigned char* dst, unsigned int x, unsigned int y, unsigned int dstW, unsigned int dstH );



#endif /* __IMAGE_BITMAP_FONT_H__ */

