# CMakeLists.txt - glew

include_directories("include")

if(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch i386")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i386 -F/Library/Frameworks")
endif()

add_library(glew STATIC
                 src/glew.c  src/glewinfo.c  src/visualinfo.c
)

#if(MINGW)
#  target_link_libraries(enet wsock32 ws2_32 Winmm)
#endif()

#if(WIN32)
#  find_library(WS2_LIBRARY NAMES "ws2_32" PATHS "C:/Windows/System32")
#  target_link_libraries(enet ${WS2_LIBRARY})
#endif()
