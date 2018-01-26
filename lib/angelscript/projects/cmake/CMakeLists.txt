cmake_minimum_required(VERSION 2.6)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
cmake_policy(SET CMP0003 NEW)

project(angelscript)

option(BUILD_SHARED_LIBS "Build shared library" OFF)

if(APPLE)
    option(BUILD_FRAMEWORK "Build Framework bundle for OSX" OFF)
endif()

file(READ ../../include/angelscript.h ANGELSCRIPT_H)
string(REGEX MATCH "#define ANGELSCRIPT_VERSION_STRING \"([0-9]*).([0-9]*).([0-9]*)" ANGELSCRIPT_VERSION_REGEX ${ANGELSCRIPT_H})
set(ANGELSCRIPT_VERSION_MAJOR ${CMAKE_MATCH_1})
set(ANGELSCRIPT_VERSION_MINOR ${CMAKE_MATCH_2})
set(ANGELSCRIPT_VERSION_PATCH ${CMAKE_MATCH_3})
set(PROJECT_VERSION ${ANGELSCRIPT_VERSION_MAJOR}.${ANGELSCRIPT_VERSION_MINOR}.${ANGELSCRIPT_VERSION_PATCH})

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

if(MSVC AND CMAKE_CL_64)
    enable_language(ASM_MASM)
    if(CMAKE_ASM_MASM_COMPILER_WORKS)
        set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ../../source/as_callfunc_x64_msvc_asm.asm)
    else()
        message(FATAL ERROR "MSVC x86_64 target requires a working assembler")
    endif()
endif()

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^arm")
    enable_language(ASM)
    if(CMAKE_ASM_COMPILER_WORKS)
        set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ../../source/as_callfunc_arm.cpp ../../source/as_callfunc_arm_gcc.S)
        set_property(SOURCE ../../source/as_callfunc_arm_gcc.S APPEND PROPERTY COMPILE_FLAGS " -Wa,-mimplicit-it=always")
    else()
        message(FATAL ERROR "ARM target requires a working assembler")
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)    
endif()

add_definitions(-DANGELSCRIPT_EXPORT -D_LIB)

# Fix x64 issues on Linux
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" AND NOT APPLE)
    add_definitions(-fPIC)
endif()

if(NOT BUILD_FRAMEWORK)
    set(ANGELSCRIPT_LIBRARY_NAME angelscript)
else()
    set(ANGELSCRIPT_LIBRARY_NAME Angelscript) # OS X frameworks should have capitalized name
    set(BUILD_SHARED_LIBS TRUE)
endif()
set(ANGELSCRIPT_LIBRARY_NAME ${ANGELSCRIPT_LIBRARY_NAME} CACHE STRING "" FORCE)

add_library(${ANGELSCRIPT_LIBRARY_NAME} ${ANGELSCRIPT_SOURCE} ${ANGELSCRIPT_HEADERS})
# Don't override the default library output path to avoid conflicts when building for multiple target platforms
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../lib)
target_link_libraries(${ANGELSCRIPT_LIBRARY_NAME} ${CMAKE_THREAD_LIBS_INIT})

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

if(MSVC)
    set_target_properties(${ANGELSCRIPT_LIBRARY_NAME} PROPERTIES COMPILE_FLAGS "/MP")
endif()

# Don't override the default runtime output path to avoid conflicts when building for multiple target platforms
#set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin)


