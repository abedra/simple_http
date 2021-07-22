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

TEST_CASE("Trimming") {
  SECTION("Left Trim")
  {
    std::string candidate = "\n  test";
    CHECK(candidate != "test");
    CHECK(SimpleHttp::left_trim(candidate) == "test");
  }

  SECTION("Right Trim")
  {
    std::string candidate = "test \r\n";
    CHECK(candidate != "test");
    CHECK(SimpleHttp::right_trim(candidate) == "test");
  }

  SECTION("Trim")
  {
    std::string candidate = "\v test \f";
    CHECK(candidate != "test");
    CHECK(SimpleHttp::trim(candidate) == "test");
  }

  SECTION("Only trims at the outermost parts")
  {
    std::string candidate = "\t the \n thing \r works \v in \r\n  ways \f\f\n";
    CHECK(SimpleHttp::trim(candidate) == "the \n thing \r works \v in \r\n  ways");
  }
}

TEST_CASE("PathSegments") {
  SECTION("Path segments to_string no segments")
  {
    CHECK(SimpleHttp::PathSegments{}.to_string().empty());
  }

  SECTION("Path segments to_string one segment")
  {
    SimpleHttp::PathSegments segments{{SimpleHttp::PathSegment{"one"}}};
    CHECK(segments.to_string() == "/one");
  }

  SECTION("Path segments to_string multiple segment")
  {
    SimpleHttp::PathSegments segments{{
      SimpleHttp::PathSegment{"one"},
      SimpleHttp::PathSegment{"two"}
    }};

    CHECK(segments.to_string() == "/one/two");
  }
}

TEST_CASE("HttpUrl") {
  SECTION("path_segments parsed constructor, no segments")
  {
    SimpleHttp::HttpUrl url{"https://example.com"};
    CHECK(url.path_segments().value().empty());
    CHECK(url.path_segments().to_string().empty());
  }

  SECTION("path_segments parsed constructor, one segment")
  {
    SimpleHttp::HttpUrl url{"https://example.com/one"};
    // This exposes the lack of complete parsing when constructed with a string value
    CHECK(url.path_segments().value().empty());
    CHECK(url.path_segments().to_string().empty());
  }

  SECTION("path_segments parsed constructor, one segment")
  {
    SimpleHttp::HttpUrl url{"https://example.com/one/two"};
    // This exposes the lack of complete parsing when constructed with a string value
    CHECK(url.path_segments().value().empty());
    CHECK(url.path_segments().to_string().empty());
  }

  SECTION("Fully constructed, multiple segments")
  {
    SimpleHttp::HttpUrl url = SimpleHttp::HttpUrl()
        .with_protocol(SimpleHttp::Protcol{"https"})
        .with_host(SimpleHttp::Host{"example.com"})
        .with_path_segments(SimpleHttp::PathSegments{{
          SimpleHttp::PathSegment{"one"},
          SimpleHttp::PathSegment{"two"}
        }});

    CHECK(url.path_segments().to_string() == "/one/two");
  }

  SECTION("value, when constructed with a string")
  {
    CHECK(SimpleHttp::HttpUrl{"http://example.com"}.value() == "http://example.com");
  }
}