cmake_minimum_required(VERSION 3.20)
project(ComprobantesElectronicosCore VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)

# Flags específicos para MSVC (Windows)
if(MSVC)
    add_compile_options(/bigobj /utf-8 /EHsc)
    # windows.h (que puede entrar por OpenSSL) define min y max como macros y rompe std::numeric_limits
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
    set_source_files_properties(
        src/common/DivisionTerritorial.cpp
        PROPERTIES
        COMPILE_FLAGS "/O1 /Gy /Zm500"
    )
endif()

# Archivos fuente del Core
set(CORE_SOURCES
    src/c_api/cec_c_api.cpp
    src/codec/JsonCodec.cpp
    src/crypto/Pkcs12Signer.cpp
    src/crypto/XmlCanonicalizer.cpp
    src/crypto/XadesSigner.cpp
    src/common/DivisionTerritorial.cpp
    src/common/Catalogos.cpp
    src/common/Monedas.cpp
    src/common/UnidadesMedida.cpp
    src/common/FormasFarmaceuticas.cpp
    libs/pugixml.cpp
)

# Librería compartida (Shared) o estática (Static)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

add_library(ComprobantesElectronicosCore ${CORE_SOURCES})
if(BUILD_SHARED_LIBS)
    target_compile_definitions(ComprobantesElectronicosCore PRIVATE cec_EXPORTS cec_BUILDING_DLL)
else()
    target_compile_definitions(ComprobantesElectronicosCore PUBLIC cec_STATIC)
endif()
target_link_libraries(ComprobantesElectronicosCore PUBLIC OpenSSL::Crypto OpenSSL::SSL)

# CMake Target Alias
add_library(cec::core ALIAS ComprobantesElectronicosCore)

target_include_directories(ComprobantesElectronicosCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>
    $<INSTALL_INTERFACE:include>
)

option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)
    enable_testing()
    find_package(Threads REQUIRED)

    # Submódulo en libs/googletest con fallback si no se inicializó
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/googletest/CMakeLists.txt")
        set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)   # mismo runtime de C que OpenSSL en MSVC
        add_subdirectory(libs/googletest EXCLUDE_FROM_ALL)
    else()
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.14.0
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "test/*.cpp")
    add_executable(CECTests ${TEST_SOURCES})
    target_link_libraries(CECTests PRIVATE cec::core GTest::gtest Threads::Threads)
    include(GoogleTest)
    gtest_discover_tests(CECTests WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
