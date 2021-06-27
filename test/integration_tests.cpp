#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "json.hpp"
#include "../simple_http.hpp"

TEST_CASE("Integration Tests")
{
    SimpleHttp::Client client;
    
    SECTION("GET Request")
    {
        auto maybeResponse = client.get(SimpleHttp::Url{"http://localhost:5000/get"});

        REQUIRE(maybeResponse);

        SimpleHttp::HttpResponse response = maybeResponse.value();
        auto keys = nlohmann::json::parse(response.body.value());

        CHECK(keys["get"] == "ok");
    }

    SECTION("Post request")
    {
        auto maybeResponse = client.post(SimpleHttp::Url{"http://localhost:5000/post"},
                                         SimpleHttp::HttpRequestBody{"{\"name\":\"test\"}"},
                                         {{"Content-Type", "application/json"}});

        REQUIRE(maybeResponse);

        SimpleHttp::HttpResponse response = maybeResponse.value();
        auto keys = nlohmann::json::parse(response.body.value());

        CHECK(keys["hello"] == "test");
    }

    SECTION("Put request")
    {
        auto maybeResponse = client.put(SimpleHttp::Url{"http://localhost:5000/put"},
                                        SimpleHttp::HttpRequestBody{"{\"update\":\"test\"}"},
                                        {{"Content-Type", "application/json"}});

        REQUIRE(maybeResponse);

        SimpleHttp::HttpResponse response = maybeResponse.value();
        auto keys = nlohmann::json::parse(response.body.value());

        CHECK(keys["update"] == "test");
    }

    SECTION("Delete request")
    {
        auto maybeResponse = client.del(SimpleHttp::Url{"http://localhost:5000/delete"});

        REQUIRE(maybeResponse);
        CHECK(maybeResponse.value().status == SimpleHttp::OK);
    }

    SECTION("Connection error")
    {
        std::string error;
        SimpleHttp::Client client{
            [&error](auto &err) { error = "Error: " + err; }
        };

        auto maybeResponse = client.get(SimpleHttp::Url{"error"});

        REQUIRE(!maybeResponse);
        CHECK(error == "Error: Couldn't resolve host name");
    }

    SECTION("Post request that expects a 204 NO_CONTENT response")
    {
        auto maybeResponse = client.post(SimpleHttp::Url{"http://localhost:5000/empty_post_response"},
                                         SimpleHttp::HttpRequestBody{""},
                                         SimpleHttp::eq(SimpleHttp::NO_CONTENT),
                                         {{"Content-Type", "application/json"}});

        REQUIRE(maybeResponse);
    
        SimpleHttp::HttpResponse response = maybeResponse.value();
        
        CHECK(response.status == SimpleHttp::NO_CONTENT);
        CHECK(response.body.value().empty());
    }

    SECTION("Get request that expects a 405 METHOD_NOT_ALLOWED response")
    {
        auto maybeResponse = client.get(SimpleHttp::Url{"http://localhost:5000/empty_post_response"},
                                        SimpleHttp::eq(SimpleHttp::METHOD_NOT_ALLOWED));

        REQUIRE(maybeResponse);
        CHECK(maybeResponse.value().status == SimpleHttp::METHOD_NOT_ALLOWED);
    }
}