#ifndef __IMAGE_HINTS_H__
#define __IMAGE_HINTS_H__



#include <cstddef>



/**
 * The CImage class supports setting 'hints' for exporters and
 * importers. These hints are numbers that can be used for
 * configuring certain exporter/importer dependend features, like,
 * for instance the quality of the jpeg exporter.
 * If an exporter or importer adds hints, they have to be added
 * to the image_hint enumerator with a unique id.
 */
enum image_hint
{
    /// The JPEG exporter quality. A size_t percentage value. Default is 75.
    IH_JPEG_EXPORT_QUALITY = 0xA000,

    /// The number of columns for the ascii art exporter(size_t). Default is 80.
    IH_ASCII_EXPORT_COLUMNS = 0xA010,

    /// If true(bool value). The ascii art exporter generates vt100 color codes. Default is false.
    IH_ASCII_EXPORT_VT100_COLORS = 0xA011
};



#endif /* __IMAGE_HINTS_H__ */

