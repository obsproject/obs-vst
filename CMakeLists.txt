project(obs-vst)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Qt5Widgets REQUIRED)

option(VST_USE_BUNDLED_HEADERS "Build with Bundled Headers" ON)

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

if(APPLE)
	find_library(FOUNDATION_FRAMEWORK Foundation)
	find_library(COCOA_FRAMEWORK Cocoa)
endif(APPLE)

include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/UI/obs-frontend-api")

set(obs-vst_SOURCES
	obs-vst.cpp
	VSTPlugin.cpp
	EditorWidget.cpp
	vst-settings.cpp)

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
	headers/VSTPlugin.h
	vst-settings.h)

qt5_wrap_ui(obs-vst_UI_HEADERS
	forms/vst-settings.ui)
include_directories(${CMAKE_BINARY_DIR}/plugins/obs-vst)

add_library(obs-vst MODULE
	${obs-vst_SOURCES}
	${obs-vst_HEADERS}
	${vst-HEADER}
	${obs-vst_UI_HEADERS})

target_link_libraries(obs-vst
	libobs
	obs-frontend-api
	Qt5::Widgets)

if(APPLE)
	target_link_libraries(obs-vst
		${COCOA_FRAMEWORK}
		${FOUNDATION_FRAMEWORK})
endif(APPLE)

install_obs_plugin_with_data(obs-vst data)