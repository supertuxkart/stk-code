# CMakeLists.txt - glew

include_directories("include")

if(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch i386")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i386 -F/Library/Frameworks")
endif()

add_library(glew STATIC
    include/GL/glew.h  include/GL/glxew.h  include/GL/wglew.h
    src/glew.c         src/glewinfo.c
)

add_definitions(-DGLEW_STATIC)
