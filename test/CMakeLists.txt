add_executable( test_exporters test_exporters.c )
add_executable( test_loaders   test_loaders.c   )

target_link_libraries( test_exporters img )
target_link_libraries( test_loaders   img )

file( COPY        ${CMAKE_CURRENT_SOURCE_DIR}/samples
      DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )

