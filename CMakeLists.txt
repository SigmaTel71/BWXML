cmake_minimum_required(VERSION 3.19)
project(BWXML)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(BUILD_SHARED_LIBS OFF)

find_package(pugixml CONFIG REQUIRED)

set(TOBIASLOCKER_BASE64_INCLUDE "${TOBIASLOCKER_BASE64_DIR}/include")
if (NOT EXISTS "${TOBIASLOCKER_BASE64_INCLUDE}/base64.hpp")
    message(ERROR "Tobias Locker's Base64 header not found")
endif()

# Library
set (BWXML_LIB_SOURCES "src/bwxml-lib/Base64.cpp"
                       "src/bwxml-lib/BWCommon.cpp"
                       "src/bwxml-lib/BWReader.cpp"
                       "src/bwxml-lib/BWWriter.cpp"
                       "src/bwxml-lib/DataStream.cpp")
add_library(libbwxml ${BWXML_LIB_SOURCES})
target_include_directories(libbwxml PRIVATE ${TOBIASLOCKER_BASE64_INCLUDE})
target_link_libraries(libbwxml pugixml::pugixml)

# CLI
option(BUILD_BWXML_TOOL "Build CLI BWXML tool" OFF)
if (BUILD_BWXML_TOOL)
    if (NOT VCPKG_TOOLCHAIN)
      set(Boost_USE_STATIC_LIBS ON)
      set(Boost_USE_STATIC_RUNTIME ON)
    endif()

    find_package(Boost CONFIG COMPONENTS program_options thread REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(bwxml "src/bwxml-cli/main.cpp")
    target_include_directories(bwxml PRIVATE "src/bwxml-lib")
    target_link_libraries(bwxml ${Boost_LIBRARIES} libbwxml)

    install(TARGETS bwxml RUNTIME DESTINATION bin)
endif()

#Python
option(BUILD_PYTHON_MODULE "Build python module" ON)
if(BUILD_PYTHON_MODULE)
    find_package(Python 2.6.4 COMPONENTS Interpreter Development REQUIRED)
    
    add_library(bwxml-pyd SHARED "src/bwxml-python/pythonModule.cpp")

    target_include_directories(bwxml-pyd PRIVATE ${Python_INCLUDE_DIRS} "src/bwxml-lib")
    target_link_libraries(bwxml-pyd libbwxml ${Python_LIBRARIES})

    set_target_properties(bwxml-pyd PROPERTIES OUTPUT_NAME "bwxml" SUFFIX ".pyd")
endif()

# Installation
install(TARGETS libbwxml ARCHIVE DESTINATION lib)

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
set(CPACK_PACKAGE_CONTACT "Vitaly Orekhov <vkvo2000@vivaldi.net>")

include(CPack)

