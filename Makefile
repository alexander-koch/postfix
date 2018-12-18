CC := clang++
CPPFLAGS := -std=c++14 -Wall -g
LDFLAGS := -lreadline
APP := pfix

SOURCES := src/types.cpp
OBJECTS := $(SOURCES:src/%.cpp=obj/%.o)

all: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(OBJECTS) src/main.cpp -o $(APP) $(LDFLAGS)

lib:
	$(CC) $(CPPFLAGS) -dynamiclib -flat_namespace example/example.cc $(OBJECTS) -o example.so

$(OBJECTS): obj/%.o: src/%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf obj/*.o