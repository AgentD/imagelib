set( JPEG_LIB jcapimin.c
              jcapistd.c
              jdapimin.c
              jdapistd.c
              jcomapi.c
              jcparam.c
              jctrans.c
              jdtrans.c
              jaricom.c
              jerror.c
              jmemmgr.c
              jutils.c
              jmemnobs.c )

set( JPEG_COMPRESS jcinit.c
                   jcmaster.c
                   jcmainct.c
                   jcprepct.c
                   jccoefct.c
                   jccolor.c
                   jcsample.c
                   jcdctmgr.c
                   jfdctint.c
                   jfdctfst.c
                   jfdctflt.c
                   jchuff.c
                   jcarith.c
                   jcmarker.c
                   jdatadst.c )

set( JPEG_EXTRACT jdmaster.c
                  jdinput.c
                  jdmainct.c
                  jdcoefct.c
                  jdpostct.c
                  jdmarker.c
                  jdhuff.c
                  jdarith.c
                  jddctmgr.c
                  jidctint.c
                  jidctfst.c
                  jidctflt.c
                  jdsample.c
                  jdcolor.c
                  jdmerge.c
                  jquant1.c
                  jquant2.c
                  jdatasrc.c )

add_library( jpglib STATIC ${JPEG_LIB} ${JPEG_COMPRESS} ${JPEG_EXTRACT} )

