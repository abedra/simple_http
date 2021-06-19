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
  HttpStatusCode rawStatusCode;
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

HttpStatusCode HTTP_OK = HttpStatusCode{200};

struct Client {
  Client() : error_callback_(NoopErrorCallback), debug_(false) {}
  explicit Client(bool debug) : error_callback_(NoopErrorCallback), debug_(debug) {}
  explicit Client(const ErrorCallback error_callback) : error_callback_(error_callback), debug_(false) {}
  explicit Client(const ErrorCallback error_callback, bool debug) : error_callback_(error_callback), debug_(debug) {}

  [[nodiscard]]
  std::optional<HttpResponse> get(const Url &url,
                                  const Headers &headers = {}) {
    return execute(url, make_header_callback(headers), NoopCurlSetupCallback, eq(HTTP_OK));
  }

  [[nodiscard]]
  std::optional<HttpResponse> post(const Url &url, 
                                   const HttpRequestBody &body,
                                   const Headers &headers = {}) {
    CurlSetupCallback setup = [&](CURL *curl) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.value().c_str());
    };

    return execute(url, make_header_callback(headers), setup, eq(HTTP_OK));
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
