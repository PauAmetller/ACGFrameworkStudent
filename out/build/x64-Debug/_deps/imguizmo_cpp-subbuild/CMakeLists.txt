# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.29.5-msvc4)

# Reject any attempt to use a toolchain file. We must not use one because
# we could be downloading it here. If the CMAKE_TOOLCHAIN_FILE environment
# variable is set, the cache variable will have been initialized from it.
unset(CMAKE_TOOLCHAIN_FILE CACHE)
unset(ENV{CMAKE_TOOLCHAIN_FILE})

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(imguizmo_cpp-populate NONE)



include(ExternalProject)
ExternalProject_Add(imguizmo_cpp-populate
                     "UPDATE_DISCONNECTED" "False" "EXTERNALPROJECT_INTERNAL_ARGUMENT_SEPARATOR" "URL" "https://raw.githubusercontent.com/CedricGuillemet/ImGuizmo/master/ImGuizmo.cpp" "DOWNLOAD_DIR" "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/libraries/imguizmo" DOWNLOAD_NO_EXTRACT YES
                    SOURCE_DIR          "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/build/x64-Debug/_deps/imguizmo_cpp-src"
                    BINARY_DIR          "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/build/x64-Debug/_deps/imguizmo_cpp-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


ExternalProject_Get_Property(imguizmo_cpp-populate DOWNLOADED_FILE)
get_filename_component(dlFileName "${DOWNLOADED_FILE}" NAME)

ExternalProject_Add_Step(imguizmo_cpp-populate copyfile
  COMMAND    "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe" -E copy_if_different
             "<DOWNLOADED_FILE>" "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/build/x64-Debug/_deps/imguizmo_cpp-src"
  DEPENDEES  patch
  DEPENDERS  configure
  BYPRODUCTS "C:/Users/pauam/Documents/GitHub/Advanced Computer Graphics/Volumetric_Framework/ACGFrameworkStudent/out/build/x64-Debug/_deps/imguizmo_cpp-src/${dlFileName}"
  COMMENT    "Copying file to SOURCE_DIR"
)

