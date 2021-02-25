cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

####################
# TARGET PLATFORM
####################
option(PLATFORM_LINUX "Linux platform target" OFF)
option(PLATFORM_WINDOWS "Windows platform target (msys/mingw64)" OFF)
include(Toolchain.cmake)
####################
# TARGET PLATFORM
####################

project(gdiopt)

# sources
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*)
# includes
set(INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ldflags
set(LDFLAGS)

set(CMAKE_CXX_STANDARD 14)

#####################
# PLATORM SPECIFIC
#####################
if (PLATFORM_LINUX)
endif ()
if (PLATFORM_WINDOWS)
endif ()

#####################
# SCREENSCRAP LIBRARY
#####################
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_compile_options(${PROJECT_NAME} PUBLIC ${CFLAGS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LDFLAGS})
