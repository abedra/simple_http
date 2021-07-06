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
    SimpleHttp::Predicate<SimpleHttp::HttpStatusCode> informational = SimpleHttp::informational();

    CHECK(informational(SimpleHttp::CONTINUE));
    CHECK(informational(SimpleHttp::PROCESSING));
    CHECK(informational(SimpleHttp::EARLY_HINTS));
    CHECK(!informational(SimpleHttp::OK));
  }

  SECTION("successful")
  {
    SimpleHttp::Predicate<SimpleHttp::HttpStatusCode> successful = SimpleHttp::successful();

    CHECK(successful(SimpleHttp::OK));
    CHECK(successful(SimpleHttp::IM_USED));
    CHECK(successful(SimpleHttp::NO_CONTENT));
    CHECK(!successful(SimpleHttp::INTERNAL_SERVER_ERROR));

    //TODO: this exposes the issue with not using an explicit set
    CHECK(successful(SimpleHttp::HttpStatusCode{209}));
  }

  SECTION("redirect")
  {
    SimpleHttp::Predicate<SimpleHttp::HttpStatusCode> redirect = SimpleHttp::redirect();

    CHECK(redirect(SimpleHttp::MULTIPLE_CHOICE));
    CHECK(redirect(SimpleHttp::TEMPORARY_REDIRECT));
    CHECK(redirect(SimpleHttp::PERMANENT_REDIRECT));
    CHECK(!redirect(SimpleHttp::OK));
  }

  SECTION("client error")
  {
    SimpleHttp::Predicate<SimpleHttp::HttpStatusCode> client_error = SimpleHttp::client_error();

    CHECK(client_error(SimpleHttp::BAD_REQUEST));
    CHECK(client_error(SimpleHttp::PAYMENT_REQUIRED));
    CHECK(client_error(SimpleHttp::UNAVAILABLE_FOR_LEGAL_REASONS));
    CHECK(!client_error(SimpleHttp::OK));
  }

  SECTION("server error")
  {
    SimpleHttp::Predicate<SimpleHttp::HttpStatusCode> server_error = SimpleHttp::server_error();

    CHECK(server_error(SimpleHttp::INTERNAL_SERVER_ERROR));
    CHECK(server_error(SimpleHttp::GATEWAY_TIMEOUT));
    CHECK(server_error(SimpleHttp::NETWORK_AUTHENTICATION_REQUIRED));
    CHECK(!server_error(SimpleHttp::OK));
  }
}
