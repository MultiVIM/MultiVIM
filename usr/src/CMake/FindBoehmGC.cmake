# - Try to find Boehm GC
#   Once done, this will define
#
#   BoehmGC_FOUND - system has Boehm GC
#   BoehmGC_INCLUDE_DIR - the Boehm GC include directories
#   BoehmGC_LIBRARIES - link these to use Boehm GC
# 
#   Copyright (c) 2010-2015  Takashi Kato <ktakashi@ymail.com>
# 
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
# 
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
# 
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
# 
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
#  $Id: $
# 

# CMake module to find Boehm GC

# use pkg-config if available
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_BDW_GC QUIET bdw-gc)

# try newer one first in case of gc.h is overwritten.
FIND_PATH(BoehmGC_INCLUDE_DIR gc/gc.h
  HINTS ${PC_BDW_GC_INCLUDEDIR} ${PC_BDW_GC_INCLUDE_DIRS})

IF (NOT BoehmGC_INCLUDE_DIR)
  FIND_PATH(BoehmGC_INCLUDE_DIR gc.h
    HINTS ${PC_BDW_GC_INCLUDEDIR} ${PC_BDW_GC_INCLUDE_DIRS})
  IF (BoehmGC_INCLUDE_DIR)
    SET(HAVE_GC_H TRUE)
  ENDIF()
ELSE()
  SET(HAVE_GC_GC_H TRUE)
ENDIF()

# For FreeBSD we need to use gc-threaded
IF (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
  # checks if 'gc' supports 'GC_get_parallel' and if it does
  # then use it
  INCLUDE(${CMAKE_ROOT}/Modules/CheckCSourceCompiles.cmake)
  # not sure if this links properly...
  FIND_LIBRARY(BoehmGC_LIBRARIES NAMES gc
    HINTS ${PC_BDW_GC_LIBDIR} ${PC_BDW_GC_LIBRARY_DIRS})
  MESSAGE(STATUS "GC library ${BoehmGC_LIBRARIES}")
  SET(CMAKE_REQUIRED_LIBRARIES "gc")
  SET(CMAKE_REQUIRED_DEFINITIONS "-DGC_THREADS")
  SET(CMAKE_REQUIRED_INCLUDES "${BoehmGC_INCLUDE_DIR}")
  SET(CMAKE_REQUIRED_FLAGS "-L${PC_BDW_GC_LIBRARY_DIRS}")
  MESSAGE(STATUS "Boehm GC include dir: ${CMAKE_REQUIRED_INCLUDES}")
  CHECK_C_SOURCE_RUNS(
    "#include <gc.h>
int main() {
int i= GC_get_parallel();
return 0;
}
" GC_GET_PARALLEL_WORKS)
  IF (NOT GC_GET_PARALLEL_WORKS)
    MESSAGE(STATUS "Try gc-threaded")

    # bdw-gc-threaded is the proper name for FreeBSD pkg-config
    PKG_CHECK_MODULES(PC_BDW_GC_THREADED bdw-gc-threaded)
    FIND_LIBRARY(BoehmGC_THREADED_LIBRARIES NAMES gc-threaded
      HINTS ${PC_BDW_GC_THREADED_LIBDIR} ${PC_BDW_GC_THREADED_THREADED_DIRS})

    MESSAGE(STATUS "GC threaded library ${BoehmGC_THREADED_LIBRARIES}")
    IF (BoehmGC_THREADED_LIBRARIES)
      # OK just use it
      SET(BoehmGC_LIBRARIES "${BoehmGC_THREADED_LIBRARIES}")
    ENDIF()
  ENDIF()
ELSE()
  FIND_LIBRARY(BoehmGC_LIBRARIES NAMES gc
    HINTS ${PC_BDW_GC_LIBDIR} ${PC_BDW_GC_LIBRARY_DIRS})
  # OpenSolaris uses bgc as Boehm GC runtime in its package manager.
  # so try it
  IF (NOT BoehmGC_LIBRARIES)
    FIND_LIBRARY(BoehmGC_LIBRARIES NAMES bgc
      HINTS ${PC_BDW_GC_LIBDIR} ${PC_BDW_GC_LIBRARY_DIRS})
  ENDIF()
ENDIF()

MESSAGE(STATUS "Found GC library: ${BoehmGC_LIBRARIES}")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BoehmGC DEFAULT_MSG
                                  BoehmGC_LIBRARIES BoehmGC_INCLUDE_DIR)

MARK_AS_ADVANCED(BoehmGC_LIBRARIES BoehmGC_INCLUDE_DIR)

if(BoehmGC_FOUND AND NOT TARGET BoehmGC::BoehmGC)
  add_library(BoehmGC::BoehmGC UNKNOWN IMPORTED)
  set_target_properties(BoehmGC::BoehmGC PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${BoehmGC_INCLUDE_DIRS}")
  set_property(TARGET BoehmGC::BoehmGC APPEND PROPERTY
      IMPORTED_LOCATION "${BoehmGC_LIBRARIES}")
endif()