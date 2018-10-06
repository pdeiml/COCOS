cmake_minimum_required(VERSION 3.5)
#This is hacky and works only on the HPC Woodycluster in Erlangen
#set(CMAKE_C_COMPILER "/apps/gcc/gcc-7.2.0-x86_64/bin/gcc")
#set(CMAKE_CXX_COMPILER "/apps/gcc/gcc-7.2.0-x86_64/bin/g++")
project (Cocos)

option(WALL "-Wall" OFF)
option(WEXTRA "-Wextra" OFF)



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")
if(WALL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()
if(WEXTRA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()

include_directories("${Cocos_SOURCE_DIR}/include")

# Root
find_package(ROOT REQUIRED)
include_directories(${ROOT_INCLUDE_DIRS})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SOURCES)
add_library(Cocos SHARED ${SOURCES})

#aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src_exe EXE_NAMES)
set(EXE_NAMES COCOS COCOSdt_vun_hpc COCOShpc COCOSoutfile JobcreatorOutfile JobcreatorHPC)

foreach(EXE_NAME ${EXE_NAMES})
    add_executable(${EXE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src_exe/${EXE_NAME}.cpp)
    target_link_libraries(${EXE_NAME} Cocos ${ROOT_LIBRARIES})
endforeach()
