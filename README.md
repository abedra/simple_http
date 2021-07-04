# Simple Http

Simple Http is a header only C++ wrapper for libcurl. The aim is to have a small wrapper that introduces type safety and ergonomics while maintaining a small footprint.

## Quick Start

The following example demonstrates an `HTTP GET` request with JSON parsing provided by `nlohmann::json`. All calls will return `std::optional<SimpleHttp::HttpResponse>`

```c++
#include <iostream>
#include "nlohmann/json.hpp"
#include "simple_http.hpp"

int main() {
  SimpleHttp::Client client;
  SimpleHttp::HttpUrl url{"https://jsonplaceholder.typicode.com/users"};
  const std::optional<SimpleHttp::HttpResponse> &maybeResponse = client.get(url{"https://jsonplaceholder.typicode.com/users"});

  if (maybeResponse) {
    auto parsed = nlohmann::json::parse(maybeResponse.value().body.value());
    std::cout << parsed[0]["id"] << std::endl;
  } else {
    std::cout << "Failed GET for: " << url.value() << std::endl;
  }
}
```

## Installation

Simple Http is a header only library. You can simply download and place in your project, or embed it with your favorite package manager. Using Simple Http will introduce a dependency on cURL, so you will need to ensure you link to curl when compiling your program.

## Embedding With CMake

The following example demonstrates how to embed Simple Http into your project using CMake's `FetchContent` (CMake 3.11 or later).

```cmake
include(FetchContent)

FetchContent_Declare(simple_http
    GIT_REPOSITORY https://github.com/abedra/simple_http
    GIT_TAG master # it's better to pick specific sha or tag here
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "")

FetchContent_GetProperties(simple_http)

if(NOT simple_http_POPULATED)
  FetchContent_Populate(simple_http)
endif()

add_library(simple_http INTERFACE)
target_include_directories(simple_http INTERFACE ${simple_http_SOURCE_DIR})

# You may have a different way of resolving cURL. This is only here to provide a complete example
find_package(CURL)
if (CURL_FOUND)
  include_directories(${CURL_INCLUDE_DIR})
else (CURL_FOUND)
  message(FATAL_ERROR "CURL not found")
endif (CURL_FOUND)

target_link_libraries(${PROJECT_NAME} PRIVATE simple_http curl)
```

## Advanced Usage

The [integration tests](test/integration_tests.cpp) are a good source of examples for the features provided by Simple Http. It is recommended to read through the tests to get a better sense for how to consume this library.
