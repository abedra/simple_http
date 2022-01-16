#include <iostream>
#include "nlohmann/json.hpp"
#include "simple_http.hpp"

int main() {
  SimpleHttp::Client client;
  SimpleHttp::HttpUrl url{"https://jsonplaceholder.typicode.com/users"};
  client.get(url).template match<void>(
      [&url](const SimpleHttp::HttpFailure &failure){
        std::cout << "Failed GET for: " << url.value() << std::endl;
      },
      [](const SimpleHttp::HttpSuccess &success){
        auto parsed = nlohmann::json::parse(success.body().value());
        std::cout << parsed[0]["id"] << std::endl;
      }
  );
}
