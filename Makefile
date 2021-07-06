default: all

test/unit_tests.o: simple_http.hpp test/unit_tests.cpp
	g++ -Wall -Werror -Wno-unused-function -std=c++17 -c test/unit_tests.cpp -o test/unit_tests.o

unit_tests: test/unit_tests.o
	g++ test/unit_tests.o -o unit_tests -lcurl

test/integration_tests.o: simple_http.hpp test/integration_tests.cpp
	g++ -Wall -Werror -std=c++17 -c test/integration_tests.cpp -o test/integration_tests.o

integration_tests: test/integration_tests.o
	g++ test/integration_tests.o -o integration_tests -lcurl

tests: test/unit_tests.o test/integration_tests.o
	g++ test/unit_tests.o test/integration_tests.o -o tests -lcurl

.PHONY: all
all: tests
	./tests

.PHONY: clean
clean:
	rm -f test/*.o integration_tests