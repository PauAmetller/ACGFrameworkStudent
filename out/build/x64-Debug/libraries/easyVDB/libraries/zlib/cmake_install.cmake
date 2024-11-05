# Install script for directory: C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/libraries/easyVDB/libraries/zlib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/lib/zlibd.lib")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/build/x64-Debug/libraries/easyVDB/libraries/zlib/zlibd.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/bin/zlibd.dll")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/bin" TYPE SHARED_LIBRARY FILES "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/build/x64-Debug/libraries/easyVDB/libraries/zlib/zlibd.dll")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/lib/zlibstaticd.lib")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/lib" TYPE STATIC_LIBRARY FILES "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/build/x64-Debug/libraries/easyVDB/libraries/zlib/zlibstaticd.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/include/zconf.h;C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/include/zlib.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/include" TYPE FILE FILES
    "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/build/x64-Debug/libraries/easyVDB/libraries/zlib/zconf.h"
    "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/libraries/easyVDB/libraries/zlib/zlib.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/share/man/man3/zlib.3")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/share/man/man3" TYPE FILE FILES "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/libraries/easyVDB/libraries/zlib/zlib.3")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/share/pkgconfig/zlib.pc")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/install/x64-Debug/share/pkgconfig" TYPE FILE FILES "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/build/x64-Debug/libraries/easyVDB/libraries/zlib/zlib.pc")
endif()
