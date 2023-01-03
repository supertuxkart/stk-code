# Usage:
# cmake .. -DDEPS_PATH=/path/to/dependencies -DIOS_ASSETS=/path/to/generated/assets -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-ios-xcode.cmake -G Xcode
# Need to use ../android/generate_assets.sh for assets first
# In Xcode you need to choose Product -> Scheme -> supertuxkart
# And then Signing & Capabilities choose a suitable team
# You may need to use another bundle identifier as the current one is already used by STK team
# You can also use -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=xxxxxxxxxx to specify team

# Increase every upload to App store
SET(IOS_BUILD_VERSION 26)

# Get SDK path
execute_process(COMMAND xcodebuild -version -sdk iphoneos Path
    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT_IOS
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND xcodebuild -version -sdk iphonesimulator Path
    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT_SIMULATOR
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND xcodebuild -sdk ${CMAKE_OSX_SYSROOT} -version SDKVersion
    OUTPUT_VARIABLE SDK_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT DEFINED CMAKE_OSX_SYSROOT_IOS OR NOT DEFINED CMAKE_OSX_SYSROOT_SIMULATOR OR NOT DEFINED SDK_VERSION)
    message(FATAL_ERROR "Cannot find iphoneos or iphonesimulator sdk location and their version info.")
else()
    message(STATUS "Using SDK path: ${CMAKE_OSX_SYSROOT_IOS}.")
endif()
set(CMAKE_OSX_SYSROOT "iphoneos" CACHE INTERNAL "")

# Manaully set the values for both arm64 and simulator
set(JPEG_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libjpeg.a CACHE STRING "")
set(JPEG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")
set(ZLIB_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libz.a CACHE STRING "")
set(ZLIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")
set(PNG_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libpng16.a CACHE STRING "")
set(PNG_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")
set(PNG_PNG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")
set(OGGVORBIS_OGG_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libogg.a CACHE STRING "")
set(OGGVORBIS_OGG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include/ogg CACHE STRING "")
set(OGGVORBIS_VORBIS_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libvorbis.a CACHE STRING "")
set(OGGVORBIS_VORBIS_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include/vorbis CACHE STRING "")
set(OGGVORBIS_VORBISFILE_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libvorbisfile.a CACHE STRING "")
set(OGGVORBIS_VORBISFILE_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include/vorbis CACHE STRING "")
set(OGGVORBIS_VORBISENC_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libvorbisenc.a CACHE STRING "")
set(OGGVORBIS_VORBISENC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include/vorbis CACHE STRING "")
set(HARFBUZZ_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libharfbuzz.a CACHE STRING "")
set(HARFBUZZ_INCLUDEDIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")
set(FREETYPE_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libfreetype.a CACHE STRING "")
set(FREETYPE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include/freetype2 CACHE STRING "")
set(CURL_LIBRARY "${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libcurl.a;${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libmbedtls.a;${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libmbedx509.a;${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libmbedcrypto.a" CACHE STRING "")
set(CURL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")
set(MBEDCRYPTO_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libmbedcrypto.a CACHE STRING "")
set(MBEDTLS_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")
set(SDL2_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libSDL2.a CACHE STRING "")
set(SDL2_INCLUDEDIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include/SDL2 CACHE STRING "")
set(LIBSAMPLERATE_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libsamplerate.a CACHE STRING "")
set(LIBSAMPLERATE_INCLUDEDIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")
set(MOLTENVK_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libMoltenVK.a CACHE STRING "")
set(VULKAN_INCLUDEDIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")
set(LIBASTCENC_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libastcenc.a CACHE STRING "")
set(LIBASTCENC_INCLUDEDIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")
set(SHADERC_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies\${EFFECTIVE_PLATFORM_NAME}/lib/libshaderc_combined.a CACHE STRING "")
set(SHADERC_INCLUDEDIR ${PROJECT_SOURCE_DIR}/dependencies-iphoneos/include CACHE STRING "")

# For universal iOS and simulator
set(LIBRESOLV_LIBRARY -lresolv CACHE STRING "")
set(PTHREAD_LIBRARY -lpthread CACHE STRING "")

# Standard config
set(CMAKE_SYSTEM_VERSION ${SDK_VERSION} CACHE INTERNAL "")
set(UNIX TRUE CACHE BOOL "")
set(APPLE TRUE CACHE BOOL "")
set(IOS TRUE CACHE BOOL "")
set(CMAKE_AR ar CACHE FILEPATH "" FORCE)
set(CMAKE_RANLIB ranlib CACHE FILEPATH "" FORCE)
set(CMAKE_STRIP strip CACHE FILEPATH "" FORCE)

# Set the architectures for which to build
set(CMAKE_OSX_ARCHITECTURES "arm64 x86_64" CACHE STRING "Build architecture for iOS")
set(CMAKE_C_SIZEOF_DATA_PTR 8)
set(CMAKE_CXX_SIZEOF_DATA_PTR 8)
set(CMAKE_SYSTEM_PROCESSOR "arm64")
set(CMAKE_SYSTEM_NAME iOS CACHE INTERNAL "" FORCE)

# Change the type of target generated for try_compile() so it'll work when cross-compiling
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# All iOS/Darwin specific settings - some may be redundant.
set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
set(CMAKE_SHARED_MODULE_PREFIX "lib")
set(CMAKE_SHARED_MODULE_SUFFIX ".so")
set(CMAKE_C_COMPILER_ABI ELF)
set(CMAKE_CXX_COMPILER_ABI ELF)
set(CMAKE_C_HAS_ISYSROOT 1)
set(CMAKE_CXX_HAS_ISYSROOT 1)
set(CMAKE_MODULE_EXISTS 1)
set(CMAKE_DL_LIBS "")
set(CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG "-compatibility_version ")
set(CMAKE_C_OSX_CURRENT_VERSION_FLAG "-current_version ")
set(CMAKE_CXX_OSX_COMPATIBILITY_VERSION_FLAG "${CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG}")
set(CMAKE_CXX_OSX_CURRENT_VERSION_FLAG "${CMAKE_C_OSX_CURRENT_VERSION_FLAG}")

# Fixed variables in iOS STK
set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO CACHE INTERNAL "")
set(CMAKE_XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN YES CACHE INTERNAL "")
set(USE_WIIUSE FALSE CACHE BOOL "")
set(USE_SQLITE3 FALSE CACHE BOOL "")
set(USE_CRYPTO_OPENSSL FALSE CACHE BOOL "")
set(IOS_LAUNCHSCREEN ${DEPS_PATH}/ios-icon/launch_screen.storyboard)
set(IOS_IMAGES_XCASSETS ${DEPS_PATH}/ios-icon/Images.xcassets)
set(SDK_NAME_VERSION_FLAGS "-miphoneos-version-min=9.0")
set(CMAKE_OSX_DEPLOYMENT_TARGET 9.0 CACHE STRING "Set CMake deployment target" FORCE)

set(CMAKE_XCODE_ATTRIBUTE_ARCHS[sdk=iphoneos*] "arm64")
set(CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS[sdk=iphoneos*] "arm64")

set(CMAKE_XCODE_ATTRIBUTE_ARCHS[sdk=iphonesimulator*] "x86_64")
set(CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS[sdk=iphonesimulator*] "x86_64")

set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym" CACHE INTERNAL "")

# Replace the above two with these if you use an apple silicon
#set(CMAKE_XCODE_ATTRIBUTE_ARCHS[sdk=iphonesimulator*] "arm64")
#set(CMAKE_XCODE_ATTRIBUTE_VALID_ARCHS[sdk=iphonesimulator*] "arm64")
