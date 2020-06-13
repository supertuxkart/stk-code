#!/bin/sh

#
# (C) 2020 Dawid Gan, under the Holy Hedgehog License (do whatever you want)
#

# This is a build script that creates STK linux package.
#
# To run the script you need debootstrap and schroot packages, and working
# chroot environment.
#
# The build environment can be created using debootstrap:
#
#   debootstrap --arch i386 --components=main,restricted,universe,multiverse \
#               xenial ./chroot-xenial32 http://archive.ubuntu.com/ubuntu/
#
#   debootstrap --arch amd64 --components=main,restricted,universe,multiverse \
#               xenial ./chroot-xenial64 http://archive.ubuntu.com/ubuntu/
#
#
# Here is example configuration for schroot:
# /etc/schroot/chroot.d/chroot-xenial32.conf
#
#   [chroot-xenial32]
#   description=Ubuntu Xenial
#   personality=linux32
#   directory=/path/to/chroot-xenial32
#   root-users=deve
#   type=directory
#   users=deve
#
#
# /etc/schroot/chroot.d/chroot-xenial64.conf
#
#   [chroot-xenial64]
#   description=Ubuntu Xenial 64-bit
#   #personality=linux32
#   directory=/path/to/chroot-xenial64
#   root-users=deve
#   type=directory
#   users=deve
#
#
# And some packages that may be useful (esp. build tools, sdl2 and openal 
# dependencies)
#
#   autoconf automake autopoint autotools-dev binutils bsdmainutils
#   build-essential cmake cmake-data cpp cpp-5 debhelper dh-autoreconf
#   dh-strip-nondeterminism diffstat docbook docbook-to-man docbook-xml 
#   docbook-xsl doxygen dpkg-dev fontconfig fontconfig-config fonts-dejavu-core 
#   g++ g++-5 gcc gcc-5 gettext gettext-base graphviz groff-base icu-devtools 
#   intltool-debian libalut-dev libalut0 libarchive-zip-perl libarchive13 
#   libasan2 libasn1-8-heimdal libasound2 libasound2-data libasound2-dev 
#   libasprintf0v5 libasyncns0 libatomic1 libbluetooth-dev libbluetooth3 
#   libboost-filesystem1.58.0 libboost-system1.58.0 libbz2-dev libc-dev-bin 
#   libc6-dev libcaca-dev libcaca0 libcairo2 libcc1-0 libcdt5 libcgraph6 
#   libcilkrts5 libclang1-3.6 libcroco3 libcurl3 libcurl3-gnutls 
#   libcurl4-gnutls-dev libdatrie1 libdbus-1-3 libdbus-1-dev libdpkg-perl 
#   libdrm-amdgpu1 libdrm-dev libdrm-intel1 libdrm-nouveau2 libdrm-radeon1 
#   libdrm2 libedit2 libegl1-mesa libegl1-mesa-dev libelf1 libenet-dev libenet7 
#   libexpat1-dev libffi-dev libfile-stripnondeterminism-perl libflac8 
#   libfontconfig1 libfreetype6 libfribidi-dev libgbm1 libgcc-5-dev libgd3 
#   libgdbm3 libgl1-mesa-dev libgl1-mesa-dri libgl1-mesa-glx libglapi-mesa 
#   libgles2-mesa libgles2-mesa-dev libglib2.0-0 libglib2.0-bin libglib2.0-data 
#   libglib2.0-dev libglu1-mesa libglu1-mesa-dev libgomp1 libgraphite2-3 
#   libgssapi-krb5-2 libgssapi3-heimdal libgvc6 libgvpr2 libharfbuzz0b 
#   libhcrypto4-heimdal libheimbase1-heimdal libheimntlm0-heimdal 
#   libhx509-5-heimdal libice-dev libice6 libicu-dev libicu55 libisl15 libitm1 
#   libjack-dev libjack0 libjbig-dev libjbig0 libjpeg-dev libjpeg-turbo8 
#   libjpeg-turbo8-dev libjpeg8 libjpeg8-dev libjsoncpp1 libk5crypto3 
#   libkeyutils1 libkrb5-26-heimdal libkrb5-3 libkrb5support0 libldap-2.4-2 
#   libllvm3.6v5 libllvm3.8 libltdl7 liblzma-dev liblzo2-2 libmirclient-dev 
#   libmirclient9 libmircommon-dev libmircommon5 libmircookie-dev libmircookie2 
#   libmirprotobuf3 libmpc3 libmpfr4 libmpx0 libobjc-5-dev libobjc4 libogg-dev 
#   libogg0 libopenal-data libopenal-dev libopenal1 libpango-1.0-0 
#   libpangocairo-1.0-0 libpangoft2-1.0-0 libpathplan4 libpciaccess0 libpcre16-3
#   libpcre3-dev libpcre32-3 libpcrecpp0v5 libperl5.22 libpipeline1 
#   libpixman-1-0 libpng12-dev libportaudio2 libportaudiocpp0 libprotobuf-dev 
#   libprotobuf-lite9v5 libprotobuf9v5 libpthread-stubs0-dev libpulse-dev 
#   libpulse-mainloop-glib0 libpulse0 libpython-stdlib libpython2.7-minimal 
#   libpython2.7-stdlib libquadmath0 libreadline-dev libreadline6-dev 
#   libroken18-heimdal librtmp1 libsasl2-2 libsasl2-modules-db libsdl-image1.2 
#   libsdl-image1.2-dev libsdl-ttf2.0-0 libsdl-ttf2.0-dev libsdl1.2-dev 
#   libsdl1.2debian libsigsegv2 libslang2-dev libsm-dev libsm6 libsndfile1 
#   libsndio-dev libsndio6.1 libsp1c2 libstdc++-5-dev libtcl8.6 libthai-data 
#   libthai0 libtiff5 libtiff5-dev libtiffxx5 libtimedate-perl libtinfo-dev 
#   libtool libubsan0 libudev-dev libunistring0 libvorbis-dev libvorbis0a 
#   libvorbisenc2 libvorbisfile3 libvpx-dev libvpx3 libwayland-client0 
#   libwayland-cursor0 libwayland-dev libwayland-egl1-mesa libwayland-server0 
#   libwebp-dev libwebp5 libwebpdemux1 libwebpmux1 libwind0-heimdal libwrap0 
#   libx11-6 libx11-data libx11-dev libx11-xcb-dev libx11-xcb1 libxau-dev 
#   libxau6 libxaw7 libxcb-dri2-0 libxcb-dri2-0-dev libxcb-dri3-0 
#   libxcb-dri3-dev libxcb-glx0 libxcb-glx0-dev libxcb-present-dev 
#   libxcb-present0 libxcb-randr0 libxcb-randr0-dev libxcb-render0 
#   libxcb-render0-dev libxcb-shape0 libxcb-shape0-dev libxcb-shm0 
#   libxcb-sync-dev libxcb-sync1 libxcb-xfixes0 libxcb-xfixes0-dev libxcb1 
#   libxcb1-dev libxcursor-dev libxcursor1 libxdamage-dev libxdamage1 
#   libxdmcp-dev libxdmcp6 libxext-dev libxext6 libxfixes-dev libxfixes3
#   libxi-dev libxi6 libxinerama-dev libxinerama1 libxkbcommon-dev libxkbcommon0
#   libxml2-dev libxml2-utils libxml2 libxmu6 libxpm4 libxrandr-dev libxrandr2 
#   libxrender-dev libxrender1 libxshmfence-dev libxshmfence1 libxslt1.1 
#   libxss-dev libxss1 libxt-dev libxt6 libxv-dev libxv1 libxxf86vm-dev 
#   libxxf86vm1 linux-libc-dev m4 make man-db mesa-common-dev meson ninja-build 
#   patch perl perl-modules-5.22 php-cli php-common php7.0-cli php7.0-common 
#   php7.0-json php7.0-opcache php7.0-readline pkg-config po-debconf 
#   portaudio19-dev psmisc python python-minimal python2.7 python2.7-minimal 
#   quilt sgml-base sgml-data sp tcl8.6 tcl8.6-dev uuid-dev x11-common 
#   x11proto-core-dev x11proto-damage-dev x11proto-dri2-dev x11proto-fixes-dev 
#   x11proto-gl-dev x11proto-input-dev x11proto-kb-dev x11proto-randr-dev 
#   x11proto-render-dev x11proto-scrnsaver-dev x11proto-video-dev 
#   x11proto-xext-dev x11proto-xf86vidmode-dev x11proto-xinerama-dev 
#   xml-core xmlto xorg-sgml-doctools xsltproc xtrans-dev xz-utils yasm 
#   zlib1g-dev
      

export DIRNAME="$(dirname "$(readlink -f "$0")")"

######################## CONFIG ########################

export STK_VERSION="git20200530"
export THREADS_NUMBER=`nproc`
export SCHROOT_32BIT_NAME="chroot-xenial32"
export SCHROOT_64BIT_NAME="chroot-xenial64"

export STKCODE_DIR="$DIRNAME/.."
export STKASSETS_DIR="$STKCODE_DIR/../supertuxkart-assets"
export OPENGLRECORDER_DIR="$STKCODE_DIR/../libopenglrecorder"
export STKEDITOR_DIR="$STKCODE_DIR/../supertuxkart-editor"

export BLACKLIST_LIBS="ld-linux libbsd.so libc.so libdl.so libdrm libexpat \
                       libGL libgl libm.so libmvec.so libpthread libresolv \
                       librt.so libX libxcb libxshm"

export BUILD_DIR_32BIT="build-linux-32bit"
export BUILD_DIR_64BIT="build-linux-64bit"
export DEPENDENCIES_DIR_32BIT="$STKCODE_DIR/dependencies-linux-32bit"
export DEPENDENCIES_DIR_64BIT="$STKCODE_DIR/dependencies-linux-64bit"
export STK_INSTALL_DIR="$STKCODE_DIR/build-linux-install"

########################################################


# A helper function that checks if error ocurred
check_error()
{
    if [ $? -gt 0 ]; then
        echo "Error ocurred."
        exit
    fi
}

write_run_game_sh()
{
    if [ -z "$1" ]; then
        return
    fi
    
    export INSTALL_DIR=$1
    export FILE="$INSTALL_DIR/run_game.sh"
    
    echo '#!/bin/sh'                                                      > "$FILE"
    echo ''                                                              >> "$FILE"
    echo 'export DIRNAME="$(dirname "$(readlink -f "$0")")"'             >> "$FILE"
    echo 'export MACHINE_TYPE=`uname -m`'                                >> "$FILE"
    echo 'export SYSTEM_LD_LIBRARY_PATH="$LD_LIBRARY_PATH"'              >> "$FILE"
    echo ''                                                              >> "$FILE"
    echo 'export SUPERTUXKART_DATADIR="$DIRNAME"'                        >> "$FILE"
    echo 'export SUPERTUXKART_ASSETS_DIR="$DIRNAME/data/"'               >> "$FILE"
    echo ''                                                              >> "$FILE"
    echo 'cd "$DIRNAME"'                                                 >> "$FILE"
    echo ''                                                              >> "$FILE"
    echo 'if [ $MACHINE_TYPE = "x86_64" ]; then'                         >> "$FILE"
    echo '    echo "Running 64-bit version..."'                          >> "$FILE"
    echo '    export LD_LIBRARY_PATH="$DIRNAME/lib-64:$LD_LIBRARY_PATH"' >> "$FILE"
    echo '    "$DIRNAME/bin-64/supertuxkart" "$@"'                       >> "$FILE"
    echo 'else'                                                          >> "$FILE"
    echo '    echo "Running 32-bit version..."'                          >> "$FILE"
    echo '    export LD_LIBRARY_PATH="$DIRNAME/lib:$LD_LIBRARY_PATH"'    >> "$FILE"
    echo '    "$DIRNAME/bin/supertuxkart" "$@"'                          >> "$FILE"
    echo 'fi'                                                            >> "$FILE"
}

build_stk()
{
    if [ -z "$1" ] || [ -z "$2" ]; then
        return
    fi
    
    export DEPENDENCIES_DIR="$1"
    export BUILD_DIR="$2"
    export STK_CMAKE_FLAGS="$3"
    export INSTALL_DIR="$DEPENDENCIES_DIR/dependencies"
    export INSTALL_LIB_DIR="$INSTALL_DIR/lib"
    export INSTALL_INCLUDE_DIR="$INSTALL_DIR/include"

    export PKG_CONFIG_PATH="$INSTALL_LIB_DIR/pkgconfig"
    export CFLAGS="-I$INSTALL_INCLUDE_DIR"
    export CPPFLAGS="-I$INSTALL_INCLUDE_DIR"
    export LDFLAGS="-Wl,-rpath,$INSTALL_LIB_DIR -L$INSTALL_LIB_DIR"
    
    cd "$STKCODE_DIR"
    mkdir -p "$DEPENDENCIES_DIR"
    
    # Zlib
    if [ ! -f "$DEPENDENCIES_DIR/zlib.stamp" ]; then
        echo "Compiling zlib"
        mkdir -p "$DEPENDENCIES_DIR/zlib"
        cp -a -f "$DEPENDENCIES_DIR/../lib/zlib/"* "$DEPENDENCIES_DIR/zlib"
    
        cd "$DEPENDENCIES_DIR/zlib"
        cmake . -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DINSTALL_PKGCONFIG_DIR="$PKG_CONFIG_PATH" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/zlib.stamp"
    fi
    
    # Libpng
    if [ ! -f "$DEPENDENCIES_DIR/libpng.stamp" ]; then
        echo "Compiling libpng"
        mkdir -p "$DEPENDENCIES_DIR/libpng"
        mkdir -p "$DEPENDENCIES_DIR/libpng/lib"
        cp -a -f "$DEPENDENCIES_DIR/../lib/libpng/"* "$DEPENDENCIES_DIR/libpng"
    
        cd "$DEPENDENCIES_DIR/libpng"
        cmake . -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DZLIB_LIBRARY="$INSTALL_LIB_DIR/libz.so" \
                -DZLIB_INCLUDE_DIR="$INSTALL_INCLUDE_DIR" \
                -DPNG_TESTS=0 &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/libpng.stamp"
    fi
    
    # Fribidi
    if [ ! -f "$DEPENDENCIES_DIR/fribidi.stamp" ]; then
        echo "Compiling fribidi"
        mkdir -p "$DEPENDENCIES_DIR/fribidi"
        cp -a -f "$DEPENDENCIES_DIR/../lib/fribidi/"* "$DEPENDENCIES_DIR/fribidi"
    
        cd "$DEPENDENCIES_DIR/fribidi"
        ./configure --prefix="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/fribidi.stamp"
    fi
    
    # Freetype bootstrap
    if [ ! -f "$DEPENDENCIES_DIR/freetype_bootstrap.stamp" ]; then
        echo "Compiling freetype"
        mkdir -p "$DEPENDENCIES_DIR/freetype"
        cp -a -f "$DEPENDENCIES_DIR/../lib/freetype/"* "$DEPENDENCIES_DIR/freetype"
    
        cd "$DEPENDENCIES_DIR/freetype"
        ZLIB_CFLAGS="-I$INSTALL_INCLUDE_DIR" ZLIB_LIBS="-L$INSTALL_LIB_DIR -l:libz.so"\
        LIBPNG_CFLAGS="-I$INSTALL_INCLUDE_DIR" LIBPNG_LIBS="-L$INSTALL_LIB_DIR -l:libpng.so"\
        ./configure --prefix="$INSTALL_DIR" --with-bzip2=no --with-harfbuzz=no --with-png=yes --with-zlib=yes &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        # We need to rebuild freetype after harfbuzz is compiled
        touch "$DEPENDENCIES_DIR/freetype_bootstrap.stamp"
    fi
    
    # Harfbuzz
    if [ ! -f "$DEPENDENCIES_DIR/harfbuzz.stamp" ]; then
        echo "Compiling harfbuzz"
        mkdir -p "$DEPENDENCIES_DIR/harfbuzz"
        cp -a -f "$DEPENDENCIES_DIR/../lib/harfbuzz/"* "$DEPENDENCIES_DIR/harfbuzz"
    
        cd "$DEPENDENCIES_DIR/harfbuzz"
        FREETYPE_CFLAGS="-I$INSTALL_INCLUDE_DIR -I$INSTALL_INCLUDE_DIR/freetype2" \
        FREETYPE_LIBS="-L$INSTALL_LIB_DIR -l:libfreetype.so -l:libpng.so -l:libz.so"\
        ./configure --prefix="$INSTALL_DIR" --with-glib=no --with-gobject=no --with-cairo=no \
                    --with-fontconfig=no --with-icu=no --with-graphite2=no &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/harfbuzz.stamp"
    fi
    
    # Freetype
    if [ ! -f "$DEPENDENCIES_DIR/freetype.stamp" ]; then
        echo "Compiling freetype"
        mkdir -p "$DEPENDENCIES_DIR/freetype"
        cp -a -f "$DEPENDENCIES_DIR/../lib/freetype/"* "$DEPENDENCIES_DIR/freetype"
    
        cd "$DEPENDENCIES_DIR/freetype"
        ZLIB_CFLAGS="-I$INSTALL_INCLUDE_DIR" ZLIB_LIBS="-L$INSTALL_LIB_DIR -l:libz.so" \
        LIBPNG_CFLAGS="-I$INSTALL_INCLUDE_DIR" LIBPNG_LIBS="-L$INSTALL_LIB_DIR -l:libpng.so" \
        HARFBUZZ_CFLAGS="-I$INSTALL_INCLUDE_DIR/harfbuzz" HARFBUZZ_LIBS="-L$INSTALL_LIB_DIR -l:libharfbuzz.so" \
        ./configure --prefix="$INSTALL_DIR" --with-bzip2=no --with-harfbuzz=yes --with-png=yes --with-zlib=yes &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/freetype.stamp"
    fi
    
    # Openal
    if [ ! -f "$DEPENDENCIES_DIR/openal.stamp" ]; then
        echo "Compiling openal"
        mkdir -p "$DEPENDENCIES_DIR/openal"
        cp -a -f "$DEPENDENCIES_DIR/../lib/openal/"* "$DEPENDENCIES_DIR/openal"
    
        cd "$DEPENDENCIES_DIR/openal"
        cmake . -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DALSOFT_UTILS=0            \
                -DALSOFT_EXAMPLES=0         \
                -DALSOFT_TESTS=0            \
                -DALSOFT_BACKEND_SNDIO=0 &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/openal.stamp"
    fi
    
    # OpenSSL
    if [ ! -f "$DEPENDENCIES_DIR/openssl.stamp" ]; then
        echo "Compiling openssl"
        mkdir -p "$DEPENDENCIES_DIR/openssl"
        cp -a -f "$DEPENDENCIES_DIR/../lib/openssl/"* "$DEPENDENCIES_DIR/openssl"
    
        cd "$DEPENDENCIES_DIR/openssl"
        ./config --prefix="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/openssl.stamp"
    fi
    
    # Curl
    if [ ! -f "$DEPENDENCIES_DIR/curl.stamp" ]; then
        echo "Compiling curl"
        mkdir -p "$DEPENDENCIES_DIR/curl"
        cp -a -f "$DEPENDENCIES_DIR/../lib/curl/"* "$DEPENDENCIES_DIR/curl"
    
        cd "$DEPENDENCIES_DIR/curl"
        CPPFLAGS="-I$INSTALL_INCLUDE_DIR" \
        LDFLAGS="-L$INSTALL_LIB_DIR"          \
        ./configure --prefix="$INSTALL_DIR" \
                    --with-ssl                              \
                    --enable-shared                        \
                    --enable-threaded-resolver \
                    --disable-ldap \
                    --without-libidn \
                    --without-libidn2 \
                    --without-libpsl \
                    --without-librtmp &&
        make -j$THREADS_NUMBER && \
        make install
        check_error
        touch "$DEPENDENCIES_DIR/curl.stamp"
    fi
    
    # Libjpeg
    if [ ! -f "$DEPENDENCIES_DIR/libjpeg.stamp" ]; then
        echo "Compiling libjpeg"
        mkdir -p "$DEPENDENCIES_DIR/libjpeg"
        cp -a -f "$DEPENDENCIES_DIR/../lib/libjpeg/"* "$DEPENDENCIES_DIR/libjpeg"
    
        cd "$DEPENDENCIES_DIR/libjpeg"
        chmod a+x ./configure
        cmake . -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/libjpeg.stamp"
    fi
    
    # Libogg
    if [ ! -f "$DEPENDENCIES_DIR/libogg.stamp" ]; then
        echo "Compiling libogg"
        mkdir -p "$DEPENDENCIES_DIR/libogg"
        cp -a -f "$DEPENDENCIES_DIR/../lib/libogg/"* "$DEPENDENCIES_DIR/libogg"
    
        cd "$DEPENDENCIES_DIR/libogg"
        ./configure --prefix="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/libogg.stamp"
    fi
    
    # Libvorbis
    if [ ! -f "$DEPENDENCIES_DIR/libvorbis.stamp" ]; then
        echo "Compiling libvorbis"
        mkdir -p "$DEPENDENCIES_DIR/libvorbis"
        cp -a -f "$DEPENDENCIES_DIR/../lib/libvorbis/"* "$DEPENDENCIES_DIR/libvorbis"
    
        cd "$DEPENDENCIES_DIR/libvorbis"
        CPPFLAGS="-I$INSTALL_INCLUDE_DIR" \
        LDFLAGS="-L$INSTALL_LIB_DIR -lm" \
        ./configure --prefix="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/libvorbis.stamp"
    fi

    # Wayland
    if [ ! -f "$DEPENDENCIES_DIR/wayland.stamp" ]; then
        echo "Compiling wayland"
        mkdir -p "$DEPENDENCIES_DIR/wayland"
        cp -a -f "$DEPENDENCIES_DIR/../lib/wayland/"* "$DEPENDENCIES_DIR/wayland"
    
        cd "$DEPENDENCIES_DIR/wayland"
        ./autogen.sh
        ./configure --prefix="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/wayland.stamp"
    fi
        
    # SDL2
    if [ ! -f "$DEPENDENCIES_DIR/sdl2.stamp" ]; then
        echo "Compiling SDL2"
        mkdir -p "$DEPENDENCIES_DIR/sdl2"
        cp -a -f "$DEPENDENCIES_DIR/../lib/sdl2/"* "$DEPENDENCIES_DIR/sdl2"
    
        cd "$DEPENDENCIES_DIR/sdl2"
        ./configure --prefix="$INSTALL_DIR" &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/sdl2.stamp"
    fi
    
    # Libvpx
    if [ ! -f "$DEPENDENCIES_DIR/libvpx.stamp" ]; then
        echo "Compiling libvpx"
        mkdir -p "$DEPENDENCIES_DIR/libvpx"
        cp -a -f "$DEPENDENCIES_DIR/../lib/libvpx/"* "$DEPENDENCIES_DIR/libvpx"
    
        cd "$DEPENDENCIES_DIR/libvpx"
        ./configure --prefix="$INSTALL_DIR" \
                    --enable-shared &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/libvpx.stamp"
    fi
    
    # Libbluetooth
    if [ ! -f "$DEPENDENCIES_DIR/bluez.stamp" ]; then
        echo "Compiling libbluetooth"
        mkdir -p "$DEPENDENCIES_DIR/bluez"
        cp -a -f "$DEPENDENCIES_DIR/../lib/bluez/"* "$DEPENDENCIES_DIR/bluez"
    
        cd "$DEPENDENCIES_DIR/bluez"
        ./configure --prefix="$INSTALL_DIR" \
                    --enable-library \
                    --disable-debug \
                    --disable-systemd \
                    --disable-tools \
                    --disable-obex \
                    --disable-cups \
                    --disable-client \
                    --disable-datafiles \
                    --disable-monitor \
                    --disable-udev &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/bluez.stamp"
    fi
    
    # Sqlite
    if [ ! -f "$DEPENDENCIES_DIR/sqlite.stamp" ]; then
        echo "Compiling sqlite"
        mkdir -p "$DEPENDENCIES_DIR/sqlite"
        cp -a -f "$DEPENDENCIES_DIR/../lib/sqlite/"* "$DEPENDENCIES_DIR/sqlite"
    
        cd "$DEPENDENCIES_DIR/sqlite"
        ./configure --prefix="$INSTALL_DIR" \
                    --disable-tcl &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/sqlite.stamp"
    fi
    
    # Openglrecorder
    if [ ! -f "$DEPENDENCIES_DIR/openglrecorder.stamp" ]; then
        echo "Compiling openglrecorder"
        mkdir -p "$DEPENDENCIES_DIR/openglrecorder"
        cp -a -f "$OPENGLRECORDER_DIR/"* "$DEPENDENCIES_DIR/openglrecorder"
    
        cd "$DEPENDENCIES_DIR/openglrecorder"
        cmake . -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
                -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
                -DBUILD_PULSE_WO_DL=0 &&
        make -j$THREADS_NUMBER &&
        make install
        check_error
        touch "$DEPENDENCIES_DIR/openglrecorder.stamp"
    fi

    # Supertuxkart
    mkdir -p "$STKCODE_DIR/$BUILD_DIR"
    cd "$STKCODE_DIR/$BUILD_DIR"
    cmake .. -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
             -DUSE_SYSTEM_ANGELSCRIPT=0 \
             -DUSE_SYSTEM_ENET=0 \
             -DUSE_SYSTEM_GLEW=0 \
             -DUSE_SYSTEM_WIIUSE=0 \
             -DENABLE_WAYLAND_DEVICE=0 \
             -DCMAKE_DISABLE_FIND_PACKAGE_Fontconfig=1 \
             $STK_CMAKE_FLAGS &&
    make -j$THREADS_NUMBER
    check_error
    
    # Stk editor
    mkdir -p "$STKEDITOR_DIR/$BUILD_DIR"
    cd "$STKEDITOR_DIR/$BUILD_DIR"
    cmake .. -DCMAKE_FIND_ROOT_PATH="$INSTALL_DIR" \
             -DSTATIC_ZLIB=1 \
             -DSTATIC_PHYSFS=1 \
             -DCMAKE_DISABLE_FIND_PACKAGE_Fontconfig=1 \
             $STK_CMAKE_FLAGS &&
    make -j$THREADS_NUMBER
    check_error
}

copy_libraries()
{
    if [ -z "$1" ] || [ -z "$2" ]; then
        return
    fi
    
    export DEPENDENCIES_DIR=$1
    export BUILD_DIR=$2
    export LIB_INSTALL_DIR=$3
    
    if [ -z "$DEPENDENCIES_DIR" ] || [ -z "$BUILD_DIR" ] || [ -z "$LIB_INSTALL_DIR" ]; then
        return
    fi
    
    LIBRARIES_LIST=`LD_LIBRARY_PATH="$DEPENDENCIES_DIR/dependencies/lib" \
                    ldd "$STKCODE_DIR/$BUILD_DIR/bin/supertuxkart" | \
                    cut -d">" -f2 | cut -d"(" -f1 | grep "\.so"`

    for FILE in $LIBRARIES_LIST; do 
        BLACKLISTED=0
    
        for BLACKLIST_LIB in $BLACKLIST_LIBS; do
            if [ `echo "$FILE" | grep -c "$BLACKLIST_LIB"` -gt 0 ]; then
                BLACKLISTED=1
                break
            fi
        done

        if [ $BLACKLISTED -eq 1 ]; then
            continue
        fi 
    
        if [ -f "$FILE" ]; then
            echo " Copying $FILE"
            cp -L "$FILE" "$LIB_INSTALL_DIR"
        fi
    done
}

test_package()
{
    if [ -z "$1" ]; then
        return
    fi
    
    PACKAGE_DIR="$1"
    
    if [ `objdump -a "$PACKAGE_DIR/bin/supertuxkart" | grep -c "elf32-i386"` -eq 0 ]; then
        echo "Error: bin/supertuxkart is not 32-bit"
        exit 1
    fi
    
    if [ `objdump -a "$PACKAGE_DIR/bin/supertuxkart-editor" | grep -c "elf32-i386"` -eq 0 ]; then
        echo "Error: bin/supertuxkart-editor is not 32-bit"
        exit 1
    fi

    if [ `objdump -a "$PACKAGE_DIR/bin-64/supertuxkart" | grep -c "elf64-x86-64"` -eq 0 ]; then
        echo "Error: bin-64/supertuxkart is not 64-bit"
        exit 1
    fi
    
    if [ `objdump -a "$PACKAGE_DIR/bin-64/supertuxkart-editor" | grep -c "elf64-x86-64"` -eq 0 ]; then
        echo "Error: bin-64/supertuxkart-editor is not 64-bit"
        exit 1
    fi

    if [ `LD_LIBRARY_PATH="$PACKAGE_DIR/lib" ldd "$PACKAGE_DIR/bin/supertuxkart" | grep -c "not found"` -gt 0 ]; then
        echo "Error: bin/supertuxkart has some missing libraries"
        exit 1
    fi
    
    if [ `ldd "$PACKAGE_DIR/bin/supertuxkart-editor" | grep -c "not found"` -gt 0 ]; then
        echo "Error: bin/supertuxkart-editor has some missing libraries"
        exit 1
    fi

    if [ `LD_LIBRARY_PATH="$PACKAGE_DIR/lib-64" ldd "$PACKAGE_DIR/bin-64/supertuxkart" | grep -c "not found"` -gt 0 ]; then
        echo "Error: bin-64/supertuxkart has some missing libraries"
        exit 1
    fi
    
    if [ `ldd "$PACKAGE_DIR/bin-64/supertuxkart-editor" | grep -c "not found"` -gt 0 ]; then
        echo "Error: bin-64/supertuxkart-editor has some missing libraries"
        exit 1
    fi
    
    LD_LIBRARY_PATH="$PACKAGE_DIR/lib" "$PACKAGE_DIR/bin/supertuxkart" --version
    
    if [ $? -ne 0 ]; then
        echo "Error: Couldn't start bin/supertuxkart"
        exit 1
    fi
    
    LD_LIBRARY_PATH="$PACKAGE_DIR/lib-64" "$PACKAGE_DIR/bin-64/supertuxkart" --version
    
    if [ $? -ne 0 ]; then
        echo "Error: Couldn't start bin-64/supertuxkart"
        exit 1
    fi
}

# Handle clean command
if [ ! -z "$1" ] && [ "$1" = "clean" ]; then
    rm -rf "$DEPENDENCIES_DIR_32BIT"
    rm -rf "$DEPENDENCIES_DIR_64BIT"
    rm -rf "$STKCODE_DIR/$BUILD_DIR_32BIT"
    rm -rf "$STKCODE_DIR/$BUILD_DIR_64BIT"
    rm -rf "$STKCODE_DIR/$BUILD_DIR_32BIT-symbols"
    rm -rf "$STKCODE_DIR/$BUILD_DIR_64BIT-symbols"
    rm -rf "$STKEDITOR_DIR/$BUILD_DIR_32BIT"
    rm -rf "$STKEDITOR_DIR/$BUILD_DIR_64BIT"
    rm -rf "$STKEDITOR_DIR/$BUILD_DIR_32BIT-symbols"
    rm -rf "$STKEDITOR_DIR/$BUILD_DIR_64BIT-symbols"
    rm -rf "$STK_INSTALL_DIR"
    exit 0
fi

# Handle build_stk command (internal only)
if [ ! -z "$1 " ] && [ "$1" = "build_stk" ]; then
    build_stk "$2" "$3" "$4"
    exit 0
fi

# Handle copy_libraries command (internal only)
if [ ! -z "$1 " ] && [ "$1" = "copy_libraries" ]; then
    copy_libraries "$2" "$3" "$4"
    exit 0
fi

# Building STK
echo "Building 32-bit version..."

schroot -c $SCHROOT_32BIT_NAME -- "$0" build_stk "$DEPENDENCIES_DIR_32BIT" "$BUILD_DIR_32BIT"

if [ ! -f "$STKCODE_DIR/$BUILD_DIR_32BIT/bin/supertuxkart" ]; then
    echo "Couldn't build 32-bit version."
    exit 1
fi

echo "Building 64-bit version..."

schroot -c $SCHROOT_64BIT_NAME -- "$0" build_stk "$DEPENDENCIES_DIR_64BIT" "$BUILD_DIR_64BIT"

if [ ! -f "$STKCODE_DIR/$BUILD_DIR_64BIT/bin/supertuxkart" ]; then
    echo "Couldn't build 64-bit version."
    exit 1
fi

echo "Building 32-bit version with symbols..."

schroot -c $SCHROOT_32BIT_NAME -- "$0" build_stk "$DEPENDENCIES_DIR_32BIT" "$BUILD_DIR_32BIT-symbols" "-DDEBUG_SYMBOLS=1"

if [ ! -f "$STKCODE_DIR/$BUILD_DIR_32BIT/bin/supertuxkart" ]; then
    echo "Couldn't build 32-bit version with symbols."
    exit 1
fi

echo "Building 64-bit version with symbols..."

schroot -c $SCHROOT_64BIT_NAME -- "$0" build_stk "$DEPENDENCIES_DIR_64BIT" "$BUILD_DIR_64BIT-symbols" "-DDEBUG_SYMBOLS=1"

if [ ! -f "$STKCODE_DIR/$BUILD_DIR_64BIT/bin/supertuxkart" ]; then
    echo "Couldn't build 64-bit version with symbols."
    exit 1
fi

# Making package

echo "Prepare package..."

STK_PACKAGE_DIR="$STK_INSTALL_DIR/SuperTuxKart-$STK_VERSION-linux"

mkdir -p "$STK_PACKAGE_DIR"
mkdir -p "$STK_PACKAGE_DIR/bin"
mkdir -p "$STK_PACKAGE_DIR/bin-64"
mkdir -p "$STK_PACKAGE_DIR/lib"
mkdir -p "$STK_PACKAGE_DIR/lib-64"

schroot -c $SCHROOT_32BIT_NAME -- "$0" copy_libraries "$DEPENDENCIES_DIR_32BIT" "$BUILD_DIR_32BIT" "$STK_PACKAGE_DIR/lib"
schroot -c $SCHROOT_64BIT_NAME -- "$0" copy_libraries "$DEPENDENCIES_DIR_64BIT" "$BUILD_DIR_64BIT" "$STK_PACKAGE_DIR/lib-64"

find "$STK_PACKAGE_DIR/lib" -type f -exec strip -s {} \;
find "$STK_PACKAGE_DIR/lib-64" -type f -exec strip -s {} \;

mv "$STK_PACKAGE_DIR/lib/libgcc_s.so.1" "$STK_PACKAGE_DIR/lib/libgcc_s.so.1-orig"
mv "$STK_PACKAGE_DIR/lib-64/libgcc_s.so.1" "$STK_PACKAGE_DIR/lib-64/libgcc_s.so.1-orig"
mv "$STK_PACKAGE_DIR/lib/libstdc++.so.6" "$STK_PACKAGE_DIR/lib/libstdc++.so.6-orig"
mv "$STK_PACKAGE_DIR/lib-64/libstdc++.so.6" "$STK_PACKAGE_DIR/lib-64/libstdc++.so.6-orig"

write_run_game_sh "$STK_PACKAGE_DIR"

cp "$STKCODE_DIR/$BUILD_DIR_32BIT-symbols/bin/supertuxkart" "$STK_INSTALL_DIR/supertuxkart-$STK_VERSION-linux32-symbols"
cp "$STKCODE_DIR/$BUILD_DIR_64BIT-symbols/bin/supertuxkart" "$STK_INSTALL_DIR/supertuxkart-$STK_VERSION-linux64-symbols"
cp "$STKEDITOR_DIR/$BUILD_DIR_32BIT-symbols/bin/supertuxkart-editor" "$STK_INSTALL_DIR/supertuxkart-editor-$STK_VERSION-linux32-symbols"
cp "$STKEDITOR_DIR/$BUILD_DIR_64BIT-symbols/bin/supertuxkart-editor" "$STK_INSTALL_DIR/supertuxkart-editor-$STK_VERSION-linux64-symbols"

cp -a "$STKCODE_DIR/$BUILD_DIR_32BIT/bin/supertuxkart" "$STK_PACKAGE_DIR/bin/"
cp -a "$STKCODE_DIR/$BUILD_DIR_64BIT/bin/supertuxkart" "$STK_PACKAGE_DIR/bin-64/"
cp -a "$STKEDITOR_DIR/$BUILD_DIR_32BIT/bin/supertuxkart-editor" "$STK_PACKAGE_DIR/bin/"
cp -a "$STKEDITOR_DIR/$BUILD_DIR_64BIT/bin/supertuxkart-editor" "$STK_PACKAGE_DIR/bin-64/"

cp -a "$STKCODE_DIR/data/." "$STK_PACKAGE_DIR/data"
cp -a "$STKASSETS_DIR/editor" "$STK_PACKAGE_DIR/data/"
cp -a "$STKASSETS_DIR/karts" "$STK_PACKAGE_DIR/data/"
cp -a "$STKASSETS_DIR/library" "$STK_PACKAGE_DIR/data/"
cp -a "$STKASSETS_DIR/models" "$STK_PACKAGE_DIR/data/"
cp -a "$STKASSETS_DIR/music" "$STK_PACKAGE_DIR/data/"
cp -a "$STKASSETS_DIR/sfx" "$STK_PACKAGE_DIR/data/"
cp -a "$STKASSETS_DIR/textures" "$STK_PACKAGE_DIR/data/"
cp -a "$STKASSETS_DIR/tracks" "$STK_PACKAGE_DIR/data/"
cp -a "$STKASSETS_DIR/licenses.txt" "$STK_PACKAGE_DIR/data/"

chmod a+rwx "$STK_PACKAGE_DIR" -R
find "$STK_PACKAGE_DIR" -type f -exec chmod a-x {} \;
find "$STK_PACKAGE_DIR/bin" -type f -exec chmod a+x {} \;
find "$STK_PACKAGE_DIR/bin-64" -type f -exec chmod a+x {} \;
chmod a+x "$STK_PACKAGE_DIR/run_game.sh"

test_package "$STK_PACKAGE_DIR"

# Compress package

echo "Compress package..."

cd "$STK_INSTALL_DIR"
tar cf - "SuperTuxKart-$STK_VERSION-linux" | xz -z -e -f - > "SuperTuxKart-$STK_VERSION-linux.tar.xz"
cd -

echo "Success."
