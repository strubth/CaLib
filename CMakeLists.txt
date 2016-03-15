cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(CaLib)

# more warnings
add_definitions(-D_FORTIFY_SOURCE=2)

# set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# define variables
set(DOCS OFF CACHE BOOL "create CaLib HTML documentation")

# Mac OS X special settings
if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
endif(APPLE)

# use copied CMake modules from ROOT 6
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{ROOTSYS}/etc/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

# adjust some paths
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED MODULE COMPONENTS MathCore RIO Hist Gui Spectrum MySQL SQLite)

# check for MySQL and SQLite support
if(NOT ROOT_mysql_FOUND AND NOT ROOT_sqlite_FOUND)
    message(FATAL_ERROR "No MySQL and SQLite support in ROOT installation!")
endif()
if(ROOT_mysql_FOUND)
    message(STATUS "MySQL support in ROOT installation")
else()
    message(WARNING "No MySQL support in ROOT installation!")
endif()
if(ROOT_sqlite_FOUND)
    message(STATUS "SQLite support in ROOT installation")
else()
    message(WARNING "No SQLite support in ROOT installation!")
endif()

# find ncurses
SET(CURSES_USE_NCURSES TRUE)
find_package(Curses REQUIRED)

# define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

# source file globbing
file(GLOB SRCS src/TC*.cxx)

# header directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# create the dictionary
ROOT_GENERATE_DICTIONARY(G__CaLib include/TC*.h LINKDEF LinkDef.h)

# create the shared library
add_library(CaLib SHARED ${SRCS} G__CaLib.cxx)
target_link_libraries(CaLib ${ROOT_LIBRARIES})

# create executables
add_executable(calib_manager src/MainCaLibManager.cxx)
target_link_libraries(calib_manager CaLib ${CURSES_LIBRARIES})

# generate rootmap
add_custom_command(TARGET CaLib POST_BUILD 
    COMMAND rlibmap -o ${PROJECT_BINARY_DIR}/lib/libCaLib.rootmap
                    -l ${PROJECT_BINARY_DIR}/lib/libCaLib.so
                    -d ${ROOT_LIBRARIES}
                    -c ${CMAKE_SOURCE_DIR}/include/LinkDef.h
)

# generate HTML documentation
if (DOCS)
    add_custom_command(TARGET CaLib POST_BUILD
        COMMAND root -b ${CMAKE_SOURCE_DIR}/src/htmldoc.C'(\"${CMAKE_SOURCE_DIR}\") '
    )
endif()

