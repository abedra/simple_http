#pragma once

#include <curl/curl.h>
#include <string>
#include <sstream>
#include <optional>
#include <functional>
#include <algorithm>
#include <utility>

namespace SimpleHttp {

template<typename Name, typename A>
struct Tiny {
    Tiny() noexcept(std::is_nothrow_constructible_v<A>) = default;
    explicit Tiny(const A &value) noexcept(std::is_nothrow_copy_constructible_v<A>) : value_(value) {}
    explicit Tiny(A &&value) noexcept(std::is_nothrow_move_constructible_v<A>) : value_(std::move(value)) {}

    friend std::ostream &operator<<(std::ostream &os, const Tiny &object) {
      return os << object.value();
    }

    friend bool operator==(const Tiny &lhs, const Tiny &rhs) {
      return lhs.value() == rhs.value();
    }

    friend bool operator<(const Tiny &lhs, const Tiny &rhs) {
      return lhs.value() < rhs.value();
    }

    friend bool operator<=(const Tiny &lhs, const Tiny &rhs) {
      return lhs.value() <= rhs.value();
    }

    friend bool operator>(const Tiny &lhs, const Tiny &rhs) {
      return lhs.value() > rhs.value();
    }

    friend bool operator>=(const Tiny &lhs, const Tiny &rhs) {
      return lhs.value() >= rhs.value();
    }

    explicit operator A& () const noexcept {
      return value();
    }

    [[nodiscard]] 
    const A& value() const noexcept {
      return value_;
    }

    [[nodiscard]]
    std::string toString() const {
      std::ostringstream ss;
      ss << value();
      return ss.str();
    }

protected:
    A value_;
};

#define SIMPLE_HTTP_TINY_STRING(Name)           \
  struct Name##Detail {};                       \
  using Name = Tiny<Name##Detail, std::string>; \

#define SIMPLE_HTTP_TINY_LONG(Name)      \
  struct Name##Detail {};                \
  using Name = Tiny<Name##Detail, long>; \

SIMPLE_HTTP_TINY_STRING(HttpResponseBody)
SIMPLE_HTTP_TINY_STRING(HttpRequestBody)
SIMPLE_HTTP_TINY_STRING(Protcol)
SIMPLE_HTTP_TINY_STRING(Host)
SIMPLE_HTTP_TINY_STRING(PathSegment)
SIMPLE_HTTP_TINY_STRING(QueryParameterKey)
SIMPLE_HTTP_TINY_STRING(QueryParameterValue)

SIMPLE_HTTP_TINY_LONG(HttpStatusCode)

#undef SIMPLE_HTTP_TINY_STRING
#undef SIMPLE_HTTP_TINY_LONG

using CurlSetupCallback = std::function<void(CURL *curl)>;
inline CurlSetupCallback NoopCurlSetupCallback = [](auto){};

using CurlHeaderCallback = std::function<curl_slist*(curl_slist *chunk)>;
inline CurlHeaderCallback NoopCurlHeaderCallback = [](curl_slist *chunk){ return chunk; };

using ErrorCallback = std::function<void(const std::string&)>;
inline ErrorCallback NoopErrorCallback = [](auto&){};

using Headers = std::unordered_map<std::string, std::string>;
using PathSegments = std::vector<PathSegment>;
using QueryParameters = std::vector<std::pair<QueryParameterKey, QueryParameterValue>>;

inline static const std::string WHITESPACE = "\n\t\f\v\r ";

static std::string leftTrim(const std::string &candidate) {
  size_t start = candidate.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : candidate.substr(start);
}

static std::string rightTrim(const std::string &candidate) {
  size_t end = candidate.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : candidate.substr(0, end + 1);
}

static std::string trim(const std::string &candidate) {
  return rightTrim(leftTrim(candidate));
}

static std::vector<std::string> vec(const std::string& candidate, const char separator) {
  std::vector<std::string> container;
  std::stringstream ss(candidate);
  std::string temp;
  while (std::getline(ss, temp, separator)) {
    container.push_back(trim(temp));
  }
  return container;
}

struct HttpResponseHeaders {
  explicit HttpResponseHeaders(Headers headers) : headers_(std::move(headers)) {}
  explicit HttpResponseHeaders(const std::string &header_string) : headers_(parse(header_string)) {}

  const Headers& value() const {
    return headers_;
  }

private:
  Headers headers_;

  static Headers parse(const std::string &header_string) {
    std::stringstream ss(header_string);
    std::string container;
    Headers headers;
    while(std::getline(ss, container, '\n')) {
      std::size_t pos = container.find(':');
      if (pos != std::string::npos) {
        headers.emplace(
          trim(container.substr(0, pos)),
          trim(container.substr(pos + 1))
        );
      }
    }
    return headers;
  }
};

struct HttpResponse {
  HttpStatusCode status;
  HttpResponseHeaders headers;
  HttpResponseBody body;
};

struct HttpUrl {
  HttpUrl() = default;

  HttpUrl(Protcol protocol,
          Host host,
          PathSegments path_segments = {},
          QueryParameters query_parameters = {})
    : protocol_(std::move(protocol))
    , host_(std::move(host))
    , path_segments_(std::move(path_segments))
    , query_parameters_(std::move(query_parameters))
    , value_(to_string())
  { }

  explicit HttpUrl(std::string value)
    : protocol_(detect_protocol(value))
    , value_(std::move(value))
  { }

  [[nodiscard]]
  const Protcol & protocol() {
    return protocol_;
  }
  
  [[nodiscard]]
  std::string value() {
    if (value_.empty()) {
      value_ = to_string();
    }
    return value_;
  }

  [[nodiscard]]
  HttpUrl& with_protocol(Protcol protocol) {
    protocol_ = std::move(protocol);
    return *this;
  }

  [[nodiscard]]
  HttpUrl& with_host(Host host) {
    host_ = std::move(host);
    return *this;
  }

  [[nodiscard]]
  HttpUrl& with_path_segments(PathSegments path_segments) {
    path_segments_ = std::move(path_segments);
    return *this;
  }

  [[nodiscard]]
  HttpUrl& with_query_parameters(QueryParameters query_parameters) {
    query_parameters_ = std::move(query_parameters);
    return *this;
  }

private:
  Protcol protocol_;
  Host host_;
  PathSegments path_segments_;
  QueryParameters query_parameters_;
  std::string value_;

  static std::string detect_protocol(const std::string &url_string) {
    unsigned long i = url_string.find(':');
    if (i != std::string::npos) {
      return url_string.substr(0, i);
    } else {
      return "unknown";
    }
  }
  
  [[nodiscard]]
  std::string to_string() {
    std::stringstream ss;
    ss << protocol_ << "://" << host_;
    if (!path_segments_.empty()) {
      for (const auto& segment : path_segments_) {
        ss << "/" << segment;
      }
    }

    if (!query_parameters_.empty()) {
      ss << "?" << query_parameters_[0].first << "=" << query_parameters_[0].second;
      for (long unsigned int i = 1; i < query_parameters_.size(); ++i) {
        ss << "&" << query_parameters_[i].first << "=" << query_parameters_[i].second;
      }
    }

    return trim(ss.str());
  }
};

template<class A>
using Predicate = std::function<bool(const A &a)>;

template<class A>
Predicate<A> eq(const A &a) {
  return [a](const A &other) {
    return a == other;
  };
}

template<class A>
Predicate<A> between_inclusive(const A &a, const A &b) {
  return [a, b](const A &other) {
    return other >= a && other <= b;
  };
}

// Information Responses
inline HttpStatusCode CONTINUE = HttpStatusCode{100};
inline HttpStatusCode SWITCHING_PROTOCOL = HttpStatusCode{101};
inline HttpStatusCode PROCESSING = HttpStatusCode{102};
inline HttpStatusCode EARLY_HINTS = HttpStatusCode{103};

// Successful Responses
inline HttpStatusCode OK = HttpStatusCode{200};
inline HttpStatusCode CREATED = HttpStatusCode{201};
inline HttpStatusCode ACCEPTED = HttpStatusCode{202};
inline HttpStatusCode NON_AUTHORITATIVE_INFORMATION = HttpStatusCode{203};
inline HttpStatusCode NO_CONTENT = HttpStatusCode{204};
inline HttpStatusCode RESET_CONTENT = HttpStatusCode{205};
inline HttpStatusCode PARTIAL_CONTENT = HttpStatusCode{206};
inline HttpStatusCode MULTI_STATUS = HttpStatusCode{207};
inline HttpStatusCode ALREADY_REPORTED = HttpStatusCode{208};
inline HttpStatusCode IM_USED = HttpStatusCode{226};

// Redirection Messages
inline HttpStatusCode MULTIPLE_CHOICE = HttpStatusCode{300};
inline HttpStatusCode MOVED_PERMANENTLY = HttpStatusCode{301};
inline HttpStatusCode FOUND = HttpStatusCode{302};
inline HttpStatusCode SEE_OTHER = HttpStatusCode{303};
inline HttpStatusCode NOT_MODIFIED = HttpStatusCode{304};
inline HttpStatusCode USE_PROXY = HttpStatusCode{305};
inline HttpStatusCode TEMPORARY_REDIRECT = HttpStatusCode{307};
inline HttpStatusCode PERMANENT_REDIRECT = HttpStatusCode{308};

// Client Error Responses
inline HttpStatusCode BAD_REQUEST = HttpStatusCode{400};
inline HttpStatusCode UNAUTHORIZED = HttpStatusCode{401};
inline HttpStatusCode PAYMENT_REQUIRED = HttpStatusCode{402};
inline HttpStatusCode FORBIDDEN = HttpStatusCode{403};
inline HttpStatusCode NOT_FOUND = HttpStatusCode{404};
inline HttpStatusCode METHOD_NOT_ALLOWED = HttpStatusCode{405};
inline HttpStatusCode NOT_ACCEPTABLE = HttpStatusCode{406};
inline HttpStatusCode PROXY_AUTHENTICATION_REQUIRED = HttpStatusCode{407};
inline HttpStatusCode REQUEST_TIMEOUT = HttpStatusCode{408};
inline HttpStatusCode CONFLICT = HttpStatusCode{409};
inline HttpStatusCode GONE = HttpStatusCode{410};
inline HttpStatusCode LENGTH_REQUIRED = HttpStatusCode{411};
inline HttpStatusCode PRECONDITION_FAILED = HttpStatusCode{412};
inline HttpStatusCode PAYLOAD_TOO_LARGE = HttpStatusCode{413};
inline HttpStatusCode URI_TOO_LONG = HttpStatusCode{414};
inline HttpStatusCode UNSUPPORTED_MEDIA_TYPE = HttpStatusCode{415};
inline HttpStatusCode RANGE_NOT_SATISFIABLE = HttpStatusCode{416};
inline HttpStatusCode EXPECTATION_FAILED = HttpStatusCode{417};
inline HttpStatusCode IM_A_TEAPOT = HttpStatusCode{418};
inline HttpStatusCode UNPROCESSABLE_ENTITY = HttpStatusCode{422};
inline HttpStatusCode FAILED_DEPENDENCY = HttpStatusCode{424};
inline HttpStatusCode TOO_EARLY = HttpStatusCode{425};
inline HttpStatusCode UPGRADE_REQUIRED = HttpStatusCode{426};
inline HttpStatusCode PRECONDITION_REQUIRED = HttpStatusCode{428};
inline HttpStatusCode TOO_MANY_REQUESTS = HttpStatusCode{429};
inline HttpStatusCode REQUEST_HEADER_FIELDS_TOO_LARGE = HttpStatusCode{431};
inline HttpStatusCode UNAVAILABLE_FOR_LEGAL_REASONS = HttpStatusCode{451};

// Server Error Responses
inline HttpStatusCode INTERNAL_SERVER_ERROR = HttpStatusCode{500};
inline HttpStatusCode NOT_IMPLEMENTED = HttpStatusCode{501};
inline HttpStatusCode BAD_GATEWAY = HttpStatusCode{502};
inline HttpStatusCode SERVICE_UNAVAILABLE = HttpStatusCode{503};
inline HttpStatusCode GATEWAY_TIMEOUT = HttpStatusCode{504};
inline HttpStatusCode HTTP_VERSION_NOT_SUPPORTED = HttpStatusCode{505};
inline HttpStatusCode VARIANT_ALSO_NEGOTIATES = HttpStatusCode{506};
inline HttpStatusCode INSUFFICIENT_STORAGE = HttpStatusCode{507};
inline HttpStatusCode LOOP_DETECTED = HttpStatusCode{508};
inline HttpStatusCode NOT_EXTENDED = HttpStatusCode{510};
inline HttpStatusCode NETWORK_AUTHENTICATION_REQUIRED = HttpStatusCode{511};

inline bool informational(const HttpStatusCode &subject) {
  return between_inclusive(CONTINUE, EARLY_HINTS)(subject);
}

inline bool success(const HttpStatusCode &subject) {
  // TODO: an exact list would be more correct
  return between_inclusive(OK, IM_USED)(subject);
}

inline bool redirect(const HttpStatusCode &subject) {
  return between_inclusive(MULTIPLE_CHOICE, PERMANENT_REDIRECT)(subject);
}

inline bool client_error(const HttpStatusCode &subject) {
  return between_inclusive(BAD_REQUEST, UNAVAILABLE_FOR_LEGAL_REASONS)(subject);
}

inline bool server_error(const HttpStatusCode &subject) {
  return between_inclusive(INTERNAL_SERVER_ERROR, NETWORK_AUTHENTICATION_REQUIRED)(subject);
}

struct Client {
  Client() : error_callback_(NoopErrorCallback), debug_(false), verify_(true) {}
  explicit Client(ErrorCallback error_callback) : error_callback_(std::move(error_callback)), debug_(false), verify_(true) {}

  Client& with_tls_verification(bool verify) {
    verify_ = verify;
    return *this;
  }

  Client& with_debug(bool debug) {
    debug_ = debug;
    return *this;
  }

  Client& with_error_callback(ErrorCallback error_callback) {
    error_callback_ = std::move(error_callback);
    return *this;
  }

  [[nodiscard]]
  std::optional<HttpResponse> get(const HttpUrl &url,
                                  const Headers &headers = {}) {
    return get(url, eq(OK), headers);
  }

  [[nodiscard]]
  std::optional<HttpResponse> get(const HttpUrl &url,
                                  const Predicate<HttpStatusCode> &successPredicate,
                                  const Headers &headers = {}) {
    return execute(url, make_header_callback(headers), NoopCurlSetupCallback, successPredicate);
  }

  [[nodiscard]]
  std::optional<HttpResponse> post(const HttpUrl &url,
                                   const HttpRequestBody &body,
                                   const Headers &headers = {}) {
    return post(url, body, eq(OK), headers);
  }

  [[nodiscard]]
  std::optional<HttpResponse> post(const HttpUrl &url,
                                   const HttpRequestBody &body,
                                   const Predicate<HttpStatusCode> &successPredicate,
                                   const Headers &headers = {}) {
    CurlSetupCallback setup = [&](CURL *curl) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.value().c_str());
    };

    return execute(url, make_header_callback(headers), setup, successPredicate);
  }

  [[nodiscard]]
  std::optional<HttpResponse> put(const HttpUrl &url,
                                  const HttpRequestBody &body,
                                  const Headers &headers = {}) {
    return put(url, body, eq(OK), headers);
  }

  [[nodiscard]]
  std::optional<HttpResponse> put(const HttpUrl &url,
                                  const HttpRequestBody &body,
                                  const Predicate<HttpStatusCode> &successPredicate,
                                  const Headers &headers = {}) {
      CurlSetupCallback setup = [&](CURL *curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.value().c_str());
      };

      return execute(url, make_header_callback(headers), setup, successPredicate);
  }

  [[nodiscard]]
  std::optional<HttpResponse> del(const HttpUrl &url,
                                  const Headers &headers = {}) {
    return del(url, eq(OK), headers);
  }

  [[nodiscard]]
  std::optional<HttpResponse> del(const HttpUrl &url,
                                  const Predicate<HttpStatusCode> &successPredicate,
                                  const Headers &headers = {}) {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    };
    
    return execute(url, make_header_callback(headers), setup, successPredicate);
  }

  [[nodiscard]]
  std::optional<HttpResponse> head(const HttpUrl &url) {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    };

    return execute(url, NoopCurlHeaderCallback, setup, eq(OK));
  }

  [[nodiscard]]
  std::optional<HttpResponse> options(const HttpUrl &url) {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
    };

    return execute(url, NoopCurlHeaderCallback, setup, eq(OK));
  }

  [[nodiscard]]
  std::optional<HttpResponse> trace(const HttpUrl &url) {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "TRACE");
    };

    return execute(url, NoopCurlHeaderCallback, setup, eq(OK));
  }

  [[nodiscard]]
  std::optional<HttpResponse> execute(HttpUrl url,
                                      const CurlHeaderCallback &curl_header_callback,
                                      const CurlSetupCallback &curl_setup_callback,
                                      const Predicate<HttpStatusCode> &successPredicate) {
      std::string body_buffer;
      std::string header_buffer;
      long rawStatusCode = 0;

      CURL *curl = curl_easy_init();
      if (curl) {
          struct curl_slist *chunk = nullptr;

          curl_easy_setopt(curl, CURLOPT_URL, url.value().c_str());
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body_buffer);          
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_header_callback(chunk));
          curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_callback);
          curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_buffer);
          curl_easy_setopt(curl, CURLOPT_VERBOSE, debug_);
          curl_setup_callback(curl);

          if (url.protocol().value() == "https") {
            verify_
                ? curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L)
                : curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
          }

          CURLcode res = curl_easy_perform(curl);
          if (res != CURLE_OK) {
            error_callback_(curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            curl_slist_free_all(chunk);
            return std::nullopt;
          }

          curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rawStatusCode);
          curl_easy_cleanup(curl);
          curl_slist_free_all(chunk);
      }

      HttpStatusCode status{rawStatusCode};
      if (successPredicate(status)) {
        return std::optional<HttpResponse>({
            status, 
            HttpResponseHeaders{header_buffer}, 
            HttpResponseBody{body_buffer}
          });
      } else {
        return std::nullopt;
      }
  }

private:
  ErrorCallback error_callback_;
  bool debug_;
  bool verify_;

  static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
  }

  static CurlHeaderCallback make_header_callback(const Headers &headers) {
    return headers.empty() 
      ? NoopCurlHeaderCallback 
      : [&headers](curl_slist *chunk) {
          for (const auto& [key, value] : headers) {
            chunk = curl_slist_append(chunk, (key + ": " + value).c_str());
          }
          
          return chunk;
      };
  }
};
}  // namespace SimpleHttp
