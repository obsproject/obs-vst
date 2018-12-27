# Is there some way to set this for both in-tree and out-of-tree building
# to match OBS-Studio's cmake minimum?
cmake_minimum_required(VERSION 2.8.12)

if(DISABLE_UI)
	message(STATUS "UI disabled, so the vst plugin is also disabled.")
	return()
endif()

project(obs-vst)

option(VST_USE_BUNDLED_HEADERS "Build with Bundled Headers" ON)

# Detect if module is being built in tree or not.
get_filename_component(PARENT_DIR PROJECT_SOURCE_DIR DIRECTORY)
if(${PARENT_DIR} MATCHES "plugins")
	option(VST_BUILD_OUT_OF_TREE "Enable Out-of-Tree builds" OFF)
else()
	option(VST_BUILD_OUT_OF_TREE "Enable Out-of-Tree builds" ON)
endif()

if(NOT VST_BUILD_OUT_OF_TREE)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
else()
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTOUIC ON)	
endif()

find_package(Qt5Widgets REQUIRED)

if(VST_BUILD_OUT_OF_TREE)
	include(external/FindLibObs.cmake)
	find_package(LibObs REQUIRED)
	find_package(CXX11 REQUIRED)
endif()

if(APPLE)
	find_package(Qt5MacExtras REQUIRED)
	find_library(FOUNDATION_FRAMEWORK Foundation)
	find_library(COCOA_FRAMEWORK Cocoa)
endif(APPLE)

if(VST_USE_BUNDLED_HEADERS)
	message(STATUS "Using the bundled VST header.")
	include_directories(vst_header)
	set(vst_HEADER
		vst_header/aeffectx.h)
else()
	set(VST_INCLUDE_DIR "" CACHE PATH
		"Path to Steinburg headers (e.g. C:/VST3 SDK/pluginterfaces/vst2.x)")

	message(WARNING "You should only use the Steinburg headers for debugging or local
	 builds. It is illegal to distribute the Steinburg headers with anything, and
	 possibly against the GPL to distribute the binaries from the resultant compile.")
	include_directories(${VST_INCLUDE_DIR})
	set(vst_HEADER
		${VST_INCLUDE_DIR}/aeffectx.h)
endif()

set(obs-vst_SOURCES
	obs-vst.cpp
	VSTPlugin.cpp
	EditorWidget.cpp)

if(APPLE)
	list(APPEND obs-vst_SOURCES
		mac/VSTPlugin-osx.mm
		mac/EditorWidget-osx.mm)

elseif(WIN32)
	list(APPEND obs-vst_SOURCES
		win/VSTPlugin-win.cpp
		win/EditorWidget-win.cpp)

elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	list (APPEND obs-vst_SOURCES
		linux/VSTPlugin-linux.cpp
		linux/EditorWidget-linux.cpp)
endif()

list(APPEND obs-vst_HEADERS
	headers/vst-plugin-callbacks.hpp
	headers/EditorWidget.h
	headers/VSTPlugin.h)

add_library(obs-vst MODULE
	${obs-vst_SOURCES}
	${obs-vst_HEADERS}
	${vst-HEADER})

include_directories(
		"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
		${Qt5Widgets_INCLUDES})

target_link_libraries(obs-vst
	libobs
	Qt5::Widgets)

if(APPLE)
	target_link_libraries(obs-vst
		${COCOA_FRAMEWORK}
		${FOUNDATION_FRAMEWORK}
		Qt5::MacExtras)
endif(APPLE)

if(NOT VST_BUILD_OUT_OF_TREE)
	install_obs_plugin_with_data(obs-vst data)
	return()
else()
	# Additional commands to install the module in the correct place.
	# Find all the translation files so we can copy them to the correct
	# place later on.
	file(GLOB VST_TRANSLATION_FILES "data/locale/*.ini")
	
	# Set base/non-platform-specific build options first:
	if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE RelWithDebInfo)
	endif()
	
	# Set ARCH depth
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(ARCH 64)
	else()
		set(ARCH 32)
	endif()
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS}")
	set(CMAKE_C_FLAGS_DEBUG "-DDEBUG=1 -D_DEBUG=1 ${CMAKE_C_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG=1 -D_DEBUG=1 ${CMAKE_C_FLAGS_DEBUG}")
	
	# Set non-platform-specific compiler-specific options next
	if(${CMAKE_C_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_COMPILER_IS_CLANG TRUE)
	endif()
	
	if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
		set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wvla -Wno-unused-function -Werror-implicit-function-declaration -Wno-missing-field-initializers ${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
		set(CMAKE_C_FLAGS "-Wall -Wextra -Wvla -Wno-unused-function -Werror-implicit-function-declaration -Wno-missing-braces -Wno-missing-field-initializers ${CMAKE_C_FLAGS} -std=gnu99 -fno-strict-aliasing")
	elseif(MSVC)
		if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
			string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
		endif()
		# Disable pointless constant condition warnings
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127 /wd4201 /wd4456 /wd4457 /wd4458 /wd4459 /wd4595")

		if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
			set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
		endif()
	
	elseif(MINGW)
		set(CMAKE_WIDL "widl" CACHE STRING "wine IDL header file generation program")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_WIN32_WINNT=0x0600 -DWINVER=0x0600")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0600 -DWINVER=0x0600")
	endif()

	# Now set OS/platform-specific options
	# Windows
	if(WIN32)
		add_definitions(-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
		string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
		message("${CMAKE_CXX_FLAGS_RELEASE}")

		find_package(w32-pthreads REQUIRED)

		target_link_libraries(obs-vst
				"${OBS_FRONTEND_LIB}"
				w32-pthreads)
	endif()

	# OSX
	if(APPLE)
		set(CMAKE_MACOSX_RPATH TRUE)
		set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
		list(APPEND CMAKE_INSTALL_RPATH "@loader_path/" "@executable_path/")
		
		option(USE_LIBC++ "Use libc++ instead of libstdc++" ${APPLE})
		if(USE_LIBC++)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		endif()
		
		set_target_properties(obs-vst PROPERTIES PREFIX "")
		target_link_libraries(obs-vst
				"${OBS_FRONTEND_LIB}")
	endif()

	# Linux
	if(UNIX AND NOT APPLE)
		if(ARCH EQUAL 64)
			set(ARCH_NAME "x86_64")
		else()
			set(ARCH_NAME "i686")
		endif()
		
		option(USE_XDG "Utilize XDG Base Directory Specification" ON)
		if(USE_XDG)
			add_definitions(-DUSE_XDG)
		endif()

		if(NOT UNIX_STRUCTURE)
			list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN")
		endif()

		set_target_properties(obs-vst PROPERTIES PREFIX "")

		target_link_libraries(obs-vst
				obs-frontend-api)

		install(TARGETS obs-vst
				LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/obs-plugins)
		install(FILES ${VST_TRANSLATION_FILES}
				DESTINATION "${CMAKE_INSTALL_PREFIX}/share/obs/obs-plugins/obs-vst/locale")
	endif()
endif()
