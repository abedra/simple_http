default: all

test/integration_tests.o: simple_http.hpp test/integration_tests.cpp
	g++ -Wall -Werror -std=c++17 -c test/integration_tests.cpp -o test/integration_tests.o

integration_tests: test/integration_tests.o
	g++ test/integration_tests.o -o integration_tests -lcurl

.PHONY: integration_tests
all: integration_tests
	./integration_tests

.PHONY: clean
clean:
	rm -f test/*.o integration_tests