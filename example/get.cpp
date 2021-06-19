#include <iostream>

#include "../simple_http.hpp"
#include "json.hpp"

int main(void) {
  SimpleHttp::Client client;

  auto maybeResponse = client.get(SimpleHttp::Url{"http://localhost:5000/get"});

  if (maybeResponse) {
    SimpleHttp::HttpResponse response = maybeResponse.value();
    auto keys = nlohmann::json::parse(response.body.value());
    std::cout << keys["get"] << std::endl;
  }
}