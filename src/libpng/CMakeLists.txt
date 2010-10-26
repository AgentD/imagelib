add_library( libpng STATIC png.c
                           pngerror.c
                           pnggccrd.c
                           pngget.c
                           pngmem.c
                           pngpread.c
                           pngread.c
                           pngrio.c
                           pngrtran.c
                           pngrutil.c
                           pngset.c
                           pngtrans.c
                           pngvcrd.c
                           pngwio.c
                           pngwrite.c
                           pngwtran.c
                           pngwutil.c )

target_link_libraries( libpng zlib )

