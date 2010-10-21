set( IMAGE_IMPORT import/bmp.cpp import/tga.cpp import/jpg.cpp )
set( IMAGE_EXPORT export/bmp.cpp export/tga.cpp export/jpg.cpp )
set( IMAGE_LIB image.cpp )


if( IMAGE_LOAD_JPG OR IMAGE_SAVE_JPG )
  set( IMAGE_DEP ${IMAGE_DEP} jpglib )
endif( )


add_library( img STATIC ${IMAGE_LIB} ${IMAGE_IMPORT} ${IMAGE_EXPORT} )
target_link_libraries( img ${IMAGE_DEP} )

