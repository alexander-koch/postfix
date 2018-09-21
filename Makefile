CC := clang++
CPPFLAGS := -std=c++14 -Wall
LDFLAGS := -lreadline

all:
	$(CC) $(CPPFLAGS) main.cpp -o main $(LDFLAGS)
