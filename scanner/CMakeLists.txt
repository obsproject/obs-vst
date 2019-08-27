project(obs-vst-scanner)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${OBS_JANSSON_INCLUDE_DIRS})

set(obs-vst-scanner_SOURCES
	scanner.cpp)

set(obs-vst-scanner_HEADERS
	scanner.h)

add_executable(obs-vst-scanner
	${obs-vst-scanner_SOURCES}
	${obs-vst-scanner_HEADERS})

target_link_libraries(obs-vst-scanner
	libobs
	${OBS_JANSSON_IMPORT})

install_obs_core(obs-vst-scanner)
