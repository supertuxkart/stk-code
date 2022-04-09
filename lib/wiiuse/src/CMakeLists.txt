if(NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
endif()

# Modified for STK to avoid compiler warnings with MinGW
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
	classic.c
	dynamics.c
	events.c
	guitar_hero_3.c
	io.c
	ir.c
	nunchuk.c
	wiiuse.c
	wiiboard.c
	classic.h
	definitions.h
	definitions_os.h
	dynamics.h
	events.h
	guitar_hero_3.h
	motion_plus.h
	motion_plus.c
	io.h
	ir.h
	nunchuk.h
	os.h
	util.c
	wiiuse_internal.h
	wiiboard.h)
set(API
	include/wiiuse.h)

if(WIN32)
	list(APPEND SOURCES os_win.c)
	set(CMAKE_DEBUG_POSTFIX _debug)
elseif(APPLE)
	set(MAC_OBJC_SOURCES
		os_mac/os_mac.m
		os_mac/os_mac_interface.m
		os_mac/os_mac_find.m)
	set(MAC_SOURCES
		os_mac/os_mac.h
		os_mac/os_mac.c
		${MAC_OBJC_SOURCES})
	list(APPEND SOURCES ${MAC_SOURCES})
	# make sure we use the gcc for Objective-C files as well so that the
	# sysroot and deployment target arguments are correctly passed to the compiler
	set_source_files_properties(${MAC_OBJC_SOURCES} PROPERTIES LANGUAGE C)
else()
	list(APPEND SOURCES os_nix.c)
endif()

if(MSVC)
	list(APPEND API include/wiiuse_msvcstdint.h)
endif()

add_definitions(-DWIIUSE_COMPILE_LIB)

add_library(wiiuse ${WIIUSE_LIB_TYPE} ${SOURCES} ${API})

if(WIN32)
	target_link_libraries(wiiuse ws2_32 setupapi ${WINHID_LIBRARIES})
elseif(LINUX)
	target_link_libraries(wiiuse m rt ${BLUEZ_LIBRARIES})
elseif(APPLE)
	# link libraries
	find_library(IOBLUETOOTH_FRAMEWORK
		NAMES
		IOBluetooth)
	find_library(COREFOUNDATION_FRAMEWORK
		NAMES
		CoreFoundation)
	find_library(FOUNDATION_FRAMEWORK
		NAMES
		Foundation)
	target_link_libraries(wiiuse ${IOBLUETOOTH_FRAMEWORK} ${COREFOUNDATION_FRAMEWORK} ${FOUNDATION_FRAMEWORK})

	# do not link Objective-C runtime with clang
	set_target_properties(wiiuse PROPERTIES XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME "NO")
endif()

set_property(TARGET
	wiiuse
	PROPERTY
	PUBLIC_HEADER
	${API})

# Modified for STK, see https://github.com/supertuxkart/stk-code/commit/15e741fa0490f55718bd7f009ef83f0540b9d0a1#diff-a3e272598233e89e4b577a434cc2a89d
if(MSVC)
	set_property(TARGET wiiuse APPEND PROPERTY
	  COMPILE_DEFINITIONS $<$<CONFIG:Debug>:WITH_WIIUSE_DEBUG>
	)
endif()

install(TARGETS wiiuse
	RUNTIME DESTINATION bin
	COMPONENT runtime
	ARCHIVE
	DESTINATION
	${CMAKE_INSTALL_LIBDIR}
	COMPONENT
	development
	LIBRARY
	DESTINATION
	${CMAKE_INSTALL_LIBDIR}
	COMPONENT
	runtime
	PUBLIC_HEADER
	DESTINATION
	include
	COMPONENT
	development)
