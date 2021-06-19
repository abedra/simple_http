#include <iostream>

#include "../simple_http.hpp"
#include "json.hpp"

int main(void) {
  SimpleHttp::Client client;

  auto maybeResponse =
      client.post(SimpleHttp::Url{"http://localhost:5000/post"},
                  SimpleHttp::HttpRequestBody{"{\"name\":\"test\"}"},
                  {{"Content-Type", "application/json"}});

  if (maybeResponse) {
    SimpleHttp::HttpResponse response = maybeResponse.value();
    auto keys = nlohmann::json::parse(response.body.value());
    std::cout << keys["hello"] << std::endl;
  } else {
    std::cout << "request failed" << std::endl;
  }
}