emconfigure cmake .. \
-DUSE_GLES2=1 \
-DBUILD_RECORDER=off \
-DJPEG_INCLUDE_DIR=/home/heatingdevice/projects/emscripten-libjpeg-turbo \
-DJPEG_LIBRARY=/home/heatingdevice/projects/emscripten-libjpeg-turbo \
-DPNG_PNG_INCLUDE_DIR=/home/heatingdevice/projects/libpng-1.2.49/src \
-DPNG_LIBRARY=/home/heatingdevice/projects/libpng-1.2.49/cmake_build \
-DZLIB_INCLUDE_DIR=/home/heatingdevice/projects/emscripten-zlib \
-DZLIB_LIBRARY=/home/heatingdevice/projects/emscripten-zlib/cmake_build \
-DNO_IRR_COMPILE_WITH_X11_=yes \
-DUSE_WIIUSE=0 \
-DENABLE_WAYLAND_DEVICE=0 \
-DOGGVORBIS_OGG_INCLUDE_DIR=/home/heatingdevice/projects/ogg/include \
-DOGGVORBIS_VORBIS_INCLUDE_DIR=/home/heatingdevice/projects/vorbis/include \
-DOGGVORBIS_OGG_LIBRARY=/home/heatingdevice/projects/ogg/cmake_build \
-DOGGVORBIS_VORBIS_LIBRARY=/home/heatingdevice/projects/vorbis/cmake_build \
-DOGGVORBIS_VORBISFILE_LIBRARY=/home/heatingdevice/projects/vorbis/cmake_build \
-DFREETYPE_INCLUDE_DIRS=/home/heatingdevice/projects/freetype/include \
-DUSE_FRIBIDI=0 \
-DPTHREAD_LIBRARY=/home/heatingdevice/projects/emsdk/emscripten/incoming/system/lib/pthread/library_pthread.c \
-DOGGVORBIS_VORBISENC_LIBRARY=/home/heatingdevice/projects/vorbis/cmake_build \
-DCMAKE_CXX_FLAGS="-D_REENTRANT -s ALLOW_MEMORY_GROWTH=1 -s DISABLE_EXCEPTION_CATCHING=0 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -I/home/heatingdevice/projects/emsdk/emscripten/incoming/system/include -s WASM=0 -s USE_PTHREADS=0 -s FULL_ES3=1 -s ASSERTIONS=1 -s GL_ASSERTIONS=1 -s GL_DEBUG=1 -O0" \
-DEGL_INCLUDE_DIR=/home/heatingdevice/projects/emsdk/emscripten/incoming/system/include \
-DEGL_LIBRARY=/home/heatingdevice/projects/emsdk/emscripten/incoming/system/include/EGL \
-DCMAKE_INSTALL_PREFIX="./opt/stk"



# -DCHECK_ASSETS=off \
