cmake_minimum_required(VERSION 3.17.2)
project(BWXML)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
option(USE_TOBIASLOCKER_BASE64 "Use header-only Base64 implementation by Tobias Locker instead of Boost" OFF)

set(BUILD_SHARED_LIBS OFF)

if(NOT VCPKG_TOOLCHAIN)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif()

# Boost
find_package(Boost CONFIG COMPONENTS program_options serialization thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Library
set (BWXML_LIB_SOURCES "src/bwxml-lib/Base64.cpp"
                       "src/bwxml-lib/BWCommon.cpp"
                       "src/bwxml-lib/BWReader.cpp"
                       "src/bwxml-lib/BWWriter.cpp"
                       "src/bwxml-lib/DataStream.cpp")
add_library(libbwxml ${BWXML_LIB_SOURCES})

if(USE_TOBIASLOCKER_BASE64)
    set(TOBIASLOCKER_BASE64_INCLUDE "${TOBIASLOCKER_BASE64_DIR}/include")
    if (NOT EXISTS "${TOBIASLOCKER_BASE64_INCLUDE}/base64.hpp")
        message(ERROR "Tobias Locker's Base64 header not found")
    endif()

    target_include_directories(libbwxml PRIVATE ${TOBIASLOCKER_BASE64_INCLUDE})
    target_compile_definitions(libbwxml PRIVATE BWXML_USE_TOBIASLOCKER_BASE64)
else()
    target_compile_definitions(libbwxml PRIVATE BWXML_USE_BOOST_BASE64)
endif()

target_link_libraries(libbwxml ${Boost_LIBRARIES})

# CLI
set (BWXML_CLI_SOURCES "src/bwxml-cli/main.cpp")
add_executable(bwxml ${BWXML_CLI_SOURCES})
target_include_directories(bwxml PRIVATE "src/bwxml-lib")
target_link_libraries(bwxml libbwxml)

#Python
option(BUILD_PYTHON_MODULE "Build python module" ON)
if(BUILD_PYTHON_MODULE)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    
    add_library(bwxml-pyd SHARED "src/bwxml-python3/pythonModule.cpp")

    target_include_directories(bwxml-pyd PRIVATE ${Python3_INCLUDE_DIRS})
    target_include_directories(bwxml-pyd PRIVATE "src/bwxml-lib")

    target_link_libraries(bwxml-pyd libbwxml)
    target_link_libraries(bwxml-pyd ${Python3_LIBRARIES})

    set_target_properties(bwxml-pyd PROPERTIES OUTPUT_NAME "bwxml")
    set_target_properties(bwxml-pyd PROPERTIES SUFFIX ".pyd")
endif()

# Installation
#
install(
    TARGETS libbwxml bwxml 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib)

#
# CPack
#
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_PACKAGE_ARCHITECTURE amd64)
else()
  set(CPACK_PACKAGE_ARCHITECTURE i686)
endif()

set(CPACK_PACKAGE_NAME "bwxml")
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}) 
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A tool for packing and unpacking BigWorld compressed data sections from/to plain XML")
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCHITECTURE})
set(CPACK_PACKAGE_CONTACT "Mikhail Paulyshka <me@mixaill.tk>")

include(CPack)

