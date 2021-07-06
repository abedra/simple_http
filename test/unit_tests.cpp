#include "catch.hpp"
#include "../simple_http.hpp"

TEST_CASE("Predicates")
{
  SECTION("eq")
  {
    SimpleHttp::Predicate<int> subject = SimpleHttp::eq(1);
    CHECK(subject(1));
    CHECK(!subject(2));
  }

  SECTION("between_inclusive")
  {
    SimpleHttp::Predicate<int> subject = SimpleHttp::between_inclusive(1, 10);
    CHECK(subject(1));
    CHECK(subject(5));
    CHECK(subject(10));
    CHECK(!subject(0));
    CHECK(!subject(11));
  }

  SECTION("informational")
  {
    CHECK(SimpleHttp::informational(SimpleHttp::CONTINUE));
    CHECK(SimpleHttp::informational(SimpleHttp::PROCESSING));
    CHECK(SimpleHttp::informational(SimpleHttp::EARLY_HINTS));
    CHECK(!SimpleHttp::informational(SimpleHttp::OK));
  }

  SECTION("successful")
  {
    CHECK(SimpleHttp::success(SimpleHttp::OK));
    CHECK(SimpleHttp::success(SimpleHttp::IM_USED));
    CHECK(SimpleHttp::success(SimpleHttp::NO_CONTENT));
    CHECK(!SimpleHttp::success(SimpleHttp::INTERNAL_SERVER_ERROR));

    //TODO: this exposes the issue with not using an explicit set
    CHECK(SimpleHttp::success(SimpleHttp::HttpStatusCode{209}));
  }

  SECTION("redirect")
  {
    CHECK(SimpleHttp::redirect(SimpleHttp::MULTIPLE_CHOICE));
    CHECK(SimpleHttp::redirect(SimpleHttp::TEMPORARY_REDIRECT));
    CHECK(SimpleHttp::redirect(SimpleHttp::PERMANENT_REDIRECT));
    CHECK(!SimpleHttp::redirect(SimpleHttp::OK));
  }

  SECTION("client error")
  {
    CHECK(SimpleHttp::client_error(SimpleHttp::BAD_REQUEST));
    CHECK(SimpleHttp::client_error(SimpleHttp::PAYMENT_REQUIRED));
    CHECK(SimpleHttp::client_error(SimpleHttp::UNAVAILABLE_FOR_LEGAL_REASONS));
    CHECK(!SimpleHttp::client_error(SimpleHttp::OK));
  }

  SECTION("server error")
  {
    CHECK(SimpleHttp::server_error(SimpleHttp::INTERNAL_SERVER_ERROR));
    CHECK(SimpleHttp::server_error(SimpleHttp::GATEWAY_TIMEOUT));
    CHECK(SimpleHttp::server_error(SimpleHttp::NETWORK_AUTHENTICATION_REQUIRED));
    CHECK(!SimpleHttp::server_error(SimpleHttp::OK));
  }
}
