#include <iostream>
#include <assert.h>
#include "../simple_http.hpp"

int main(void) {
    SimpleHttp::Client client(true);

    auto maybeResponse = client.get(
        SimpleHttp::Url{"http://localhost:5000/empty_post_response"},
        SimpleHttp::eq(SimpleHttp::METHOD_NOT_ALLOWED));

    if (maybeResponse) {
        SimpleHttp::HttpResponse response = maybeResponse.value();
        assert(response.status == SimpleHttp::METHOD_NOT_ALLOWED);
    } else {
        std::cout << "request failed" << std::endl;
    }
}