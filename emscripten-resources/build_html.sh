EMSCRIPTEN_DIR="/home/heatingdevice/projects/emsdk/emscripten/incoming"
BUILD_DIR="/home/heatingdevice/projects/stk-code/cmake_build"

/usr/bin/python "$EMSCRIPTEN_DIR/emcc.py" \
-D_REENTRANT \
\
-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
-s USE_PTHREADS=0 \
-s WASM=0 \
-s FULL_ES2=1 \
-s DISABLE_EXCEPTION_CATCHING=0 \
\
-s DEMANGLE_SUPPORT=1 \
\
-s USE_WEBGL2=0 \
\
-s ALLOW_MEMORY_GROWTH=1 \
-s TOTAL_MEMORY=33554432 \
\
--pre-js setupFs.js \
--preload-file opt@$BUILD_DIR/opt \
--use-preload-cache \
--no-heap-copy \
\
-std=gnu++0x \
-Wall \
-Wno-unused-function \
-DNDEBUG @CMakeFiles/supertuxkart.dir/objects1.rsp \
@CMakeFiles/supertuxkart.dir/linklibs.rsp \
--emscripten-cxx \
\
\
"-I$EMSCRIPTEN_DIR/system/include"  \
-I/home/heatingdevice/projects/freetype/include \
-l/home/heatingdevice/projects/freetype/cmake_build/libfreetype \
-l/home/heatingdevice/projects/emscripten-libjpeg-turbo/libjpeg-turbo/lib64/libjpeg \
-l/home/heatingdevice/projects/libpng-1.2.49/cmake_build/libpng \
-l/home/heatingdevice/projects/emscripten-zlib/cmake_build/libz \
-l/home/heatingdevice/projects/vorbis/cmake_build/lib/libvorbisfile \
-l/home/heatingdevice/projects/vorbis/cmake_build/lib/libvorbis \
-l/home/heatingdevice/projects/vorbis/cmake_build/lib/libvorbisenc \
-l/home/heatingdevice/projects/ogg/cmake_build/libogg \
-o bin/supertuxkart.html \
-O3
