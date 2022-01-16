#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "json.hpp"
#include "../simple_http.hpp"

TEST_CASE("Integration Tests")
{
  SimpleHttp::Client client;
  SimpleHttp::HttpUrl url = SimpleHttp::HttpUrl()
      .with_protocol(SimpleHttp::Protcol{"http"})
      .with_host(SimpleHttp::Host{"localhost:5000"});

  SECTION("GET Request")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get"}}});

    SimpleHttp::HttpSuccess expected{
      SimpleHttp::HttpResponse{
          SimpleHttp::OK,
          SimpleHttp::HttpResponseHeaders{SimpleHttp::Headers{}},
          SimpleHttp::HttpResponseBody{R"({"get": "ok"})"}
      }
    };

    client.get(httpUrl).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [&expected](const SimpleHttp::HttpSuccess &success){
          CHECK(success.body() == expected.body());
        }
    );
  }
  
  SECTION("GET request with single query parameter")
  {
    SimpleHttp::HttpUrl &httpUrl = url
        .with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get_hello"}}})
        .with_query_parameters(SimpleHttp::QueryParameters{
          {{SimpleHttp::QueryParameterKey{"name"}, SimpleHttp::QueryParameterValue{"test"}}}
        });

    SimpleHttp::HttpSuccess expected{
        SimpleHttp::HttpResponse{
            SimpleHttp::OK,
            SimpleHttp::HttpResponseHeaders{SimpleHttp::Headers{}},
            SimpleHttp::HttpResponseBody{R"({"hello": "test"})"}
        }
    };

    client.get(httpUrl).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [&expected](const SimpleHttp::HttpSuccess &success){
          CHECK(success.body() == expected.body());
        }
    );
  }

  SECTION("GET request with multiple query parameters")
  {
    SimpleHttp::HttpUrl httpUrl = url
        .with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get_full"}}})
        .with_query_parameters(SimpleHttp::QueryParameters{{
          {SimpleHttp::QueryParameterKey{"first"}, SimpleHttp::QueryParameterValue{"simple"}},
          {SimpleHttp::QueryParameterKey{"last"}, SimpleHttp::QueryParameterValue{"http"}}
        }});

    SimpleHttp::HttpSuccess expected{
        SimpleHttp::HttpResponse{
            SimpleHttp::OK,
            SimpleHttp::HttpResponseHeaders{SimpleHttp::Headers{}},
            SimpleHttp::HttpResponseBody{R"({"first": "simple", "last": "http"})"}
        }
    };

    client.get(httpUrl).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [&expected](const SimpleHttp::HttpSuccess &success){
          CHECK(success.body() == expected.body());
        }
    );
  }

  SECTION("POST request")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"post"}}});
    SimpleHttp::HttpRequestBody body = SimpleHttp::HttpRequestBody{R"({"name":"test"})"};
    SimpleHttp::Headers headers{{{"Content-Type", "application/json"}}};

    SimpleHttp::HttpSuccess expected{
        SimpleHttp::HttpResponse{
            SimpleHttp::OK,
            SimpleHttp::HttpResponseHeaders{SimpleHttp::Headers{}},
            SimpleHttp::HttpResponseBody{R"({"hello": "test"})"}
        }
    };

    client.post(httpUrl, body, headers).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [&expected](const SimpleHttp::HttpSuccess &success){
          CHECK(success.body() == expected.body());
        }
    );
  }

  SECTION("PUT request")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"put"}}});
    SimpleHttp::HttpRequestBody body = SimpleHttp::HttpRequestBody{R"({"update":"test"})"};
    SimpleHttp::Headers headers{{{"Content-Type", "application/json"}}};

    SimpleHttp::HttpSuccess expected{
        SimpleHttp::HttpResponse{
            SimpleHttp::OK,
            SimpleHttp::HttpResponseHeaders{SimpleHttp::Headers{}},
            SimpleHttp::HttpResponseBody{R"({"update": "test"})"}
        }
    };

    client.put(httpUrl, body, headers).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [&expected](const SimpleHttp::HttpSuccess &success){
          CHECK(success.body() == expected.body());
        }
    );
  }

//  SECTION("DELETE request")
//  {
//    auto maybe_response = client.del(url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"delete"}}}));
//
//    REQUIRE(maybe_response);
//    CHECK(maybe_response.value().status == SimpleHttp::OK);
//  }

//  SECTION("HEAD request")
//  {
//    auto maybe_response = client.head(url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get"}}}));
//
//    REQUIRE(maybe_response);
//    CHECK(maybe_response.value().status == SimpleHttp::OK);
//  }

//  SECTION("OPTIONS request")
//  {
//    auto maybe_response = client.options(url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get"}}}));
//
//    REQUIRE(maybe_response);
//
//    SimpleHttp::Headers headers = maybe_response.value().headers.value();
//    CHECK_THAT(SimpleHttp::vec(headers.at("Allow"), ','),
//               Catch::UnorderedEquals(std::vector<std::string>{"HEAD", "OPTIONS", "GET"}));
//  }

//  SECTION("TRACE request")
//  {
//    auto maybe_response = client.trace(url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"trace"}}}));
//
//    REQUIRE(maybe_response);
//    CHECK(maybe_response.value().headers.value().at("Content-Type") == "message/http");
//  }

//  SECTION("Connection error")
//  {
//    std::string error;
//    client = client.with_error_callback([&error](auto &err) { error = "Error: " + err; });
//    auto maybe_response = client.get(url.with_protocol(SimpleHttp::Protcol{"zxcv"}));
//
//    REQUIRE(!maybe_response);
//    CHECK(error == "Error: Unsupported protocol");
//  }

//  SECTION("POST request that expects a 204 NO_CONTENT response")
//  {
//    auto maybe_response = client.post(url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"empty_post_response"}}}),
//                                      SimpleHttp::HttpRequestBody{""},
//                                      SimpleHttp::eq(SimpleHttp::NO_CONTENT),
//                                      {{"Content-Type", "application/json"}});
//
//    REQUIRE(maybe_response);
//
//    SimpleHttp::HttpResponse response = maybe_response.value();
//
//    CHECK(response.status == SimpleHttp::NO_CONTENT);
//    CHECK(response.body.value().empty());
//  }

//  SECTION("GET request that expects a 405 METHOD_NOT_ALLOWED response")
//  {
//    auto maybe_response = client.get(url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"empty_post_response"}}}),
//                                     SimpleHttp::eq(SimpleHttp::METHOD_NOT_ALLOWED));
//
//    REQUIRE(maybe_response);
//    CHECK(maybe_response.value().status == SimpleHttp::METHOD_NOT_ALLOWED);
//  }

//  SECTION("Ranged response success predicate")
//  {
//    auto maybe_response = client.get(url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get"}}}),
//                                     SimpleHttp::successful());
//
//    REQUIRE(maybe_response);
//    SimpleHttp::HttpResponse response = maybe_response.value();
//    auto keys = nlohmann::json::parse(response.body.value());
//
//    CHECK(keys["get"] == "ok");
//  }

//  SECTION("Wrap Response")
//  {
//    auto maybe_response = client.get(url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get"}}}),
//                                     SimpleHttp::successful());
//
//    REQUIRE(maybe_response);
//
//    std::function<std::string(const std::optional<SimpleHttp::HttpResponse> &response)> capture_get = [&maybe_response](const auto &response) {
//      auto keys = nlohmann::json::parse(maybe_response.value().body.value());
//      return keys["get"];
//    };
//
//    auto wrapped = SimpleHttp::wrap_response<std::string>(maybe_response, capture_get);
//
//    CHECK(wrapped == "ok");
//  }
}