#ifndef __PFIX_INTERPRETER_HPP__
#define __PFIX_INTERPRETER_HPP__

#include "types.hpp"

#include <string>
#include <dlfcn.h>

void sanitize_symbol(std::string& sym);

class PfixInterpreter {
private:
    bool evaluate_on_push = true;
    int exe_arr = 0;
    int exe_begin;

    void evaluate_dictionary(std::string& sym);
    void evaluate_symbol(std::string& sym);

public:
    PfixStack stack;
    PfixDictionary dictionary;

    void load_builtins();

    void push(std::unique_ptr<Obj> obj);
    friend std::ostream& operator<<(std::ostream& os, PfixInterpreter& interp);
};

#endif