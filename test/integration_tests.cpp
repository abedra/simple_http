#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "json.hpp"
#include "../simple_http.hpp"

using namespace SimpleHttp;

// TODO: Replace these with custom Catch2 matcher
static void CHECK_SUCCESS_STATUS(const HttpResult &result, const HttpStatusCode &statusCode) {
  result.template match<void>(
      [](const HttpFailure &failure){
        failure.template match<void>(
            [](const HttpConnectionFailure &c) {
              FAIL(c.value());
            },
            [](const HttpResponse &s){
              FAIL(s);
            }
        );
      },
      [&statusCode](const HttpSuccess &success){
        CHECK(success.status() == statusCode);
      }
  );
}

static void CHECK_SUCCESS_BODY(const HttpResult &result, const HttpResponseBody &expected) {
  result.template match<void>(
      [](const HttpFailure &failure){
        failure.template match<void>(
            [](const HttpConnectionFailure &c) {
              FAIL(c.value());
            },
            [](const HttpResponse &s){
              FAIL(s);
            }
        );
      },
      [&expected](const HttpSuccess &success){
        CHECK(expected == success.body());
      }
  );
}

static void CHECK_SUCCESS_HEADERS(const HttpResult &result,
                                  const std::string &key,
                                  const std::vector<std::string> &expected) {
  result.template match<void>(
      [](const HttpFailure &failure){
        failure.template match<void>(
            [](const HttpConnectionFailure &c) {
              FAIL(c.value());
            },
            [](const HttpResponse &s){
              FAIL(s);
            }
        );
      },
      [&key, &expected](const HttpSuccess &success){
        CHECK_THAT(vec(success.headers().value().at(key), ','),
                   Catch::UnorderedEquals(expected));
      }
  );
}

static void CHECK_SUCCESS_HEADERS(const HttpResult &result,
                                  const std::string &key,
                                  const std::string &expected) {
  result.template match<void>(
      [](const HttpFailure &failure){
        failure.template match<void>(
            [](const HttpConnectionFailure &c) {
              FAIL(c.value());
            },
            [](const HttpResponse &s){
              FAIL(s);
            }
        );
      },
      [&key, &expected](const HttpSuccess &success){
        CHECK(success.headers().value().at(key) == expected);
      }
  );
}

static void CHECK_CONNECTION_FAILURE(const HttpResult &result, const HttpConnectionFailure &expected) {
  result.template match<void>(
      [&expected](const HttpFailure &failure){
        failure.template match<void>(
            [&expected](const HttpConnectionFailure &c) {
              CHECK(expected == c);
            },
            [](const HttpResponse &s){
              FAIL(s);
            }
        );
      },
      [](const HttpSuccess &success){
        FAIL(success.status().to_string() + " : " + success.body().value());
      }
  );
}


TEST_CASE("Integration Tests")
{
  Client client;
  HttpUrl url = HttpUrl()
      .with_protocol(Protcol{"http"})
      .with_host(Host{"localhost:5000"});

  SECTION("GET Request")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"get"}}});

    CHECK_SUCCESS_BODY(client.get(httpUrl), HttpResponseBody{R"({"get": "ok"})"});
  }
  
  SECTION("GET request with single query parameter")
  {
    HttpUrl &httpUrl = url
        .with_path_segments(PathSegments{{PathSegment{"get_hello"}}})
        .with_query_parameters(QueryParameters{
          {{QueryParameterKey{"name"}, QueryParameterValue{"test"}}}
        });

    CHECK_SUCCESS_BODY(client.get(httpUrl), HttpResponseBody{R"({"hello": "test"})"});
  }

  SECTION("GET request with multiple query parameters")
  {
    HttpUrl httpUrl = url
        .with_path_segments(PathSegments{{PathSegment{"get_full"}}})
        .with_query_parameters(QueryParameters{{
          {QueryParameterKey{"first"}, QueryParameterValue{"simple"}},
          {QueryParameterKey{"last"}, QueryParameterValue{"http"}}
        }});

    CHECK_SUCCESS_BODY(client.get(httpUrl), HttpResponseBody{R"({"first": "simple", "last": "http"})"});
  }

  SECTION("POST request")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"post"}}});
    HttpRequestBody body = HttpRequestBody{R"({"name":"test"})"};
    Headers headers{{{"Content-Type", "application/json"}}};

    CHECK_SUCCESS_BODY(client.post(httpUrl, body, headers), HttpResponseBody{R"({"hello": "test"})"});
  }

  SECTION("PUT request")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"put"}}});
    HttpRequestBody body = HttpRequestBody{R"({"update":"test"})"};
    Headers headers{{{"Content-Type", "application/json"}}};

    CHECK_SUCCESS_BODY(client.put(httpUrl, body, headers), HttpResponseBody{R"({"update": "test"})"});
  }

  SECTION("DELETE request")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"delete"}}});

    CHECK_SUCCESS_STATUS(client.del(httpUrl), OK);
  }

  SECTION("HEAD request")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"get"}}});

    CHECK_SUCCESS_STATUS(client.head(httpUrl), OK);
  }

  SECTION("OPTIONS request")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"get"}}});

    CHECK_SUCCESS_HEADERS(client.options(httpUrl), "Allow", {"HEAD", "OPTIONS", "GET"});
  }

  SECTION("TRACE request")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"trace"}}});

    CHECK_SUCCESS_HEADERS(client.trace(httpUrl), "Content-Type", "message/http");
  }

  SECTION("Connection error")
  {
    HttpUrl httpUrl = url.with_protocol(Protcol{"zxcv"});

    CHECK_CONNECTION_FAILURE(client.get(httpUrl), HttpConnectionFailure{"Unsupported protocol"});
  }

  SECTION("POST request that expects a 204 NO_CONTENT response")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"empty_post_response"}}});
    HttpRequestBody body = HttpRequestBody{""};
    Predicate<HttpStatusCode> successPredicate = eq(NO_CONTENT);
    Headers headers{{{"Content-Type", "application/json"}}};
    HttpResult result = client.post(httpUrl, body, successPredicate, headers);

    CHECK_SUCCESS_STATUS(result, NO_CONTENT);
    CHECK_SUCCESS_BODY(result, HttpResponseBody{});
  }

  SECTION("GET request that expects a 405 METHOD_NOT_ALLOWED response")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"empty_post_response"}}});
    Predicate<HttpStatusCode> successPredicate = eq(METHOD_NOT_ALLOWED);

    CHECK_SUCCESS_STATUS(client.get(httpUrl, successPredicate), METHOD_NOT_ALLOWED);
  }

  SECTION("Ranged response success predicate")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"get"}}});
    Predicate<HttpStatusCode> successPredicate = successful();
    HttpResponseBody expected{ HttpResponseBody{R"({"get": "ok"})"} };

    CHECK_SUCCESS_BODY(client.get(httpUrl, successPredicate), expected);
  }

  SECTION("Wrap Response")
  {
    HttpUrl httpUrl = url.with_path_segments(PathSegments{{PathSegment{"get"}}});
    Predicate<HttpStatusCode> successPredicate = successful();

    auto wrapped = client.get(httpUrl, successPredicate).template match<std::string>(
        [](const HttpFailure &failure){
          return failure.template match<std::string>(
              [](const HttpConnectionFailure &c) {
                return c.value();
              },
              [](const HttpResponse &s){
                return s.body.value();
              }
          );
        },
        [](const HttpSuccess &success){
          auto keys = nlohmann::json::parse(success.body().value());
          return keys["get"];
        }
    );

    CHECK(wrapped == "ok");
  }
}