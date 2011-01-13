set( IMAGE_IMPORT import/bmp.cpp import/tga.cpp import/jpg.cpp import/png.cpp import/ascii.cpp )
set( IMAGE_EXPORT export/bmp.cpp export/tga.cpp export/jpg.cpp export/png.cpp export/ascii.cpp )
set( IMAGE_LIB image.cpp bitmap_font.cpp )


if( IMAGE_LOAD_JPG OR IMAGE_SAVE_JPG )
  set( IMAGE_DEP ${IMAGE_DEP} jpglib )
endif( )

if( IMAGE_LOAD_PNG OR IMAGE_SAVE_PNG )
  set( IMAGE_DEP ${IMAGE_DEP} zlib )
  set( IMAGE_DEP ${IMAGE_DEP} libpng )
endif( )


add_library( img STATIC ${IMAGE_LIB} ${IMAGE_IMPORT} ${IMAGE_EXPORT} )
target_link_libraries( img ${IMAGE_DEP} )

