project( image )

cmake_minimum_required( VERSION 2.6 )

set( IMAGE_LIB src/image.cpp
               src/import/bmp.cpp
               src/import/tga.cpp
               src/export/bmp.cpp
               src/export/tga.cpp )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_SOURCE_DIR}/src/ )

add_library( img ${IMAGE_LIB} )
add_executable( imgTest main.cpp )

target_link_libraries( imgTest img )

