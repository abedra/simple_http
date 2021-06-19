default: all

get.o: get.cpp
	g++ -Wall -Werror -fconcepts -std=c++2a -c get.cpp

get: get.o
	g++ get.o -o get -lcurl

connection_error.o: connection_error.cpp
	g++ -Wall -Werror -fconcepts -std=c++2a -c connection_error.cpp

connection_error: connection_error.o
	g++ connection_error.o -o connection_error -lcurl

post.o: post.cpp
	g++ -Wall -Werror -fconcepts -std=c++2a -c post.cpp

post: post.o
	g++ post.o -o post -lcurl

all: get connection_error post

.PHONY: clean
clean:
	rm -f *.o get connection_error post