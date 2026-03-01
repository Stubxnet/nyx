cmake_minimum_required(VERSION 3.10)
project(nyx VERSION 1.0)

# cpp norms

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find raylib, nlohmann-json and tinyxml2
find_package(fmt REQUIRED)
find_package(raylib REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED) # actually unused, will be used to read worlds.json file
#find_package(tinyxml2 REQUIRED) # unused too
#find_package(spdlog REQUIRED) # actually not working on my computer. perhaps it wills works on another distribution ?
add_compile_definitions(SPDLOG_FMT_EXTERNAL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(nyx src/main.cpp)

# link to librairies
target_link_libraries(nyx raylib tinyxml2  nlohmann_json)
