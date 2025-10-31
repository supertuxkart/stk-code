#!/bin/sh
# Tested in Ubuntu Server 24.04.2 LTS

patch_file=$(mktemp)
cat << 'EOF' > "$patch_file"
diff --git a/CMakeLists.txt b/CMakeLists.txt
index af9769a..3279caf 100644
--- a/CMakeLists.txt
+++ b/CMakeLists.txt
@@ -131,9 +131,7 @@ if (ISPC_CROSS)
             message(STATUS "Using iOS SDK path ${ISPC_IOS_SDK_PATH}")
         endif()
     else()
-        set(ISPC_WINDOWS_TARGET OFF)
         set(ISPC_PS_TARGET OFF)
-        set(ISPC_IOS_TARGET OFF)
         if (ISPC_MACOS_TARGET AND NOT ISPC_MACOS_SDK_PATH)
             message (FATAL_ERROR "Set ISPC_MACOS_SDK_PATH variable for cross compilation to MacOS e.g. /iusers/MacOSX10.14.sdk")
         endif()
diff --git a/builtins/builtins-c-cpu.cpp b/builtins/builtins-c-cpu.cpp
index 140419c..65b4f8e 100644
--- a/builtins/builtins-c-cpu.cpp
+++ b/builtins/builtins-c-cpu.cpp
@@ -39,11 +39,11 @@
 // In unistd.h we need the definition of sysconf and _SC_NPROCESSORS_ONLN used as its arguments.
 // We should include unistd.h, but it doesn't really work well for cross compilation, as
 // requires us to carry around unistd.h, which is not available on Windows out of the box.
-#include <unistd.h>
+//#include <unistd.h>
 
 // Just for the reference: these lines are eventually included from unistd.h
-// #define _SC_NPROCESSORS_ONLN 58
-// long sysconf(int);
+ #define _SC_NPROCESSORS_ONLN 58
+ long sysconf(int);
 #endif // !_MSC_VER
 
 #endif // !WASM
diff --git a/cmake/GenerateBuiltins.cmake b/cmake/GenerateBuiltins.cmake
index f403b16..56961d6 100644
--- a/cmake/GenerateBuiltins.cmake
+++ b/cmake/GenerateBuiltins.cmake
@@ -253,6 +253,10 @@ function (get_target_flags os arch out)
         if (${os} STREQUAL "macos")
             # -isystem/iusers/MacOSX10.14.sdk.tar/MacOSX10.14.sdk/usr/include
             set(include -isystem${ISPC_MACOS_SDK_PATH}/usr/include)
+        elseif (${os} STREQUAL "ios")
+            set(include -isystem${ISPC_IOS_SDK_PATH}/usr/include)
+        elseif (${os} STREQUAL "windows")
+            set(include -I/usr/include)
         elseif(NOT ${debian_triple} STREQUAL "")
             # When compiling on Linux, there are two way to support cross targets:
             # - add "foreign" architecture to the set of supported architectures and install corresponding toolchain.
EOF

apt-get update
apt install -y build-essential llvm cmake clang m4 bison flex libtbb-dev libclang-18-dev libclang-cpp-dev gcc-multilib g++-multilib
cd /opt
wget https://github.com/supertuxkart/dependencies/releases/download/cctools/cctools-14.1.tar.xz
tar xf cctools-14.1.tar.xz
rm cctools-14.1.tar.xz
cd
git clone --branch v1.26.0 --depth=1 https://github.com/ispc/ispc
cd ispc
patch -p1 < "$patch_file"
rm "$patch_file"
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/ispc -DISPC_CROSS=ON -DISPC_MACOS_TARGET=ON -DISPC_MACOS_SDK_PATH=/opt/cctools/sdk/MacOSX13.0.sdk -DISPC_IOS_SDK_PATH=/opt/cctools/sdk/iPhoneOS16.1.sdk -DISPC_INCLUDE_TESTS=OFF -DISPC_INCLUDE_EXAMPLES=OFF
make -j18
make install
cd /opt
tar -cJvf /ispc-cross-1.26.0.tar.xz ispc
