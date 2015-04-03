project(fuse-dedup)
cmake_minimum_required(VERSION 2.8)
add_definitions("-D_FILE_OFFSET_BITS=64")
add_definitions("-DFUSE_USE_VERSION=26")

FIND_PACKAGE( Boost 1.40 COMPONENTS system filesystem thread REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
link_directories(${Boost_LIBRARY_DIR})

aux_source_directory(. SRC_LIST)
aux_source_directory(easySQLite SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(fuse-dedup -lfuse -ldl -lpthread ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})
include_directories(fuse-dedup PRIVATE easySQLite)
