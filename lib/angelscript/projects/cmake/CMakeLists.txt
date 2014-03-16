cmake_minimum_required(VERSION 2.6)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
cmake_policy(SET CMP0003 NEW)

project(Angelscript)

set(ANGELSCRIPT_SOURCE
        ../../source/as_atomic.cpp
        ../../source/as_builder.cpp
        ../../source/as_bytecode.cpp
        ../../source/as_callfunc.cpp
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

if(ANDROID)
        enable_language(ASM)
        if(CMAKE_ASM_COMPILER_WORKS)
                set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ../../source/as_callfunc_arm.cpp ../../source/as_callfunc_arm_gcc.S)
        else()
                message(FATAL ERROR "Android target requires a working assembler")
        endif(CMAKE_ASM_COMPILER_WORKS)
endif()

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

add_definitions("-D_CRT_SECURE_NO_WARNINGS -DANGELSCRIPT_EXPORT -D_LIB")

# Fix x64 issues on Linux
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" AND NOT APPLE)
        add_definitions(-fPIC)
endif()

add_library(Angelscript STATIC ${ANGELSCRIPT_SOURCE} ${ANGELSCRIPT_HEADERS})
set(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/../../lib)

find_package(Threads)
target_link_libraries(Angelscript ${CMAKE_THREAD_LIBS_INIT})

if(MSVC)
        set_target_properties(Angelscript PROPERTIES COMPILE_FLAGS "/MP")
endif(MSVC)

set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin)

add_subdirectory(../../../samples/game/projects/cmake/ ./game)

