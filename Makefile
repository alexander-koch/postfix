CC := clang++
CPPFLAGS := -std=c++14 -Wall -g
LDFLAGS := -lreadline
APP := pfix

all: lib
	$(CC) $(CPPFLAGS) src/main.cpp -o $(APP) $(LDFLAGS)

lib:
	$(CC) $(CPPFLAGS) -dynamiclib -flat_namespace example/example.cc -o example.so