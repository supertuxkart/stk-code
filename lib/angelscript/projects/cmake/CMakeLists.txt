cmake_minimum_required(VERSION 3.5)

# STK Fix llvm mingw crashes
if (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(USE_AS_MAX_PORTABILITY 1)
  ADD_DEFINITIONS(-DAS_MAX_PORTABILITY)
endif()

project(angelscript)

option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(AS_NO_EXCEPTIONS "Disable exception handling in script context" OFF)

if(MSVC)
    option(MSVC_COMPILE_FLAGS "Compiler flags to use with MSVC" "/MP")
endif()

if(APPLE)
    option(BUILD_FRAMEWORK "Build Framework bundle for OSX" OFF)
endif()

file(READ ../../include/angelscript.h ANGELSCRIPT_H)
string(REGEX MATCH "#define ANGELSCRIPT_VERSION_STRING \"([0-9]*).([0-9]*).([0-9]*)" ANGELSCRIPT_VERSION_REGEX ${ANGELSCRIPT_H})
set(ANGELSCRIPT_VERSION_MAJOR ${CMAKE_MATCH_1})
set(ANGELSCRIPT_VERSION_MINOR ${CMAKE_MATCH_2})
set(ANGELSCRIPT_VERSION_PATCH ${CMAKE_MATCH_3})
set(PROJECT_VERSION ${ANGELSCRIPT_VERSION_MAJOR}.${ANGELSCRIPT_VERSION_MINOR}.${ANGELSCRIPT_VERSION_PATCH})

set(ARCHFLAGS "${CMAKE_SYSTEM_PROCESSOR}")
if (APPLE AND NOT IOS)
  if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    set(ARCHFLAGS "aarch64")
  endif ()
endif ()

message(STATUS "Configuring angelscript ${PROJECT_VERSION}")

find_package(Threads)

set(ANGELSCRIPT_HEADERS
    ../../include/angelscript.h
    ../../source/as_array.h
    ../../source/as_builder.h
    ../../source/as_bytecode.h
    ../../source/as_callfunc.h
    ../../source/as_compiler.h
    ../../source/as_config.h
    ../../source/as_configgroup.h
    ../../source/as_context.h
    ../../source/as_criticalsection.h
    ../../source/as_datatype.h
    ../../source/as_debug.h
    ../../source/as_generic.h
    ../../source/as_map.h
    ../../source/as_memory.h
    ../../source/as_module.h
    ../../source/as_objecttype.h
    ../../source/as_outputbuffer.h
    ../../source/as_parser.h
    ../../source/as_property.h
    ../../source/as_restore.h
    ../../source/as_scriptcode.h
    ../../source/as_scriptengine.h
    ../../source/as_scriptfunction.h
    ../../source/as_scriptnode.h
    ../../source/as_scriptobject.h
    ../../source/as_string.h
    ../../source/as_string_util.h
    ../../source/as_texts.h
    ../../source/as_thread.h
    ../../source/as_tokendef.h
    ../../source/as_tokenizer.h
    ../../source/as_typeinfo.h
    ../../source/as_variablescope.h
)

set(ANGELSCRIPT_SOURCE
    ../../source/as_atomic.cpp
    ../../source/as_builder.cpp
    ../../source/as_bytecode.cpp
    ../../source/as_callfunc.cpp
    ../../source/as_callfunc_mips.cpp
    ../../source/as_callfunc_x86.cpp
    ../../source/as_callfunc_x64_gcc.cpp
    ../../source/as_callfunc_x64_msvc.cpp
    ../../source/as_callfunc_x64_mingw.cpp
    ../../source/as_compiler.cpp
    ../../source/as_configgroup.cpp
    ../../source/as_context.cpp
    ../../source/as_datatype.cpp
    ../../source/as_gc.cpp
    ../../source/as_generic.cpp
    ../../source/as_globalproperty.cpp
    ../../source/as_memory.cpp
    ../../source/as_module.cpp
    ../../source/as_objecttype.cpp
    ../../source/as_outputbuffer.cpp
    ../../source/as_parser.cpp
    ../../source/as_restore.cpp
    ../../source/as_scriptcode.cpp
    ../../source/as_scriptengine.cpp
    ../../source/as_scriptfunction.cpp
    ../../source/as_scriptnode.cpp
    ../../source/as_scriptobject.cpp
    ../../source/as_string.cpp
    ../../source/as_string_util.cpp
    ../../source/as_thread.cpp
    ../../source/as_tokenizer.cpp
    ../../source/as_typeinfo.cpp
    ../../source/as_variablescope.cpp
)

# STK
set(BUILD_ARCH "unknown")
if (CMAKE_GENERATOR_PLATFORM)
    set(BUILD_ARCH "${CMAKE_GENERATOR_PLATFORM}")
endif()

if (MSVC AND (${CMAKE_GENERATOR} MATCHES "(ARM)" OR ${BUILD_ARCH} MATCHES "(ARM)"))
  set(USE_AS_MAX_PORTABILITY 1)
  ADD_DEFINITIONS(-DAS_MAX_PORTABILITY)
endif()

if(MSVC AND CMAKE_CL_64 AND NOT ${CMAKE_GENERATOR_PLATFORM} MATCHES "(ARM64)")
    enable_language(ASM_MASM)
    if(CMAKE_ASM_MASM_COMPILER_WORKS)
        set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ../../source/as_callfunc_x64_msvc_asm.asm)
    else()
        message(FATAL ERROR "MSVC x86_64 target requires a working assembler")
    endif()
endif()

if(MSVC AND CMAKE_CL_64 AND NOT ${BUILD_ARCH} MATCHES "(ARM64)")
    enable_language(ASM_MASM)
    if(CMAKE_ASM_MASM_COMPILER_WORKS)
        set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ../../source/as_callfunc_x64_msvc_asm.asm)
    else()
        message(FATAL ERROR "MSVC x86_64 target requires a working assembler")
    endif()
endif()

if(${ARCHFLAGS} MATCHES "^arm")
    enable_language(ASM)
    if (NOT USE_AS_MAX_PORTABILITY)
        if(CMAKE_ASM_COMPILER_WORKS)
            set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ../../source/as_callfunc_arm.cpp ../../source/as_callfunc_arm_gcc.S)
            if (NOT APPLE)
                set_property(SOURCE ../../source/as_callfunc_arm_gcc.S APPEND PROPERTY COMPILE_FLAGS " -Wa,-mimplicit-it=always")
            endif()
        else()
            message(FATAL ERROR "ARM target requires a working assembler")
        endif()
    endif()
endif()

if(${ARCHFLAGS} MATCHES "^aarch64")
    enable_language(ASM)
    ADD_DEFINITIONS(-DAS_MAX_PORTABILITY)
endif()

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")  
endif()

if(NOT BUILD_FRAMEWORK)
    set(ANGELSCRIPT_LIBRARY_NAME angelscript)
else()
    set(ANGELSCRIPT_LIBRARY_NAME Angelscript) # OS X frameworks should have capitalized name
    set(BUILD_SHARED_LIBS TRUE)
endif()
set(ANGELSCRIPT_LIBRARY_NAME ${ANGELSCRIPT_LIBRARY_NAME} CACHE STRING "" FORCE)

add_library(${ANGELSCRIPT_LIBRARY_NAME} ${ANGELSCRIPT_SOURCE} ${ANGELSCRIPT_HEADERS})

target_include_directories(${ANGELSCRIPT_LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

if(MSVC)
    target_compile_definitions(${ANGELSCRIPT_LIBRARY_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS)    
endif()

target_compile_definitions(${ANGELSCRIPT_LIBRARY_NAME} PRIVATE -DANGELSCRIPT_EXPORT -D_LIB)

if(AS_NO_EXCEPTIONS)
	target_compile_definitions(${ANGELSCRIPT_LIBRARY_NAME} PRIVATE AS_NO_EXCEPTIONS)
endif()

# Fix x64 issues on Linux
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" AND UNIX AND NOT APPLE AND NOT WIN32)
    target_compile_options(${ANGELSCRIPT_LIBRARY_NAME} PRIVATE -fPIC)
endif()

# Don't override the default library output path to avoid conflicts when building for multiple target platforms
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../lib)
target_link_libraries(${ANGELSCRIPT_LIBRARY_NAME} Threads::Threads)

set_target_properties(${ANGELSCRIPT_LIBRARY_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

if(BUILD_FRAMEWORK)
    set_target_properties(${ANGELSCRIPT_LIBRARY_NAME} PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION ${PROJECT_VERSION}
        MACOSX_FRAMEWORK_IDENTIFIER com.angelcode.Angelscript
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_FRAMEWORK_BUNDLE_VERSION ${PROJECT_VERSION}
        XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
        PUBLIC_HEADER ../../include/angelscript.h
    )
endif()

if(MSVC AND MSVC_COMPILE_FLAGS)
	target_compile_options(${ANGELSCRIPT_LIBRARY_NAME} PRIVATE "${MSVC_COMPILE_FLAGS}")
endif()

# Don't override the default runtime output path to avoid conflicts when building for multiple target platforms
#set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin)

#See https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#creating-packages for a detailed explanation about this part
install(TARGETS ${ANGELSCRIPT_LIBRARY_NAME} EXPORT AngelscriptTargets
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
)

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/angelscript.h
	DESTINATION include
	COMPONENT Devel
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/Angelscript/AngelscriptConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)

export(EXPORT AngelscriptTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/Angelscript/AngelscriptTargets.cmake"
	NAMESPACE Angelscript::
)
configure_file(cmake/AngelscriptConfig.cmake
	"${CMAKE_CURRENT_BINARY_DIR}/Angelscript/AngelscriptConfig.cmake"
	COPYONLY
)

set(ConfigPackageLocation lib/cmake/Angelscript)
install(EXPORT AngelscriptTargets
	FILE AngelscriptTargets.cmake
	NAMESPACE Angelscript::
	DESTINATION ${ConfigPackageLocation}
)
install(
	FILES
		cmake/AngelscriptConfig.cmake
		"${CMAKE_CURRENT_BINARY_DIR}/Angelscript/AngelscriptConfigVersion.cmake"
	DESTINATION ${ConfigPackageLocation}
	COMPONENT Devel
)

