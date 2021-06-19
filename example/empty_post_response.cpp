#include <iostream>
#include <assert.h>
#include "../simple_http.hpp"

int main(void) {
    SimpleHttp::Client client(true);

    auto maybeResponse = client.post(
        SimpleHttp::Url{"http://localhost:5000/empty_post_response"},
        SimpleHttp::HttpRequestBody{""},
        SimpleHttp::eq(SimpleHttp::NO_CONTENT),
        {{"Content-Type", "application/json"}});

    if (maybeResponse) {
        SimpleHttp::HttpResponse response = maybeResponse.value();
        assert(response.body.value().empty());
    } else {
        std::cout << "request failed" << std::endl;
    }
}