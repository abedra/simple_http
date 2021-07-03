cmake_minimum_required(VERSION 3.19)
project(jsonplaceholder)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG v3.9.1)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(simple_http
    GIT_REPOSITORY https://github.com/abedra/simple_http
    GIT_TAG master
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_GetProperties(simple_http)
if(NOT simple_http_POPULATED)
  FetchContent_Populate(simple_http)
endif()

add_library(simple_http INTERFACE)
target_include_directories(simple_http INTERFACE ${simple_http_SOURCE_DIR})

find_package(CURL)
if (CURL_FOUND)
  include_directories(${CURL_INCLUDE_DIR})
else (CURL_FOUND)
  message(FATAL_ERROR "CURL not found")
endif (CURL_FOUND)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json simple_http curl)
