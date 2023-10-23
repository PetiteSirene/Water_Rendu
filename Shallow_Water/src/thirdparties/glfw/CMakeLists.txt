cmake_minimum_required(VERSION 3.4...3.20 FATAL_ERROR)

project(GLFW VERSION 3.4.0 LANGUAGES C)

if (POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

string(COMPARE EQUAL "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" GLFW_STANDALONE)


include(GNUInstallDirs)

if (GLFW_USE_OSMESA)
    message(FATAL_ERROR "GLFW_USE_OSMESA has been removed; set the GLFW_PLATFORM init hint")
endif()

if(WIN32)
    set(GLFW_BUILD_WIN32 ON)
    set(GLFW_BUILD_COCOA OFF)
    set(GLFW_BUILD_X11 OFF)
    set(GLFW_BUILD_WAYLAND OFF)
elseif(APPLE)
    set(GLFW_BUILD_WIN32 OFF)
    set(GLFW_BUILD_COCOA ON)
    set(GLFW_BUILD_X11 OFF)
    set(GLFW_BUILD_WAYLAND OFF)
else()
    set(GLFW_BUILD_WIN32 OFF)
    set(GLFW_BUILD_COCOA OFF)
    set(GLFW_BUILD_X11 ON)
    set(GLFW_BUILD_WAYLAND OFF)
endif()
#cmake_dependent_option(GLFW_BUILD_WAYLAND "Build support for Wayland"
#    "${GLFW_USE_WAYLAND}" "UNIX;NOT APPLE" OFF)


set(GLFW_USE_HYBRID_HPG OFF)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
set(GLFW_BUILD_SHARED_LIBRARY FALSE)


list(APPEND CMAKE_MODULE_PATH "${GLFW_SOURCE_DIR}/CMake/modules")

find_package(Threads REQUIRED)


#--------------------------------------------------------------------
# Report backend selection
#--------------------------------------------------------------------
if (GLFW_BUILD_WIN32)
    message(STATUS "Including Win32 support")
endif()
if (GLFW_BUILD_COCOA)
    message(STATUS "Including Cocoa support")
endif()
if (GLFW_BUILD_WAYLAND)
    message(STATUS "Including Wayland support")
endif()
if (GLFW_BUILD_X11)
    message(STATUS "Including X11 support")
endif()

#--------------------------------------------------------------------
# Apply Microsoft C runtime library option
# This is here because it also applies to tests and examples
#--------------------------------------------------------------------
if (MSVC AND NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
    if (CMAKE_VERSION VERSION_LESS 3.15)
        foreach (flag CMAKE_C_FLAGS
                      CMAKE_C_FLAGS_DEBUG
                      CMAKE_C_FLAGS_RELEASE
                      CMAKE_C_FLAGS_MINSIZEREL
                      CMAKE_C_FLAGS_RELWITHDEBINFO)

            if (flag MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
            endif()
            if (flag MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
            endif()

        endforeach()
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

#--------------------------------------------------------------------
# Create generated files
#--------------------------------------------------------------------
include(CMakePackageConfigHelpers)

set(GLFW_CONFIG_PATH "${CMAKE_INSTALL_LIBDIR}/cmake/glfw3")

configure_package_config_file(CMake/glfw3Config.cmake.in
                              src/glfw3Config.cmake
                              INSTALL_DESTINATION "${GLFW_CONFIG_PATH}"
                              NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(src/glfw3ConfigVersion.cmake
                                 VERSION ${GLFW_VERSION}
                                 COMPATIBILITY SameMajorVersion)


add_subdirectory(src)



