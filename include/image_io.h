#ifndef IMAGE_IO_H
#define IMAGE_IO_H



#include <stddef.h>



#ifdef __cplusplus
extern "C"
{
#endif



typedef struct
{
    /**
     * \brief Callback for reading data from a file
     *
     * \param ptr    Buffer for the data read
     * \param size   Size of one block to read
     * \param blocks Number of blocks to read
     * \param handle An opaque file handle
     *
     * \return The actual number of blocks read successfully
     */
    size_t (* read )( void* ptr, size_t size, size_t blocks,
                      void* handle );

    /**
     * \brief Callback for writing data to a file
     *
     * \param ptr    Buffer for the data read
     * \param size   Size of one block to read
     * \param blocks Number of blocks to read
     * \param handle An opaque file handle
     *
     * \return The actual number of blocks read successfully
     */
    size_t (* write)( const void* ptr, size_t size, size_t blocks,
                      void* handle );

    /**
     * \brief Move the file position indicator blabla... implements fseek
     *
     * \param file   An opaque file handle
     * \param offset Offset from either beginning, current or end
     * \param whence Either SEEK_SET for beginning, SEEK_CUR for current or
     *               SEEK_END for end.
     *
     * \return 0 on success
     */
    int (* seek )( void* file, long offset, int whence );

    /**
     * \brief Get the file position blabla... implements ftell
     *
     * \param file An opaque file handle
     *
     * \return Offset from the beginning of the file
     */
    long (* tell )( void* file );

    /** \brief Returns non-zero if the end of file has been hit */
    int (* eof )( void* file );
}
SFileIOInterface;



/**
 * \brief Initialises the IO callbacks with stdio file functions
 *
 * \param io A pointer to the callback structure to write to
 */
void image_io_init_stdio( SFileIOInterface* io );



#ifdef __cplusplus
}
#endif

#endif /* IMAGE_IO_H */

