#ifndef IMAGE_HINT_H
#define IMAGE_HINT_H

typedef enum
{
    /** \brief JPEG exporter quality. Value between 1 and 3. Default: 3 */
    EIH_JPEG_EXPORT_QUALITY = 0,

    /** \brief Not a hint, but the number of possible hints. */
    EIH_NUM_HINTS
}
E_IMAGE_HINT;

typedef size_t IMAGE_HINTS[EIH_NUM_HINTS];

#endif /* IMAGE_HINT_H */

