cmake_minimum_required(VERSION 3.15)
project(AAS LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# --- nlohmann/json (header-only). FetchContent_MakeAvailable is idempotent by name, so this is a no-op
#     when a parent project (e.g. Kirchhoff / RAS) has already declared the same `json`. ---
include(FetchContent)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG tags/v3.11.3)
FetchContent_MakeAvailable(json)

# --- Core library: aas_to_cias (the AAS family's CIAS lowering). ---
# AAS is otherwise a schema-only repo; this lightweight converter is the analog-family counterpart of
# RAS/CAS/SAS's converters. It reads the agnostic `behavioral` block (raw JSON — no quicktype-typed
# header is needed for the small ideal-control-block payload).
add_library(AAS STATIC src/AasConverter.cpp)
target_include_directories(AAS PUBLIC
    src
    "${CMAKE_CURRENT_SOURCE_DIR}/../PEAS/src")    # Fidelity.hpp
target_link_libraries(AAS PUBLIC nlohmann_json::nlohmann_json)

# --- Catch2 unit tests (house standard: run ./build/aas_tests directly, never ctest). ---
option(AAS_BUILD_TESTS "Build AAS unit tests" ON)
if (AAS_BUILD_TESTS AND NOT EMSCRIPTEN)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.8.1)
    FetchContent_MakeAvailable(Catch2)
    add_executable(aas_tests tests/test_aas.cpp)
    target_link_libraries(aas_tests PRIVATE AAS Catch2::Catch2WithMain)
endif()
