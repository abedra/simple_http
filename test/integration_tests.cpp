#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "json.hpp"
#include "../simple_http.hpp"

TEST_CASE("Integration Tests")
{
    SimpleHttp::Client client;
    
    SECTION("GET Request")
    {
        auto maybe_response = client.get(SimpleHttp::Url{"http://localhost:5000/get"});

        REQUIRE(maybe_response);

        SimpleHttp::HttpResponse response = maybe_response.value();
        auto keys = nlohmann::json::parse(response.body.value());

        CHECK(keys["get"] == "ok");
    }

    SECTION("Post request")
    {
        auto maybe_response = client.post(SimpleHttp::Url{"http://localhost:5000/post"},
                                          SimpleHttp::HttpRequestBody{"{\"name\":\"test\"}"},
                                          {{"Content-Type", "application/json"}});

        REQUIRE(maybe_response);

        SimpleHttp::HttpResponse response = maybe_response.value();
        auto keys = nlohmann::json::parse(response.body.value());

        CHECK(keys["hello"] == "test");
    }

    SECTION("Put request")
    {
        auto maybe_response = client.put(SimpleHttp::Url{"http://localhost:5000/put"},
                                         SimpleHttp::HttpRequestBody{"{\"update\":\"test\"}"},
                                         {{"Content-Type", "application/json"}});

        REQUIRE(maybe_response);

        SimpleHttp::HttpResponse response = maybe_response.value();
        auto keys = nlohmann::json::parse(response.body.value());

        CHECK(keys["update"] == "test");
    }

    SECTION("Delete request")
    {
        auto maybe_response = client.del(SimpleHttp::Url{"http://localhost:5000/delete"});

        REQUIRE(maybe_response);
        CHECK(maybe_response.value().status == SimpleHttp::OK);
    }

    SECTION("Head request")
    {
        auto maybe_response = client.head(SimpleHttp::Url{"http://localhost:5000/get"});
        REQUIRE(maybe_response);
        CHECK(maybe_response.value().status == SimpleHttp::OK);
    }

    SECTION("Options request")
    {
        auto maybe_response = client.options(SimpleHttp::Url{"http://localhost:5000/get"});

        REQUIRE(maybe_response);

        SimpleHttp::Headers headers = maybe_response.value().headers.value();
        std::vector<std::string> methods = SimpleHttp::vec(headers.at("Allow"), ',');
        CHECK_THAT(methods, Catch::UnorderedEquals(std::vector<std::string>{"HEAD", "OPTIONS", "GET"}));
    }

    SECTION("Trace request")
    {
        auto maybe_response = client.trace(SimpleHttp::Url{"http://localhost:5000/trace"});

        REQUIRE(maybe_response);
        CHECK(maybe_response.value().headers.value().at("Content-Type") == "message/http");
    }

    SECTION("Connection error")
    {
        std::string error;
        SimpleHttp::Client client{
            [&error](auto &err) { error = "Error: " + err; }
        };

        auto maybe_response = client.get(SimpleHttp::Url{"error"});

        REQUIRE(!maybe_response);
        CHECK(error == "Error: Couldn't resolve host name");
    }

    SECTION("Post request that expects a 204 NO_CONTENT response")
    {
        auto maybe_response = client.post(SimpleHttp::Url{"http://localhost:5000/empty_post_response"},
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
        auto maybe_response = client.get(SimpleHttp::Url{"http://localhost:5000/empty_post_response"},
                                         SimpleHttp::eq(SimpleHttp::METHOD_NOT_ALLOWED));

        REQUIRE(maybe_response);
        CHECK(maybe_response.value().status == SimpleHttp::METHOD_NOT_ALLOWED);
    }
}