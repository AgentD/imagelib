add_executable( test_exporters test_exporters.c )
add_executable( test_edit      test_edit.c      )

target_link_libraries( test_exporters img )
target_link_libraries( test_edit      img )

