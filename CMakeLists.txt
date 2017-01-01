project(obs-vst)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(VST REQUIRED)
find_package(Qt5Widgets REQUIRED)

if(APPLE)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    find_library(COCOA_FRAMEWORK Cocoa)
endif(APPLE)

set(obs-vst_SOURCES
        obs-vst.cpp
        VSTPlugin.cpp
        EditorWidget.cpp)

if(APPLE)
    list(APPEND obs-vst_SOURCES
            VSTPlugin-osx.mm
            EditorWidget-osx.mm)
elseif(WIN32)
    list(APPEND obs-vst_SOURCES
            VSTPlugin-win.cpp
            EditorWidget-win.cpp)
endif()

set(obs-vst_HEADERS
        vst-plugin-callbacks.hpp
        EditorWidget.h
        VSTPlugin.h)

add_library(obs-vst MODULE
        ${obs-vst_SOURCES}
        ${obs-vst_HEADERS})

target_include_directories(obs-vst PUBLIC
        ${VST_INCLUDE_DIR})

target_link_libraries(obs-vst
        Qt5::Widgets
        libobs)

if(APPLE)
    target_link_libraries(obs-vst
            ${COCOA_FRAMEWORK}
            ${FOUNDATION_FRAMEWORK})
endif(APPLE)

install_obs_plugin_with_data(obs-vst data)