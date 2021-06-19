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

empty_post_response.o: empty_post_response.cpp
	g++ -Wall -Werror -fconcepts -std=c++2a -c empty_post_response.cpp

empty_post_response: empty_post_response.o
	g++ empty_post_response.o -o empty_post_response -lcurl

all: get connection_error post empty_post_response

.PHONY: clean
clean:
	rm -f *.o get connection_error post