CXX_FLAGS := -Wall -Werror -fconcepts -std=c++2a
LD_FLAGS := -lcurl

default: all

get.o: get.cpp
	g++ $(CXX_FLAGS) -c get.cpp

get: get.o
	g++ get.o -o get $(LD_FLAGS)

connection_error.o: connection_error.cpp
	g++ $(CXX_FLAGS) -c connection_error.cpp

connection_error: connection_error.o
	g++ connection_error.o -o connection_error $(LD_FLAGS)

post.o: post.cpp
	g++ $(CXX_FLAGS) -c post.cpp

post: post.o
	g++ post.o -o post $(LD_FLAGS)

empty_post_response.o: empty_post_response.cpp
	g++ $(CXX_FLAGS) -c empty_post_response.cpp

empty_post_response: empty_post_response.o
	g++ empty_post_response.o -o empty_post_response $(LD_FLAGS)

get_not_allowed.o: get_not_allowed.cpp
	g++ $(CXX_FLAGS) -c get_not_allowed.cpp

get_not_allowed: get_not_allowed.o
	g++ get_not_allowed.o -o get_not_allowed $(LD_FLAGS)

all: get connection_error post empty_post_response get_not_allowed

.PHONY: clean
clean:
	rm -f *.o get connection_error post empty_post_response get_not_allowed