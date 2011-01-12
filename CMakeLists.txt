project( image )

cmake_minimum_required( VERSION 2.6 )


#----------------------------------------------------------------------
# Change the output directories of the generated archives and binaries
#----------------------------------------------------------------------
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )


#----------------------------------------------------------------------
# Add all required include directories
#----------------------------------------------------------------------
include_directories( ${CMAKE_SOURCE_DIR}/include/
                     ${CMAKE_SOURCE_DIR}/src/
                     ${CMAKE_SOURCE_DIR}/src/jpglib/
                     ${CMAKE_SOURCE_DIR}/src/zlib/
                     ${CMAKE_SOURCE_DIR}/src/libpng/
                   )

#----------------------------------------------------------------------
# Add definitions to compile our various loaders and writers
#----------------------------------------------------------------------
option( IMAGE_LOAD_TGA "Compile Truvision TARGA(*.tga) Image File loader" ON )
option( IMAGE_LOAD_BMP "Compile Microsoft Windows Bitmap(*.bmp) Image File loader" ON )
option( IMAGE_LOAD_JPG "Compile JPEG Interchange Format(*.jpg) Image File loader" ON )
option( IMAGE_LOAD_PNG "Compile Portable Network Graphics(*.png) Image File loader" ON )
option( IMAGE_LOAD_TXT "Compile ASCII ART(*.txt) Image File loader" ON )

option( IMAGE_SAVE_TGA "Compile Truvision TARGA(*.tga) Image File writer" ON )
option( IMAGE_SAVE_BMP "Compile Microsoft Windows Bitmap(*.bmp) Image File writer" ON )
option( IMAGE_SAVE_JPG "Compile JPEG Interchange Format(*.jpg) Image File writer" ON )
option( IMAGE_SAVE_PNG "Compile Portable Network Graphics(*.png) Image File writer" ON )

if( IMAGE_LOAD_TGA )
  add_definitions( -DIMAGE_LOAD_TGA )
endif( )

if( IMAGE_LOAD_BMP )
  add_definitions( -DIMAGE_LOAD_BMP )
endif( )

if( IMAGE_LOAD_JPG )
  add_definitions( -DIMAGE_LOAD_JPG )
endif( )

if( IMAGE_LOAD_PNG )
  add_definitions( -DIMAGE_LOAD_PNG )
endif( )

if( IMAGE_LOAD_TXT )
  add_definitions( -DIMAGE_LOAD_TXT )
endif( )


if( IMAGE_SAVE_TGA )
  add_definitions( -DIMAGE_SAVE_TGA )
endif( )

if( IMAGE_SAVE_BMP )
  add_definitions( -DIMAGE_SAVE_BMP )
endif( )

if( IMAGE_SAVE_JPG )
  add_definitions( -DIMAGE_SAVE_JPG )
endif( )

if( IMAGE_SAVE_PNG )
  add_definitions( -DIMAGE_SAVE_PNG )
endif( )

#----------------------------------------------------------------------
# Add all libraries to compile
#----------------------------------------------------------------------

if( IMAGE_LOAD_PNG OR IMAGE_SAVE_PNG )
  add_subdirectory( src/zlib )
  add_subdirectory( src/libpng )
endif( )

if( IMAGE_LOAD_JPG OR IMAGE_SAVE_JPG )
  add_subdirectory( src/jpglib )
endif( )

add_subdirectory( src )

#----------------------------------------------------------------------
# Add our test application
#----------------------------------------------------------------------
add_executable( imgTest main.cpp )
target_link_libraries( imgTest img )

