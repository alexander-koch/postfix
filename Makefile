CC := clang++
CPPFLAGS := -std=c++14 -Wall -g
LDFLAGS := -lreadline
EXE := pfix

all:
	$(CC) $(CPPFLAGS) src/main.cpp -o $(EXE) $(LDFLAGS)
