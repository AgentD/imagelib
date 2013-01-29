#ifndef IMAGE_HINT_H
#define IMAGE_HINT_H



typedef enum
{
    /** \brief JPEG exporter quality. Value between 0 and 100. Default: 75 */
    EIH_JPEG_EXPORT_QUALITY = 0,

    /** \brief Not a hint, but the number of possible hints. */
    EIH_NUM_HINTS
}
E_IMAGE_HINT;



typedef size_t IMAGE_HINTS[EIH_NUM_HINTS];



#endif /* IMAGE_HINT_H */

