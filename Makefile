CC := clang++
CPPFLAGS := -std=c++14 -Wall -g
LDFLAGS := -lreadline
APP := pfix

all:
	$(CC) $(CPPFLAGS) src/main.cpp -o $(APP) $(LDFLAGS)
