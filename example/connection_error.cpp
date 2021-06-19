#include <iostream>

#include "../simple_http.hpp"

int main(void) {
  SimpleHttp::Client client{
      [](auto &err) { std::cout << "Error: " << err << std::endl; }
  };

  auto maybeResponse = client.get(SimpleHttp::Url{"error"});
}