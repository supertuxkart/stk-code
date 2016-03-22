cmake_minimum_required(VERSION 2.4.4)

if(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -F/Library/Frameworks")
endif()

SET(JPEG_SRCS
 jaricom.c
 jcapimin.c
 jcapistd.c
 jcarith.c
 jccoefct.c
 jccolor.c
 jcdctmgr.c
 jchuff.c
 jcinit.c
 jcmainct.c
 jcmarker.c
 jcmaster.c
 jcomapi.c
 jcparam.c
 jcprepct.c
 jcsample.c
 jctrans.c
 jdapimin.c
 jdapistd.c
 jdarith.c
 jdatadst.c
 jdatasrc.c
 jdcoefct.c
 jdcolor.c
 jddctmgr.c
 jdhuff.c
 jdinput.c
 jdmainct.c
 jdmarker.c
 jdmaster.c
 jdmerge.c
 jdpostct.c
 jdsample.c
 jdtrans.c
 jerror.c
 jfdctflt.c
 jfdctfst.c
 jfdctint.c
 jidctflt.c
 jidctfst.c
 jidctint.c
 jmemmgr.c
 jmemnobs.c
 jquant1.c
 jquant2.c
 jutils.c)

add_library(jpeglib STATIC ${JPEG_SRCS})
