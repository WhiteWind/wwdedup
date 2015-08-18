project(fuse-dedup)
cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")

add_definitions("-D_FILE_OFFSET_BITS=64" "-D_REENTRANT" "-D_POSIX_PTHREAD_SEMANTICS" "-DFUSE_USE_VERSION=26")

include("PrecompiledHeader.cmake")

FIND_PACKAGE(Boost 1.40 COMPONENTS system filesystem thread date_time REQUIRED)
FIND_PACKAGE(Sqlite3 REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR} ${SQLITE3_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
include_directories(SYSTEM libcds)

include_directories(fuse-dedup easySQLite)

aux_source_directory(. SRC_LIST)
aux_source_directory(easySQLite SRC_LIST)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Winvalid-pch -fexceptions -fno-strict-aliasing -mcx16")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -D_DEBUG -fno-omit-frame-pointer -fPIE -pie")
#-fsanitize=thread

aux_source_directory(libcds/src CDS_SRC_LIST)

IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RELEASE)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

#ADD_PCH_RULE(${PROJECT_NAME} pre.h SRC_LIST)
set_precompiled_header(${PROJECT_NAME} CXX pre.h prech)

add_executable(${PROJECT_NAME} ${SRC_LIST})

use_precompiled_header(${PROJECT_NAME} ${SRC_LIST})

add_library(cds SHARED ${CDS_SRC_LIST})

target_link_libraries(cds -ldl -lpthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_DATE_TIME_LIBRARY})

target_link_libraries(fuse-dedup -lfuse -ldl -lpthread ${SQLITE3_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

target_link_libraries(fuse-dedup cds)
