#ifndef IMAGE_BITMAP_FONT_H
#define IMAGE_BITMAP_FONT_H

#ifdef IMAGE_BITMAP_FONT

#ifdef __cplusplus
extern "C"
{
#endif

    /** Get the width of a single character in pixels */
    unsigned int getCharWidth( );

    /** Get the height of a single character in pixels */
    unsigned int getCharHeight( );

    /**
     * \brief Render a given ASCII character into a buffer
     *
     * \param c    The character to render
     * \param dst  The destination buffer to render to
     * \param x    The x coordinate of the upper left corner of the character
     *             pixmap relative to the upper left corner of the target buffer.
     * \param y    The y coordinate of the upper left corner of the character
     *             pixmap relative to the upper left corner of the target buffer.
     * \param dstW The width of the target buffer in pixels
     * \param dstH The height of the target buffer in pixels
     */
    void getCharacter( char c, unsigned char* dst,
                       unsigned int x, unsigned int y,
                       unsigned int dstW, unsigned int dstH );

#ifdef __cplusplus
}
#endif

#endif

#endif /* IMAGE_BITMAP_FONT_H */

