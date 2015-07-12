project(rogue)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -ggdb -Wl,-rpath . ${CMAKE_CXX_FLAGS}")

aux_source_directory(./src SRC)
include_directories(./include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(${PROJECT_NAME} ${SRC})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include_directories(/usr/local/include/SDL2)
	target_link_libraries(${PROJECT_NAME} /usr/local/lib/libSDL2.a libtcod.dylib libtcodxx.dylib)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME} libtcod.so libtcodxx.so)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(${PROJECT_NAME} ltcod-mingw.dll)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")