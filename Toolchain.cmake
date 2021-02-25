# setup toolchains

if (PLATFORM_LINUX)
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__LINUX__" CACHE STRING "C flags" FORCE)
    set(TARGET_PLATFORM linux CACHE STRING "")
elseif (PLATFORM_WINDOWS)
    set(CMAKE_SYSTEM_NAME "Windows")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__WINDOWS__" CACHE STRING "C flags" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__WINDOWS__" CACHE STRING "CXX flags" FORCE)
    #set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static"  CACHE STRING "" FORCE)
endif ()

message(STATUS "target platform: ${TARGET_PLATFORM}, cmake version: ${CMAKE_VERSION}")
