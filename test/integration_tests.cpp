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

  SECTION("DELETE request")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"delete"}}});
    client.del(httpUrl).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [](const SimpleHttp::HttpSuccess &success){
          CHECK(success.status() == SimpleHttp::OK);
        }
    );
  }

  SECTION("HEAD request")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get"}}});
    client.head(httpUrl).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [](const SimpleHttp::HttpSuccess &success){
          CHECK(success.status() == SimpleHttp::OK);
        }
    );
  }

  SECTION("OPTIONS request")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get"}}});
    client.options(httpUrl).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [](const SimpleHttp::HttpSuccess &success){
          CHECK_THAT(SimpleHttp::vec(success.headers().value().at("Allow"), ','),
                     Catch::UnorderedEquals(std::vector<std::string>{"HEAD", "OPTIONS", "GET"}));
        }
    );
  }

  SECTION("TRACE request")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"trace"}}});
    client.trace(httpUrl).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [](const SimpleHttp::HttpSuccess &success){
          CHECK(success.headers().value().at("Content-Type") == "message/http");
        }
    );
  }

  SECTION("Connection error")
  {
    std::string error;
    client = client.with_error_callback([&error](auto &err) { error = "Error: " + err; });
    SimpleHttp::HttpUrl httpUrl = url.with_protocol(SimpleHttp::Protcol{"zxcv"});
    client.get(httpUrl).template match<void>(
        [&error](const SimpleHttp::HttpFailure &failure){
          CHECK(error == "Error: Unsupported protocol");
        },
        [](const SimpleHttp::HttpSuccess &success){
          FAIL(success.status().to_string() + " : " + success.body().value());
        }
    );
  }

  SECTION("POST request that expects a 204 NO_CONTENT response")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"empty_post_response"}}});
    SimpleHttp::HttpRequestBody body = SimpleHttp::HttpRequestBody{""};
    SimpleHttp::Predicate<SimpleHttp::HttpStatusCode> successPredicate = SimpleHttp::eq(SimpleHttp::NO_CONTENT);
    SimpleHttp::Headers headers{{{"Content-Type", "application/json"}}};

    client.post(httpUrl, body, successPredicate, headers).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [](const SimpleHttp::HttpSuccess &success){
          CHECK(success.status() == SimpleHttp::NO_CONTENT);
          CHECK(success.body().value().empty());
        }
    );
  }

  SECTION("GET request that expects a 405 METHOD_NOT_ALLOWED response")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"empty_post_response"}}});
    SimpleHttp::Predicate<SimpleHttp::HttpStatusCode> successPredicate = SimpleHttp::eq(SimpleHttp::METHOD_NOT_ALLOWED);

    client.get(httpUrl, successPredicate).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [](const SimpleHttp::HttpSuccess &success){
          CHECK(success.status() == SimpleHttp::METHOD_NOT_ALLOWED);
        }
    );
  }

  SECTION("Ranged response success predicate")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get"}}});
    SimpleHttp::Predicate<SimpleHttp::HttpStatusCode> successPredicate = SimpleHttp::successful();
    SimpleHttp::HttpResponseBody expected{ SimpleHttp::HttpResponseBody{R"({"get": "ok"})"} };

    client.get(httpUrl, successPredicate).template match<void>(
        [](const SimpleHttp::HttpFailure &failure){
          FAIL(failure.status().to_string() + " : " + failure.body().value());
        },
        [&expected](const SimpleHttp::HttpSuccess &success){
          CHECK(success.body() == expected);
        }
    );
  }

  SECTION("Wrap Response")
  {
    SimpleHttp::HttpUrl httpUrl = url.with_path_segments(SimpleHttp::PathSegments{{SimpleHttp::PathSegment{"get"}}});
    SimpleHttp::Predicate<SimpleHttp::HttpStatusCode> successPredicate = SimpleHttp::successful();

    auto wrapped = client.get(httpUrl, successPredicate).template match<std::string>(
        [](const SimpleHttp::HttpFailure &failure) {
          return failure.body().value();
        },
        [](const SimpleHttp::HttpSuccess &success) {
          auto keys = nlohmann::json::parse(success.body().value());
          return keys["get"];
        }
    );

    CHECK(wrapped == "ok");
  }
}