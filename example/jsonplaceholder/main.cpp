#include <iostream>
#include <nlohmann/json.hpp>
#include <simple_http.hpp>

struct Failure final {
  std::string url;
  std::string value;
};

struct Id final {
  int value;
};

int main() {
  SimpleHttp::Client client;
  SimpleHttp::HttpUrl url{"https://jsonplaceholder.typicode.com/users"};
  auto result = client.get(url).template match<std::variant<Failure, Id>>(
      [&url](const SimpleHttp::HttpFailure &failure){
        return failure.template match<std::variant<Failure, Id>>(
            [&url](const SimpleHttp::HttpConnectionFailure &connectionFailure) {
              return Failure{url.value(),connectionFailure.value()};
            },
            [&url](const SimpleHttp::HttpResponse &semanticFailure) {
              return Failure{url.value(), semanticFailure.body.value()};
            }
        );
      },
      [](const SimpleHttp::HttpSuccess &success){
        auto parsed = nlohmann::json::parse(success.body().value());
        return Id{parsed[0]["id"]};
      }
  );

  if (std::holds_alternative<Failure>(result)) {
    Failure failure = std::get<Failure>(result);
    std::cout << "Failed HTTP request for <" << failure.url << ">: " << failure.value << std::endl;
  } else {
    std::cout << "Result: " << std::get<Id>(result).value << std::endl;
  }
}
