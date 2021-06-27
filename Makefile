default: test

test/integration_tests.o: test/integration_tests.cpp
	g++ -Wall -Werror -fconcepts -std=c++2a -c test/integration_tests.cpp -o test/integration_tests.o

integration_tests: test/integration_tests.o
	g++ test/integration_tests.o -o integration_tests -lcurl

.PHONY: integration_tests
test: integration_tests
	./integration_tests

.PHONY: clean
clean:
	rm -f test/*.o integration_tests