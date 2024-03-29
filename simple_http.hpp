#pragma once

#include <algorithm>
#include <curl/curl.h>
#include <functional>
#include <numeric>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace SimpleHttp {

template <class... As> struct visitor : As... {
  using As::operator()...;
};
template <class... As> visitor(As...) -> visitor<As...>;

template <typename Name, typename A> struct Tiny {
  Tiny() noexcept(std::is_nothrow_constructible_v<A>) = default;
  explicit Tiny(const A &value) noexcept(
      std::is_nothrow_copy_constructible_v<A>)
      : value_(value) {}
  explicit Tiny(A &&value) noexcept(std::is_nothrow_move_constructible_v<A>)
      : value_(std::move(value)) {}

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

  explicit operator A &() const noexcept { return value(); }

  [[nodiscard]] const A &value() const noexcept { return value_; }

  [[nodiscard]] std::string to_string() const {
    std::ostringstream ss;
    ss << value();
    return ss.str();
  }

protected:
  A value_;
};

#define SIMPLE_HTTP_TINY_STRING(Name)                                          \
  struct Name##Detail {};                                                      \
  using Name = Tiny<Name##Detail, std::string>;

#define SIMPLE_HTTP_TINY_int64_t(Name)                                         \
  struct Name##Detail {};                                                      \
  using Name = Tiny<Name##Detail, int64_t>;

SIMPLE_HTTP_TINY_STRING(HttpConnectionFailure)
SIMPLE_HTTP_TINY_STRING(HttpResponseBody)
SIMPLE_HTTP_TINY_STRING(HttpRequestBody)
SIMPLE_HTTP_TINY_STRING(Protcol)
SIMPLE_HTTP_TINY_STRING(Host)
SIMPLE_HTTP_TINY_STRING(PathSegment)
SIMPLE_HTTP_TINY_STRING(QueryParameterKey)
SIMPLE_HTTP_TINY_STRING(QueryParameterValue)

SIMPLE_HTTP_TINY_int64_t(HttpStatusCode)

#undef SIMPLE_HTTP_TINY_STRING
#undef SIMPLE_HTTP_TINY_int64_t

    using CurlSetupCallback = std::function<void(CURL *curl)>;
inline static CurlSetupCallback NoopCurlSetupCallback = [](auto) {};

using CurlHeaderCallback = std::function<curl_slist *(curl_slist *chunk)>;
inline static CurlHeaderCallback NoopCurlHeaderCallback =
    [](curl_slist *chunk) { return chunk; };

using Headers = std::unordered_map<std::string, std::string>;

inline static const std::string WHITESPACE = "\n\t\f\v\r ";

inline static std::string left_trim(const std::string &candidate) {
  size_t start = candidate.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : candidate.substr(start);
}

inline static std::string right_trim(const std::string &candidate) {
  size_t end = candidate.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : candidate.substr(0, end + 1);
}

inline static std::string trim(const std::string &candidate) {
  return right_trim(left_trim(candidate));
}

inline static std::vector<std::string> vec(const std::string &candidate,
                                           const char separator) {
  std::vector<std::string> container;
  std::stringstream ss(candidate);
  std::string temp;
  while (std::getline(ss, temp, separator)) {
    container.push_back(trim(temp));
  }
  return container;
}

struct QueryParameters final {
  QueryParameters() = default;

  explicit QueryParameters(
      std::vector<std::pair<QueryParameterKey, QueryParameterValue>> values)
      : values_(std::move(values)) {}

  [[nodiscard]] const std::vector<
      std::pair<QueryParameterKey, QueryParameterValue>> &
  values() const {
    return values_;
  }

  [[nodiscard]] std::string to_string() const {
    std::stringstream ss;
    if (!values_.empty()) {
      ss << "?" << values_[0].first << "=" << values_[0].second;
      for (uint64_t i = 1; i < values_.size(); ++i) {
        ss << "&" << values_[i].first << "=" << values_[i].second;
      }
    }
    return ss.str();
  }

private:
  std::vector<std::pair<QueryParameterKey, QueryParameterValue>> values_;
};

struct HttpResponseHeaders final {
  explicit HttpResponseHeaders(Headers headers)
      : headers_(std::move(headers)) {}
  explicit HttpResponseHeaders(const std::string &header_string)
      : headers_(parse(header_string)) {}

  bool operator==(const HttpResponseHeaders &rhs) const {
    return headers_ == rhs.headers_;
  }

  bool operator!=(const HttpResponseHeaders &rhs) const {
    return !(rhs == *this);
  }

  friend std::ostream &operator<<(std::ostream &os,
                                  const HttpResponseHeaders &headers) {
    const std::basic_string<char> &string = std::accumulate(
        headers.value().cbegin(), headers.value().cend(), std::string(),
        [](const std::string &s,
           const std::pair<const std::string, std::string> &v) {
          return s + (s.empty() ? std::string() : ",") + v.first + " : " +
                 v.second;
        });
    os << "[" << string << "]";
    return os;
  }

  const Headers &value() const { return headers_; }

private:
  Headers headers_;

  static Headers parse(const std::string &header_string) {
    std::stringstream ss(header_string);
    std::string container;
    Headers headers;
    while (std::getline(ss, container, '\n')) {
      std::size_t pos = container.find(':');
      if (pos != std::string::npos) {
        headers.emplace(trim(container.substr(0, pos)),
                        trim(container.substr(pos + 1)));
      }
    }
    return headers;
  }
};

struct HttpResponse final {
  HttpStatusCode status;
  HttpResponseHeaders headers;
  HttpResponseBody body;

  bool operator==(const HttpResponse &rhs) const {
    return status == rhs.status && headers == rhs.headers && body == rhs.body;
  }

  bool operator!=(const HttpResponse &rhs) const { return !(rhs == *this); }

  friend std::ostream &operator<<(std::ostream &os,
                                  const HttpResponse &response) {
    os << "status: " << response.status << " headers: " << response.headers
       << " body: " << response.body;
    return os;
  }
};

struct HttpSuccess final {
  explicit HttpSuccess(HttpResponse response) : value_(std::move(response)) {}

  bool operator==(const HttpSuccess &rhs) const { return value_ == rhs.value_; }

  bool operator!=(const HttpSuccess &rhs) const { return !(rhs == *this); }

  friend std::ostream &operator<<(std::ostream &os,
                                  const HttpSuccess &success) {
    os << success.value_;
    return os;
  }

  [[nodiscard]] const HttpResponse &value() const { return value_; }

  [[nodiscard]] const HttpStatusCode &status() const { return value_.status; }

  [[nodiscard]] const HttpResponseBody &body() const { return value_.body; }

  [[nodiscard]] const HttpResponseHeaders &headers() const {
    return value_.headers;
  }

private:
  HttpResponse value_;
};

struct HttpFailure final {
  explicit HttpFailure(HttpConnectionFailure value)
      : value_(std::move(value)) {}
  explicit HttpFailure(HttpResponse value) : value_(std::move(value)) {}

  bool operator==(const HttpFailure &rhs) const { return value_ == rhs.value_; }

  bool operator!=(const HttpFailure &rhs) const { return !(rhs == *this); }

  friend std::ostream &operator<<(std::ostream &os,
                                  const HttpFailure &failure) {
    failure.template match<void>(
        [&os](const HttpConnectionFailure &f) { os << f; },
        [&os](const HttpResponse &s) { os << s; });
    return os;
  }

  [[nodiscard]] const std::variant<HttpConnectionFailure, HttpResponse> &
  value() const {
    return value_;
  }

  template <class A>
  A match(
      const std::function<A(const HttpConnectionFailure &connectionFailure)>
          &cFn,
      const std::function<A(const HttpResponse &semanticFailure)> &sFn) const {
    return std::visit(
        visitor{[&cFn](const HttpConnectionFailure &c) { return cFn(c); },
                [&sFn](const HttpResponse &s) { return sFn(s); }},
        value_);
  }

private:
  std::variant<HttpConnectionFailure, HttpResponse> value_;
};

struct HttpResult final {
  explicit HttpResult(std::variant<HttpFailure, HttpSuccess> value)
      : value_(std::move(value)) {}

  bool operator==(const HttpResult &rhs) const { return value_ == rhs.value_; }

  bool operator!=(const HttpResult &rhs) const { return !(rhs == *this); }

  [[nodiscard]] const std::variant<HttpFailure, HttpSuccess> &value() const {
    return value_;
  }

  [[nodiscard]] std::optional<HttpFailure> failure() const {
    return std::holds_alternative<HttpFailure>(value_)
               ? std::get<HttpFailure>(value_)
               : std::optional<HttpFailure>{};
  }

  [[nodiscard]] std::optional<HttpSuccess> success() const {
    return std::holds_alternative<HttpSuccess>(value_)
               ? std::get<HttpSuccess>(value_)
               : std::optional<HttpSuccess>{};
  }

  template <class A>
  [[nodiscard]] A
  match(const std::function<A(const HttpFailure &)> failureFn,
        const std::function<A(const HttpSuccess &)> successFn) const {
    return std::visit(visitor{[&failureFn](const HttpFailure &failure) {
                                return failureFn(failure);
                              },
                              [&successFn](const HttpSuccess &success) {
                                return successFn(success);
                              }},
                      value_);
  }

private:
  std::variant<HttpFailure, HttpSuccess> value_;
};

struct PathSegments final {
  PathSegments() = default;
  PathSegments(PathSegments &path_segments) : value_(path_segments.value_) {}
  PathSegments(PathSegments &&path_segments) noexcept
      : value_(std::move(path_segments.value_)) {}
  explicit PathSegments(std::vector<PathSegment> value)
      : value_(std::move(value)) {}
  PathSegments &operator=(const PathSegments &other) {
    this->value_ = other.value_;
    return *this;
  }

  [[nodiscard]] const std::vector<PathSegment> &value() const { return value_; }

  [[nodiscard]] std::string to_string() const {
    std::stringstream ss;

    for (const auto &segment : value_) {
      ss << "/" << segment;
    }

    return ss.str();
  }

private:
  std::vector<PathSegment> value_;
};

struct HttpUrl final {
  HttpUrl() = default;

  HttpUrl(Protcol protocol, Host host, PathSegments path_segments = {},
          QueryParameters query_parameters = {})
      : protocol_(std::move(protocol)), host_(std::move(host)),
        path_segments_(std::move(path_segments)),
        query_parameters_(std::move(query_parameters)), value_(to_string()) {}

  explicit HttpUrl(std::string value)
      : protocol_(detect_protocol(value)), value_(std::move(value)) {}

  [[nodiscard]] const Protcol &protocol() const { return protocol_; }

  [[nodiscard]] std::string value() const {
    return value_.empty() ? to_string() : value_;
  }

  [[nodiscard]] HttpUrl &with_protocol(Protcol protocol) {
    protocol_ = std::move(protocol);
    return *this;
  }

  [[nodiscard]] HttpUrl &with_host(Host host) {
    host_ = std::move(host);
    return *this;
  }

  [[nodiscard]] HttpUrl &with_path_segments(const PathSegments &path_segments) {
    path_segments_ = path_segments;
    return *this;
  }

  [[nodiscard]] HttpUrl &
  with_query_parameters(QueryParameters query_parameters) {
    query_parameters_ = std::move(query_parameters);
    return *this;
  }

  [[nodiscard]] const PathSegments &path_segments() const {
    return path_segments_;
  }

private:
  Protcol protocol_;
  Host host_;
  PathSegments path_segments_;
  QueryParameters query_parameters_;
  std::string value_;

  static std::string detect_protocol(const std::string &url_string) {
    uint64_t i = url_string.find(':');
    if (i != std::string::npos) {
      return url_string.substr(0, i);
    } else {
      return "unknown";
    }
  }

  [[nodiscard]] std::string to_string() const {
    std::stringstream ss;
    ss << protocol_ << "://" << host_ << path_segments_.to_string()
       << query_parameters_.to_string();
    return trim(ss.str());
  }
};

template <class A> using Predicate = std::function<bool(const A &a)>;

template <class A> Predicate<A> eq(const A &a) {
  return [a](const A &other) { return a == other; };
}

template <class A> Predicate<A> between_inclusive(const A &a, const A &b) {
  return [a, b](const A &other) { return other >= a && other <= b; };
}

template <class A> Predicate<A> logical_or(const A &a, const A &b) {
  return [a, b](const A &other) { return other == a || other == b; };
}

// Unknown
inline static HttpStatusCode UNKNOWN = HttpStatusCode{0};

// Information Responses
inline static HttpStatusCode CONTINUE = HttpStatusCode{100};
inline static HttpStatusCode SWITCHING_PROTOCOL = HttpStatusCode{101};
inline static HttpStatusCode PROCESSING = HttpStatusCode{102};
inline static HttpStatusCode EARLY_HINTS = HttpStatusCode{103};

// Successful Responses
inline static HttpStatusCode OK = HttpStatusCode{200};
inline static HttpStatusCode CREATED = HttpStatusCode{201};
inline static HttpStatusCode ACCEPTED = HttpStatusCode{202};
inline static HttpStatusCode NON_AUTHORITATIVE_INFORMATION =
    HttpStatusCode{203};
inline static HttpStatusCode NO_CONTENT = HttpStatusCode{204};
inline static HttpStatusCode RESET_CONTENT = HttpStatusCode{205};
inline static HttpStatusCode PARTIAL_CONTENT = HttpStatusCode{206};
inline static HttpStatusCode MULTI_STATUS = HttpStatusCode{207};
inline static HttpStatusCode ALREADY_REPORTED = HttpStatusCode{208};
inline static HttpStatusCode IM_USED = HttpStatusCode{226};

// Redirection Messages
inline static HttpStatusCode MULTIPLE_CHOICE = HttpStatusCode{300};
inline static HttpStatusCode MOVED_PERMANENTLY = HttpStatusCode{301};
inline static HttpStatusCode FOUND = HttpStatusCode{302};
inline static HttpStatusCode SEE_OTHER = HttpStatusCode{303};
inline static HttpStatusCode NOT_MODIFIED = HttpStatusCode{304};
inline static HttpStatusCode USE_PROXY = HttpStatusCode{305};
inline static HttpStatusCode TEMPORARY_REDIRECT = HttpStatusCode{307};
inline static HttpStatusCode PERMANENT_REDIRECT = HttpStatusCode{308};

// Client Error Responses
inline static HttpStatusCode BAD_REQUEST = HttpStatusCode{400};
inline static HttpStatusCode UNAUTHORIZED = HttpStatusCode{401};
inline static HttpStatusCode PAYMENT_REQUIRED = HttpStatusCode{402};
inline static HttpStatusCode FORBIDDEN = HttpStatusCode{403};
inline static HttpStatusCode NOT_FOUND = HttpStatusCode{404};
inline static HttpStatusCode METHOD_NOT_ALLOWED = HttpStatusCode{405};
inline static HttpStatusCode NOT_ACCEPTABLE = HttpStatusCode{406};
inline static HttpStatusCode PROXY_AUTHENTICATION_REQUIRED =
    HttpStatusCode{407};
inline static HttpStatusCode REQUEST_TIMEOUT = HttpStatusCode{408};
inline static HttpStatusCode CONFLICT = HttpStatusCode{409};
inline static HttpStatusCode GONE = HttpStatusCode{410};
inline static HttpStatusCode LENGTH_REQUIRED = HttpStatusCode{411};
inline static HttpStatusCode PRECONDITION_FAILED = HttpStatusCode{412};
inline static HttpStatusCode PAYLOAD_TOO_LARGE = HttpStatusCode{413};
inline static HttpStatusCode URI_TOO_int64_t = HttpStatusCode{414};
inline static HttpStatusCode UNSUPPORTED_MEDIA_TYPE = HttpStatusCode{415};
inline static HttpStatusCode RANGE_NOT_SATISFIABLE = HttpStatusCode{416};
inline static HttpStatusCode EXPECTATION_FAILED = HttpStatusCode{417};
inline static HttpStatusCode IM_A_TEAPOT = HttpStatusCode{418};
inline static HttpStatusCode UNPROCESSABLE_ENTITY = HttpStatusCode{422};
inline static HttpStatusCode FAILED_DEPENDENCY = HttpStatusCode{424};
inline static HttpStatusCode TOO_EARLY = HttpStatusCode{425};
inline static HttpStatusCode UPGRADE_REQUIRED = HttpStatusCode{426};
inline static HttpStatusCode PRECONDITION_REQUIRED = HttpStatusCode{428};
inline static HttpStatusCode TOO_MANY_REQUESTS = HttpStatusCode{429};
inline static HttpStatusCode REQUEST_HEADER_FIELDS_TOO_LARGE =
    HttpStatusCode{431};
inline static HttpStatusCode UNAVAILABLE_FOR_LEGAL_REASONS =
    HttpStatusCode{451};

// Server Error Responses
inline static HttpStatusCode INTERNAL_SERVER_ERROR = HttpStatusCode{500};
inline static HttpStatusCode NOT_IMPLEMENTED = HttpStatusCode{501};
inline static HttpStatusCode BAD_GATEWAY = HttpStatusCode{502};
inline static HttpStatusCode SERVICE_UNAVAILABLE = HttpStatusCode{503};
inline static HttpStatusCode GATEWAY_TIMEOUT = HttpStatusCode{504};
inline static HttpStatusCode HTTP_VERSION_NOT_SUPPORTED = HttpStatusCode{505};
inline static HttpStatusCode VARIANT_ALSO_NEGOTIATES = HttpStatusCode{506};
inline static HttpStatusCode INSUFFICIENT_STORAGE = HttpStatusCode{507};
inline static HttpStatusCode LOOP_DETECTED = HttpStatusCode{508};
inline static HttpStatusCode NOT_EXTENDED = HttpStatusCode{510};
inline static HttpStatusCode NETWORK_AUTHENTICATION_REQUIRED =
    HttpStatusCode{511};

inline static Predicate<HttpStatusCode> informational() {
  return between_inclusive(CONTINUE, EARLY_HINTS);
}

inline static Predicate<HttpStatusCode> successful() {
  return between_inclusive(OK, IM_USED);
}

inline static Predicate<HttpStatusCode> redirect() {
  return between_inclusive(MULTIPLE_CHOICE, PERMANENT_REDIRECT);
}

inline static Predicate<HttpStatusCode> client_error() {
  return between_inclusive(BAD_REQUEST, UNAVAILABLE_FOR_LEGAL_REASONS);
}

inline static Predicate<HttpStatusCode> server_error() {
  return between_inclusive(INTERNAL_SERVER_ERROR,
                           NETWORK_AUTHENTICATION_REQUIRED);
}

struct Client final {
  Client() : debug_(false), verify_(true) {}

  Client &with_tls_verification(bool verify) {
    verify_ = verify;
    return *this;
  }

  Client &with_debug(bool debug) {
    debug_ = debug;
    return *this;
  }

  [[nodiscard]] HttpResult get(const HttpUrl &url,
                               const Headers &headers = {}) const {
    return get(url, eq(OK), headers);
  }

  [[nodiscard]] HttpResult
  get(const HttpUrl &url, const Predicate<HttpStatusCode> &successPredicate,
      const Headers &headers = {}) const {
    return execute(url, make_header_callback(headers), NoopCurlSetupCallback,
                   successPredicate);
  }

  [[nodiscard]] HttpResult post(const HttpUrl &url, const HttpRequestBody &body,
                                const Headers &headers = {}) const {
    return post(url, body, eq(OK), headers);
  }

  [[nodiscard]] HttpResult
  post(const HttpUrl &url, const HttpRequestBody &body,
       const Predicate<HttpStatusCode> &successPredicate,
       const Headers &headers = {}) const {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.value().c_str());
    };

    return execute(url, make_header_callback(headers), setup, successPredicate);
  }

  [[nodiscard]] HttpResult put(const HttpUrl &url, const HttpRequestBody &body,
                               const Headers &headers = {}) const {
    return put(url, body, eq(OK), headers);
  }

  [[nodiscard]] HttpResult
  put(const HttpUrl &url, const HttpRequestBody &body,
      const Predicate<HttpStatusCode> &successPredicate,
      const Headers &headers = {}) const {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.value().c_str());
    };

    return execute(url, make_header_callback(headers), setup, successPredicate);
  }

  [[nodiscard]] HttpResult del(const HttpUrl &url,
                               const Headers &headers = {}) const {
    return del(url, eq(OK), headers);
  }

  [[nodiscard]] HttpResult
  del(const HttpUrl &url, const Predicate<HttpStatusCode> &successPredicate,
      const Headers &headers = {}) const {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    };

    return execute(url, make_header_callback(headers), setup, successPredicate);
  }

  [[nodiscard]] HttpResult head(const HttpUrl &url) const {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    };

    return execute(url, NoopCurlHeaderCallback, setup, eq(OK));
  }

  [[nodiscard]] HttpResult options(const HttpUrl &url) const {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
    };

    return execute(url, NoopCurlHeaderCallback, setup, eq(OK));
  }

  [[nodiscard]] HttpResult trace(const HttpUrl &url) const {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "TRACE");
    };

    return execute(url, NoopCurlHeaderCallback, setup, eq(OK));
  }

  [[nodiscard]] HttpResult
  execute(const HttpUrl &url, const CurlHeaderCallback &curl_header_callback,
          const CurlSetupCallback &curl_setup_callback,
          const Predicate<HttpStatusCode> &successPredicate) const {
    CurlWrapper curlWrapper{successPredicate};

    curlWrapper.execute_header_callback(curl_header_callback);
    curlWrapper.add_option(CURLOPT_URL, url.value().c_str());
    curlWrapper.add_option(CURLOPT_VERBOSE, debug_ ? 1L : 0L);

    if (url.protocol().value() == "https") {
      verify_ ? curlWrapper.add_option(CURLOPT_SSL_VERIFYPEER, 1L)
              : curlWrapper.add_option(CURLOPT_SSL_VERIFYPEER, 0L);
    }

    curlWrapper.execute_setup_callback(curl_setup_callback);

    return curlWrapper.execute();
  }

private:
  bool debug_;
  bool verify_;

  static size_t write_callback(void *contents, size_t size, size_t nmemb,
                               void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb); // NOLINT
    return size * nmemb;
  }

  static CurlHeaderCallback make_header_callback(const Headers &headers) {
    return headers.empty()
               ? NoopCurlHeaderCallback
               : [&headers](curl_slist *chunk) {
                   for (const auto &[key, value] : headers) {
                     chunk =
                         curl_slist_append(chunk, (key + ": " + value).c_str());
                   }

                   return chunk;
                 };
  }

  struct CurlWrapper final {
    explicit CurlWrapper(const Predicate<HttpStatusCode> &success_predicate)
        : curl_(curl_easy_init()), slist_(nullptr),
          success_predicate_(success_predicate) {}

    ~CurlWrapper() {
      curl_easy_cleanup(curl_);
      curl_slist_free_all(slist_);
    }

    template <class A> void add_option(const CURLoption option, A value) {
      curl_easy_setopt(curl_, option, value);
    }

    void execute_header_callback(const CurlHeaderCallback &header_callback) {
      slist_ = header_callback(slist_);
    }

    void execute_setup_callback(const CurlSetupCallback &setup_callback) {
      setup_callback(curl_);
    }

    [[nodiscard]] HttpResult execute() {
      std::string body_buffer;
      std::string header_buffer;

      curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
      curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &body_buffer);
      curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, slist_);
      curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &header_buffer);

      CURLcode res = curl_easy_perform(curl_);
      if (res != CURLE_OK) {
        return HttpResult{
            HttpFailure{HttpConnectionFailure{curl_easy_strerror(res)}}};
      }

      int64_t status_code = 0;
      curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &status_code);

      HttpStatusCode status{status_code};
      HttpResponse httpResponse =
          HttpResponse{status, HttpResponseHeaders{header_buffer},
                       HttpResponseBody{body_buffer}};

      return success_predicate_(status) ? HttpResult{HttpSuccess{httpResponse}}
                                        : HttpResult{HttpFailure{httpResponse}};
    }

  private:
    CURL *curl_;
    curl_slist *slist_;
    Predicate<HttpStatusCode> success_predicate_;
  };
};
} // namespace SimpleHttp
