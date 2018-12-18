CC := clang++
CPPFLAGS := -std=c++14 -Wall -g
LDFLAGS := -lreadline
APP := pfix

OBJDIR = obj

SOURCES := src/types.cpp src/lexer.cpp src/interpreter.cpp
OBJECTS := $(SOURCES:src/%.cpp=$(OBJDIR)/%.o)

all: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(OBJECTS) src/main.cpp -o $(APP) $(LDFLAGS)

lib:
	$(CC) $(CPPFLAGS) -dynamiclib -flat_namespace example/example.cc $(OBJECTS) -o example.so

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJECTS): $(OBJDIR)

$(OBJDIR)/%.o: src/%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)