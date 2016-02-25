NDK_PATH=android-ndk-r10e/
# target: all - Default target. Build and create the apk.
all: build apk

# target: build - Build the native code
build: obj/freetype.stamp jni/ifaddrs/ifaddrs.h
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
	mkdir -p obj/freetype
	cd jni/freetype && \
	./configure --host=arm-linux-androideabi --prefix=/freetype --without-zlib --with-png=no --with-harfbuzz=no && \
	make -j4 && \
	make install DESTDIR=`pwd`/../../obj/freetype
	touch obj/freetype.stamp

jni/ifaddrs/ifaddrs.h:
	cd jni && git clone https://github.com/morristech/android-ifaddrs.git ifaddrs
