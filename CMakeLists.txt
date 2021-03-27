project(obs-vst)

macro(find_qt)
  set(oneValueArgs VERSION)
  set(multiValueArgs COMPONENTS COMPONENTS_WIN COMPONENTS_MAC COMPONENTS_LINUX)
  cmake_parse_arguments(FIND_QT "" "${oneValueArgs}" "${multiValueArgs}"
                        ${ARGN})

  if(OS_WINDOWS)
    find_package(
      Qt${FIND_QT_VERSION}
      COMPONENTS ${FIND_QT_COMPONENTS} ${FIND_QT_COMPONENTS_WIN}
      REQUIRED)
  elseif(OS_MACOS)
    find_package(
      Qt${FIND_QT_VERSION}
      COMPONENTS ${FIND_QT_COMPONENTS} ${FIND_QT_COMPONENTS_MAC}
      REQUIRED)
  else()
    find_package(
      Qt${FIND_QT_VERSION}
      COMPONENTS ${FIND_QT_COMPONENTS} ${FIND_QT_COMPONENTS_LINUX}
      REQUIRED)
  endif()

  foreach(_COMPONENT IN LISTS FIND_QT_COMPONENTS FIND_QT_COMPONENTS_WIN
                              FIND_QT_COMPONENTS_MAC FIND_QT_COMPONENTS_LINUX)
    if(NOT TARGET Qt::${_COMPONENT} AND TARGET
                                        Qt${FIND_QT_VERSION}::${_COMPONENT})

      add_library(Qt::${_COMPONENT} INTERFACE IMPORTED)
      set_target_properties(
        Qt::${_COMPONENT} PROPERTIES INTERFACE_LINK_LIBRARIES
                                     "Qt${FIND_QT_VERSION}::${_COMPONENT}")
    endif()
  endforeach()
endmacro()

option(ENABLE_VST "Enable building OBS with VST plugin" ON)

if(NOT ENABLE_VST)
  message(STATUS "OBS:  DISABLED   obs-vst")
  return()
endif()

option(ENABLE_VST_BUNDLED_HEADERS "Build with Bundled Headers" ON)
mark_as_advanced(ENABLE_VST_BUNDLED_HEADERS)

if(NOT QT_VERSION)
  set(QT_VERSION
      "5"
      CACHE STRING
            "OBS Qt version [5, 6]" FORCE)
  set_property(CACHE QT_VERSION PROPERTY STRINGS 5 6)
endif()

add_library(obs-vst MODULE)
add_library(OBS::vst ALIAS obs-vst)

find_qt(VERSION ${QT_VERSION} COMPONENTS Widgets)

set_target_properties(
  obs-vst
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

target_include_directories(
  obs-vst PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                  ${CMAKE_CURRENT_BINARY_DIR})

target_sources(
  obs-vst
  PRIVATE obs-vst.cpp VSTPlugin.cpp EditorWidget.cpp
          headers/vst-plugin-callbacks.hpp headers/EditorWidget.h
          headers/VSTPlugin.h)

target_link_libraries(obs-vst PRIVATE OBS::libobs Qt::Widgets)

target_include_directories(
  obs-vst PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/headers)

target_compile_features(obs-vst PRIVATE cxx_std_17)

if(ENABLE_VST_BUNDLED_HEADERS)
  message(STATUS "OBS:    -        obs-vst uses bundled VST headers")

  target_sources(obs-vst PRIVATE vst_header/aeffectx.h)

  target_include_directories(obs-vst
                             PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vst_header)
else()
  set(VST_INCLUDE_DIR
      ""
      CACHE PATH
            "Path to Steinburg headers (e.g. C:/VST3 SDK/pluginterfaces/vst2.x)"
            FORCE)
  mark_as_advanced(VST_INCLUDE_DIR)

  message(
    WARNING
      "OBS: You should only use the Steinburg headers for debugging or local builds. "
      "It is illegal to distribute the Steinburg headers with anything, and "
      "possibly against the GPL to distribute the binaries from the resultant compile."
  )

  target_sources(obs-vst PRIVATE ${VST_INCLUDE_DIR}/aeffectx.h)
endif()

if(OS_MACOS)
  find_library(FOUNDATION Foundation)
  find_library(COCOA Cocoa)
  mark_as_advanced(COCOA FOUNDATION)

  target_sources(obs-vst PRIVATE mac/VSTPlugin-osx.mm mac/EditorWidget-osx.mm)

  target_link_libraries(obs-vst PRIVATE ${COCOA} ${FOUNDATION})

elseif(OS_WINDOWS)
  target_sources(obs-vst PRIVATE win/VSTPlugin-win.cpp win/EditorWidget-win.cpp)

  target_compile_definitions(obs-vst PRIVATE UNICODE _UNICODE)

elseif(OS_POSIX)
  target_sources(obs-vst PRIVATE linux/VSTPlugin-linux.cpp
                                 linux/EditorWidget-linux.cpp)

endif()

set_target_properties(obs-vst PROPERTIES FOLDER "plugins" PREFIX "")

setup_plugin_target(obs-vst)
