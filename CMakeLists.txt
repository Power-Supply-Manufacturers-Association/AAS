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
