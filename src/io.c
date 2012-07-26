#include "image_io.h"

#include <stdio.h>



void image_io_init_stdio( SFileIOInterface* io )
{
    io->read  = (size_t(*)(void*,size_t,size_t,void*))fread;
    io->write = (size_t(*)(const void*,size_t,size_t,void*))fwrite;
    io->seek  = (int(*)(void*,long,int))fseek;
    io->tell  = (long(*)(void*))ftell;
    io->eof   = (int(*)(void*))feof;
}

