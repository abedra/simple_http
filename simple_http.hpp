#include <curl/curl.h>
#include <string>
#include <sstream>
#include <optional>
#include <functional>

namespace SimpleHttp {

template<typename Name, typename A>
struct Tiny {
    Tiny() noexcept(std::is_nothrow_constructible_v<A>) = default;
    explicit Tiny(const A &value) noexcept(std::is_nothrow_copy_constructible_v<A>) : value_(value) {}
    explicit Tiny(A &&value) noexcept(std::is_nothrow_move_constructible_v<A>) : value_(std::move(value)) {}

    friend std::ostream &operator<<(std::ostream &os, const Tiny &object) {
      return os << object.value();
    }

    friend bool operator==(const Tiny& lhs, const Tiny &rhs) {
      return lhs.value() == rhs.value();
    }

    explicit operator A& () const noexcept {
      return value();
    }

    [[nodiscard]] 
    const A& value() const noexcept {
      return value_;
    }

    [[nodiscard]]
    const std::string toString() const {
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

SIMPLE_HTTP_TINY_STRING(Url)
SIMPLE_HTTP_TINY_STRING(HttpResponseBody)
SIMPLE_HTTP_TINY_STRING(HttpRequestBody)

SIMPLE_HTTP_TINY_LONG(HttpStatusCode)

#undef SIMPLE_HTTP_TINY_STRING
#undef SIMPLE_HTTP_TINY_LONG

struct HttpResponse {
  HttpResponseBody body;
  HttpStatusCode status;
};

using CurlSetupCallback = std::function<void(CURL *curl)>;
CurlSetupCallback NoopCurlSetupCallback = [](auto){};

using CurlHeaderCallback = std::function<curl_slist*(curl_slist *chunk)>;
CurlHeaderCallback NoopCurlHeaderCallback = [](curl_slist *chunk){ return chunk; };

using ErrorCallback = std::function<void(const std::string&)>;
ErrorCallback NoopErrorCallback = [](auto&){};

using Headers = std::unordered_map<std::string, std::string>;

template<class Fn, class A, class B>
concept Fn1 = requires(Fn fn, A a) {
  { fn(a) } -> B;
};

template<class Fn, class A>
concept Predicate = Fn1<Fn, A, bool>;

template<class A>
Predicate<A> eq(const A &a) {
  return [a](const A &other) { return a == other; };
}

// Information Responses
HttpStatusCode CONTINUE = HttpStatusCode{100};
HttpStatusCode SWITCHING_PROTOCOL = HttpStatusCode{101};
HttpStatusCode PROCESSING = HttpStatusCode{102};
HttpStatusCode EARLY_HINTS = HttpStatusCode{103};

// Successful Responses
HttpStatusCode OK = HttpStatusCode{200};
HttpStatusCode CREATED = HttpStatusCode{201};
HttpStatusCode ACCEPTED = HttpStatusCode{202};
HttpStatusCode NON_AUTHORITATIVE_INFORMATION = HttpStatusCode{203};
HttpStatusCode NO_CONTENT = HttpStatusCode{204};
HttpStatusCode RESET_CONTENT = HttpStatusCode{205};
HttpStatusCode PARTIAL_CONTENT = HttpStatusCode{206};
HttpStatusCode MULTI_STATUS = HttpStatusCode{207};
HttpStatusCode ALREADY_REPORTED = HttpStatusCode{208};
HttpStatusCode IM_USED = HttpStatusCode{226};

// Redirection Messages
HttpStatusCode MULTIPLE_CHOICE = HttpStatusCode{300};
HttpStatusCode MOVED_PERMANENTLY = HttpStatusCode{301};
HttpStatusCode FOUND = HttpStatusCode{302};
HttpStatusCode SEE_OTHER = HttpStatusCode{303};
HttpStatusCode NOT_MODIFIED = HttpStatusCode{304};
HttpStatusCode USE_PROXY = HttpStatusCode{305};
HttpStatusCode TEMPORARY_REDIRECT = HttpStatusCode{307};
HttpStatusCode PERMANENT_REDIRECT = HttpStatusCode{308};

// Client Error Responses
HttpStatusCode BAD_REQUEST = HttpStatusCode{400};
HttpStatusCode UNAUTHORIZED = HttpStatusCode{401};
HttpStatusCode PAYMENT_REQUIRED = HttpStatusCode{402};
HttpStatusCode FORBIDDEN = HttpStatusCode{403};
HttpStatusCode NOT_FOUND = HttpStatusCode{404};
HttpStatusCode METHOD_NOT_ALLOWED = HttpStatusCode{405};
HttpStatusCode NOT_ACCEPTABLE = HttpStatusCode{406};
HttpStatusCode PROXY_AUTHENTICATION_REQUIRED = HttpStatusCode{407};
HttpStatusCode REQUEST_TIMEOUT = HttpStatusCode{408};
HttpStatusCode CONFLICT = HttpStatusCode{409};
HttpStatusCode GONE = HttpStatusCode{410};
HttpStatusCode LENGTH_REQUIRED = HttpStatusCode{411};
HttpStatusCode PRECONDITION_FAILED = HttpStatusCode{412};
HttpStatusCode PAYLOAD_TOO_LARGE = HttpStatusCode{413};
HttpStatusCode URI_TOO_LONG = HttpStatusCode{414};
HttpStatusCode UNSUPPORTED_MEDIA_TYPE = HttpStatusCode{415};
HttpStatusCode RANGE_NOT_SATISFIABLE = HttpStatusCode{416};
HttpStatusCode EXPECTATION_FAILED = HttpStatusCode{417};
HttpStatusCode IM_A_TEAPOT = HttpStatusCode{418};
HttpStatusCode UNPROCESSABLE_ENTITY = HttpStatusCode{422};
HttpStatusCode FAILED_DEPENDENCY = HttpStatusCode{424};
HttpStatusCode TOO_EARLY = HttpStatusCode{425};
HttpStatusCode UPGRADE_REQUIRED = HttpStatusCode{426};
HttpStatusCode PRECONDITION_REQUIRED = HttpStatusCode{428};
HttpStatusCode TOO_MANY_REQUESTS = HttpStatusCode{429};
HttpStatusCode REQUEST_HEADER_FIELDS_TOO_LARGE = HttpStatusCode{431};
HttpStatusCode UNAVAILABLE_FOR_LEGAL_REASONS = HttpStatusCode{451};

// Server Error Responses
HttpStatusCode INTERNAL_SERVER_ERROR = HttpStatusCode{500};
HttpStatusCode NOT_IMPLEMENTED = HttpStatusCode{501};
HttpStatusCode BAD_GATEWAY = HttpStatusCode{502};
HttpStatusCode SERVICE_UNAVAILABLE = HttpStatusCode{503};
HttpStatusCode GATEWAY_TIMEOUT = HttpStatusCode{504};
HttpStatusCode HTTP_VERSION_NOT_SUPPORTED = HttpStatusCode{505};
HttpStatusCode VARIANT_ALSO_NEGOTIATES = HttpStatusCode{506};
HttpStatusCode INSUFFICIENT_STORAGE = HttpStatusCode{507};
HttpStatusCode LOOP_DETECTED = HttpStatusCode{508};
HttpStatusCode NOT_EXTENDED = HttpStatusCode{510};
HttpStatusCode NETWORK_AUTHENTICATION_REQUIRED = HttpStatusCode{511};

struct Client {
  Client() : error_callback_(NoopErrorCallback), debug_(false) {}
  explicit Client(bool debug) : error_callback_(NoopErrorCallback), debug_(debug) {}
  explicit Client(const ErrorCallback error_callback) : error_callback_(error_callback), debug_(false) {}
  explicit Client(const ErrorCallback error_callback, bool debug) : error_callback_(error_callback), debug_(debug) {}

  [[nodiscard]]
  std::optional<HttpResponse> get(const Url &url,
                                  const Headers &headers = {}) {
    return get(url, eq(OK), headers);
  }

  [[nodiscard]]
  std::optional<HttpResponse> get(const Url &url,
                                  const Predicate<HttpStatusCode> &sucessPredicate,
                                  const Headers &headers = {}) {
    return execute(url, make_header_callback(headers), NoopCurlSetupCallback, sucessPredicate);
  }

  [[nodiscard]]
  std::optional<HttpResponse> post(const Url &url, 
                                   const HttpRequestBody &body,
                                   const Headers &headers = {}) {
    return post(url, body, eq(OK), headers);
  }

  [[nodiscard]]
  std::optional<HttpResponse> post(const Url &url, 
                                   const HttpRequestBody &body,
                                   const Predicate<HttpStatusCode> &successPredicate,
                                   const Headers &headers = {}) {
    CurlSetupCallback setup = [&](CURL *curl) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.value().c_str());
    };

    return execute(url, make_header_callback(headers), setup, successPredicate);
  }

  [[nodiscard]]
  std::optional<HttpResponse> put(const Url &url,
                                  const HttpRequestBody &body,
                                  const Headers &headers = {}) {
    return put(url, body, eq(OK), headers);
  }

  [[nodiscard]]
  std::optional<HttpResponse> put(const Url &url,
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
  std::optional<HttpResponse> del(const Url &url,
                                  const Headers &headers = {}) {
    return del(url, eq(OK), headers);
  }

  [[nodiscard]]
  std::optional<HttpResponse> del(const Url &url,
                                  const Predicate<HttpStatusCode> &successPredicate,
                                  const Headers &headers = {}) {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    };
    
    return execute(url, make_header_callback(headers), setup, successPredicate);
  }

  [[nodiscard]]
  std::optional<HttpResponse> head(const Url &url) {
    CurlSetupCallback setup = [&](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    };

    return execute(url, NoopCurlHeaderCallback, setup, eq(OK));
  }

  [[nodiscard]]
  std::optional<HttpResponse> execute(const Url &url,
                                      const CurlHeaderCallback &curl_header_callback,
                                      const CurlSetupCallback &curl_setup_callback,
                                      const Predicate<HttpStatusCode> &successPredicate) {
      std::string buffer;
      long rawStatusCode = 0;

      CURL *curl = curl_easy_init();
      if (curl) {
          struct curl_slist *chunk = nullptr;

          curl_easy_setopt(curl, CURLOPT_URL, url.value().c_str());
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);          
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_header_callback(chunk));
          curl_easy_setopt(curl, CURLOPT_VERBOSE, debug_);
          curl_setup_callback(curl);

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
        return std::optional<HttpResponse>({HttpResponseBody{buffer}, status});
      } else {
        return std::nullopt;
      }
  }

private:
  ErrorCallback error_callback_;
  bool debug_;

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
