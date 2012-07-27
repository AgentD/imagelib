#ifndef IMAGE_HINT_H
#define IMAGE_HINT_H



typedef enum
{
    /**
     * \brief A non-zero value indicates that the ASCII exporter should use
     *        VT100 color codes.
     */
    EIH_ASCII_EXPORT_VT100_COLORS = 0,

    /** \brief Number of columns the ASCII exporter should use. Default: 79 */
    EIH_ASCII_EXPORT_COLUMNS,

    /** \brief JPEG exporter quality. Value between 0 and 100. Default: 75 */
    EIH_JPEG_EXPORT_QUALITY,

    /** \brief Not a hint, but the number of possible hints. */
    EIH_NUM_HINTS
}
E_IMAGE_HINT;



typedef size_t IMAGE_HINTS[EIH_NUM_HINTS];



#endif /* IMAGE_HINT_H */

