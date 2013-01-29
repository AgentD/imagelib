#ifndef IMAGE_EDIT_H
#define IMAGE_EDIT_H



#include "image.h"



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Clear an image to a color
 *
 * \param img The image to clear
 * \param R   The red component of the clear color
 * \param G   The green component of the clear color
 * \param B   The blue component of the clear color
 * \param A   The alpha component of the clear color
 */
void image_clear( SImage* img, float R, float G, float B, float A );

/**
 * \brief Place a pixel onto the image
 *
 * \param img The image to write to
 * \param x   The x coordinate of the pixel to place(0=left, width-1=right)
 * \param y   The y coordinate of the pixel to place(0=top, height-1=bottom)
 * \param R   The red component of the pixel olor
 * \param G   The green component of the pixel color
 * \param B   The blue component of the pixel color
 * \param A   The alpha component of the pixel color
 */
void image_set_pixel( SImage* img, size_t x, size_t y,
                      float R, float G, float B, float A );

#ifdef __cplusplus
}
#endif



#endif /* IMAGE_EDIT_H */

