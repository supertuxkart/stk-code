NDK_PATH=android-ndk-r10e/
NDK_STANDALONE=`pwd`/${NDK_PATH}
export PATH :=$(shell pwd)/obj/bin:${PATH}
# target: all - Default target. Build and create the apk.
all: build apk

# target: build - Build the native code
build: obj/freetype.stamp jni/ifaddrs/ifaddrs.h jni/jpeglib/jpeglib.h
	NDK_CCACHE=ccache ${NDK_PATH}/ndk-build -j3

# target: apk - Make a debug APK
apk:
	ant debug

# target: install - Install the debug APK to the plugged device/emulator
install:
	ant installd

# target: help - Display callable targets.
help:
	egrep "^# target:" [Mm]akefile

run:
	adb shell am start -n org.supertuxkart.stk/android.app.NativeActivity

obj/freetype.stamp:
	mkdir -p obj/deps && cd obj/deps && wget http://mirror6.layerjet.com/nongnu//freetype/freetype-2.6.tar.gz && tar -xzf freetype-2.6.tar.gz
	mkdir -p obj/freetype
	${NDK_PATH}/build/tools/make-standalone-toolchain.sh \
	  --platform=android-21 --install-dir=`pwd`/obj/ --arch=arm
	cd obj/deps/freetype-2.6 && \
    ./configure --host=arm-linux-androideabi --prefix=/freetype --without-zlib --with-png=no --with-harfbuzz=no && \
	make -j4 && \
	make install DESTDIR=`pwd`/../../freetype && \
	echo ${PATH}
	touch obj/freetype.stamp

jni/ifaddrs/ifaddrs.h:
	cd jni && git clone https://github.com/morristech/android-ifaddrs.git ifaddrs

jni/jpeglib/jpeglib.h:
	git clone https://github.com/folecr/jpeg8d jni/jpeglib


#wget http://mirror6.layerjet.com/nongnu//freetype/freetype-2.6.tar.gz
