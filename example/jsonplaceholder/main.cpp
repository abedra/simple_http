#include <iostream>
#include "nlohmann/json.hpp"
#include "simple_http.hpp"

int main() {
  SimpleHttp::Client client;
  SimpleHttp::HttpUrl url{"https://jsonplaceholder.typicode.com/users"};
  const std::optional<SimpleHttp::HttpResponse> &maybeResponse = client.get(url);

  if (maybeResponse) {
    auto parsed = nlohmann::json::parse(maybeResponse.value().body.value());
    std::cout << parsed[0]["id"] << std::endl;
  } else {
    std::cout << "Failed GET for: " << url.value() << std::endl;
  }
}
