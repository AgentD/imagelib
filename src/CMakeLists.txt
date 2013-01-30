set( IMAGE_IMPORT import/bmp.c
                  import/tga.c
                  import/jpg.c
                  import/png.c
                  import/pbm.c )

set( IMAGE_EXPORT export/bmp.c
                  export/tga.c
                  export/jpg.c
                  export/png.c
                  export/pbm.c )

set( IMAGE_LIB image.c io.c )


if( IMAGE_LOAD_JPG OR IMAGE_SAVE_JPG )
  set( IMAGE_DEP ${IMAGE_DEP} jpglib )
endif( )

if( IMAGE_LOAD_PNG OR IMAGE_SAVE_PNG )
  include_directories( lodepng )
  set( IMAGE_LIB ${IMAGE_LIB} lodepng/lodepng.c )
endif( )


add_library( img STATIC ${IMAGE_LIB} ${IMAGE_IMPORT} ${IMAGE_EXPORT} )
target_link_libraries( img ${IMAGE_DEP} )

