CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(BST_INSTALL_CATEGORY Libraries) 

PROJECT(Rcs_2.0)

IF (NOT CMAKE_BUILD_TYPE)
  IF (WIN32)
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING
       "Choose the type of build, options are: Debug Release." FORCE)
  ELSE()
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING 
       "Choose the type of build, options are: Debug Release." FORCE)
  ENDIF()
ENDIF (NOT CMAKE_BUILD_TYPE)

IF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  SET(RCS_DEBUG_SUFFIX d)
ENDIF()

SET(RCS_THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/external)
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

IF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  MESSAGE("-- Compiler is: Clang")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  MESSAGE("-- Compiler is: GNU")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  MESSAGE("-- Compiler is: Intel")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  MESSAGE("-- Compiler is: MSVC")
ENDIF()

################################################################################
#
# Copy config directories:
#   - Symlink for Linux
#   - Deep copy of the config directory for windows
#
# This needs to go before including the sub directories, since they 
# partially create links /copies inside the config directory
#
################################################################################
IF(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/config")

  IF(UNIX)
    SET(CONFIGDIR_COPY_COMMAND create_symlink)
    MESSAGE("-- Linking config directory")
  ELSE()
    SET(CONFIGDIR_COPY_COMMAND copy_directory)
    MESSAGE("-- Copying config directory")
  ENDIF()

  EXECUTE_PROCESS (COMMAND
   ${CMAKE_COMMAND} -E ${CONFIGDIR_COPY_COMMAND}
   ${PROJECT_SOURCE_DIR}/config
   ${CMAKE_CURRENT_BINARY_DIR}/config)

ELSE()

  MESSAGE("-- Config directory already exists - skipping")

ENDIF()

################################################################################
#
# Create shared libraries for Linux, and static ones for Windows
# For CMake 3.4 and higher, there is an easy way to create windows dlls. For
# details, see http://www.kitware.com/blog/home/post/939
#
################################################################################
IF(UNIX)
  MESSAGE("-- Creating dynamically linked libraries (.so) under Unix")
  SET(LIBRARY_LINKING_TYPE "SHARED")
ENDIF()

IF(WIN32)
  MESSAGE("We are on a windows machine")
  IF(CMAKE_MAJOR_VERSION STREQUAL "3" AND CMAKE_MINOR_VERSION STREQUAL "4")
    MESSAGE("Found CMake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}: Creating windows dlls")
    SET(LIBRARY_LINKING_TYPE "SHARED")
    SET(BUILD_SHARED_LIBS ON)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    SET(CREATE_WIN_DLL TRUE)
    ADD_DEFINITIONS(-DWIN_DLL)
  ELSE()
    MESSAGE(FATAL_ERROR "Found CMake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}, but at least 3.4 is needed")
  ENDIF()
ENDIF(WIN32)

SITE_NAME(HOSTNAME)

################################################################################
#
# Default options
#
################################################################################
OPTION(HEADLESS_BUILD "Build without graphics and Guis" OFF)
OPTION(USE_WM5 "Use Wildmagic library" OFF)
# OPTION(USE_OCTOMAP "Use Octomap library" OFF)
OPTION(USE_EIGEN3 "Use Eigen3 library" OFF)
OPTION(ENABLE_DEBUG_MACROS "Enable debug macros also in release mode" ON)

IF(UNIX)
  OPTION(ENABLE_C++11 "Compile with C++11 support enabled" OFF)
ENDIF()

SET(USE_VORTEX OFF CACHE STRING "Vortex version to use")
SET_PROPERTY(CACHE USE_VORTEX PROPERTY STRINGS OFF ESSENTIALS)

SET(USE_BULLET OFF CACHE STRING "Bullet version to use")
SET_PROPERTY(CACHE USE_BULLET PROPERTY STRINGS OFF 2.83_double 2.83_float)

################################################################################
#
# Dependency settings and compile settings. These must go after Externals.cmake
#
################################################################################
INCLUDE(Externals)
INCLUDE(CompileSettings)

SET(ARCH ${CMAKE_SYSTEM_NAME})

IF(${CMAKE_BUILD_TYPE} STREQUAL "Release" AND NOT ENABLE_DEBUG_MACROS)
  ADD_DEFINITIONS(-DRCS_NO_DEBUG)
  MESSAGE("Compiling in RELEASE mode")
ENDIF()

# Add the automatically determined parts of the RPATH which point to directories 
# outside the build tree to the install RPATH. Required for Vortex and other stuff
# from the SIT.
# Note: This only works with CMake >= 3.0. With 2.8, the library directories of the dependencies
# must be added to the LD_LIBRARY_PATH manually in oredr to use the installed executables.
# This has no effect on using the package in another CMake project.
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# The RPATH to be used when installing, but only if it's not a system directory
# Source: CMake wiki (https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling)
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")


################################################################################
#
# Flags to strip down the file names for logging at compile time. Adapted from
# http://stackoverflow.com/questions/8487986/file-macro-shows-full-path
#
################################################################################
IF(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
ENDIF()

################################################################################
#
# If a user-file is in the cmake directory, it will be called
#
################################################################################
SET(USER_LIST "${PROJECT_SOURCE_DIR}/cmake/CMakeLists-$ENV{USER}.txt")

IF(EXISTS ${USER_LIST})
  MESSAGE("-- Including " ${USER_LIST})
  INCLUDE(${USER_LIST})
ELSE()
  MESSAGE("-- Not found: " ${USER_LIST})
ENDIF()


################################################################################
#
# Setup exported target collection
#
################################################################################
# Setup RCS_EXPORT_LIBRARIES so the component CMakeLists.txt can modify it
SET(RCS_EXPORT_LIBRARIES "" CACHE INTERNAL "")
# Create a macro for adding an export library
FUNCTION(RCS_ADD_EXPORT_LIBRARY library)
  SET(RCS_EXPORT_LIBRARIES ${RCS_EXPORT_LIBRARIES} ${library} CACHE INTERNAL "")
ENDFUNCTION()

################################################################################
#
# Add source code directories
#
################################################################################

ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (bin)
ADD_SUBDIRECTORY (examples)

IF(EXISTS "${PROJECT_SOURCE_DIR}/projects")
  MESSAGE("-- Including projects directory" )
  ADD_SUBDIRECTORY (projects)
ENDIF()

ADD_SUBDIRECTORY (doc)

################################################################################
#
# Export script
#
################################################################################
INCLUDE(Export)
