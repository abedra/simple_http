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
    auto maybe_response = client.get(url.with_path_segments({SimpleHttp::PathSegment{"get"}}));

    REQUIRE(maybe_response);

    SimpleHttp::HttpResponse response = maybe_response.value();
    auto keys = nlohmann::json::parse(response.body.value());

    CHECK(keys["get"] == "ok");
  }

  SECTION("Post request")
  {
    auto maybe_response = client.post(url.with_path_segments({SimpleHttp::PathSegment{"post"}}),
                                      SimpleHttp::HttpRequestBody{R"({"name":"test"})"},
                                      {{"Content-Type", "application/json"}});

    REQUIRE(maybe_response);

    SimpleHttp::HttpResponse response = maybe_response.value();
    auto keys = nlohmann::json::parse(response.body.value());

    CHECK(keys["hello"] == "test");
  }

  SECTION("Put request")
  {
    auto maybe_response = client.put(url.with_path_segments({SimpleHttp::PathSegment{"put"}}),
                                     SimpleHttp::HttpRequestBody{R"({"update":"test"})"},
                                     {{"Content-Type", "application/json"}});

    REQUIRE(maybe_response);

    SimpleHttp::HttpResponse response = maybe_response.value();
    auto keys = nlohmann::json::parse(response.body.value());

    CHECK(keys["update"] == "test");
  }

  SECTION("Delete request")
  {
    auto maybe_response = client.del(url.with_path_segments({SimpleHttp::PathSegment{"delete"}}));

    REQUIRE(maybe_response);
    CHECK(maybe_response.value().status == SimpleHttp::OK);
  }

  SECTION("Head request")
  {
    auto maybe_response = client.head(url.with_path_segments({SimpleHttp::PathSegment{"get"}}));

    REQUIRE(maybe_response);
    CHECK(maybe_response.value().status == SimpleHttp::OK);
  }

  SECTION("Options request")
  {
    auto maybe_response = client.options(url.with_path_segments({SimpleHttp::PathSegment{"get"}}));

    REQUIRE(maybe_response);

    SimpleHttp::Headers headers = maybe_response.value().headers.value();
    CHECK_THAT(SimpleHttp::vec(headers.at("Allow"), ','),
               Catch::UnorderedEquals(std::vector<std::string>{"HEAD", "OPTIONS", "GET"}));
  }

  SECTION("Trace request")
  {
    auto maybe_response = client.trace(url.with_path_segments({SimpleHttp::PathSegment{"trace"}}));

    REQUIRE(maybe_response);
    CHECK(maybe_response.value().headers.value().at("Content-Type") == "message/http");
  }

  SECTION("Connection error")
  {
    std::string error;
    client = client.with_error_callback([&error](auto &err) { error = "Error: " + err; });
    auto maybe_response = client.get(url.with_protocol(SimpleHttp::Protcol{"zxcv"}));

    REQUIRE(!maybe_response);
    CHECK(error == "Error: Unsupported protocol");
  }

  SECTION("Post request that expects a 204 NO_CONTENT response")
  {
    auto maybe_response = client.post(url.with_path_segments({SimpleHttp::PathSegment{"empty_post_response"}}),
                                      SimpleHttp::HttpRequestBody{""},
                                      SimpleHttp::eq(SimpleHttp::NO_CONTENT),
                                      {{"Content-Type", "application/json"}});

    REQUIRE(maybe_response);

    SimpleHttp::HttpResponse response = maybe_response.value();

    CHECK(response.status == SimpleHttp::NO_CONTENT);
    CHECK(response.body.value().empty());
  }

  SECTION("Get request that expects a 405 METHOD_NOT_ALLOWED response")
  {
    auto maybe_response = client.get(url.with_path_segments({SimpleHttp::PathSegment{"empty_post_response"}}),
                                     SimpleHttp::eq(SimpleHttp::METHOD_NOT_ALLOWED));

    REQUIRE(maybe_response);
    CHECK(maybe_response.value().status == SimpleHttp::METHOD_NOT_ALLOWED);
  }

  SECTION("Ranged response success predicate")
  {
    auto maybe_response = client.get(url.with_path_segments({SimpleHttp::PathSegment{"get"}}),
                                     SimpleHttp::successful());

    REQUIRE(maybe_response);
    SimpleHttp::HttpResponse response = maybe_response.value();
    auto keys = nlohmann::json::parse(response.body.value());

    CHECK(keys["get"] == "ok");
  }
}