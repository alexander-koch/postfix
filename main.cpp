#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <algorithm>

#include <readline/readline.h>
#include <readline/history.h>

#include "types.hpp"

#define VERSION "v0.1.3"

using Stack = std::vector<std::unique_ptr<Obj>>;
using StackFunction = std::function<void(Stack* s)>;

class Simulator {
public:
    Stack stack;
    std::map<std::string, StackFunction> functions;

    void define_symbol(std::string sym, StackFunction sf) {
        functions.emplace(sym, sf);
    }

    void push(std::unique_ptr<Obj> obj) {
        if(obj->tag == T_SYM) {
            auto sym = dynamic_cast<Sym*>(obj.get())->str;
            if(sym[0] == ':' || sym[sym.size()-1] == ':' || sym == "[") {
                stack.push_back(std::move(obj)); 
            } else if(sym == "]") {
                std::vector<std::unique_ptr<Obj>> arr;
                while(stack.size() > 0
                        && !(stack.back()->tag == T_SYM
                        && dynamic_cast<Sym*>(stack.back().get())->str == "[")) {
                        
                    arr.push_back(std::move(stack.back()));
                    stack.pop_back();
                }

                if(stack.size() > 0
                        && stack.back()->tag == T_SYM
                        && dynamic_cast<Sym*>(stack.back().get())->str == "[") {
                    
                    std::reverse(arr.begin(), arr.end());
                    stack.push_back(std::make_unique<Arr>(std::move(arr)));
                } else {
                    // TODO error
                }
            } else {
                auto iter = functions.find(sym);
                if(iter != functions.end()) {
                    iter->second(&stack);
                }
            }
        } else if(obj->tag == T_EXE_ARR) {
            // TODO
        } else {
            stack.push_back(std::move(obj));
        }
    }

    friend std::ostream& operator<<(std::ostream& os, Simulator& sim) {
        bool comma = false;
        os << "[";
        std::for_each(sim.stack.begin(), sim.stack.end(), [&os, &comma](auto& x) {
            if(comma) os << ", ";
            x->print(os);
            comma = true;
        });
        os << "]";
        return os;
    }
};

void add_int(Stack* s, int a, int b) {
    s->push_back(std::make_unique<Int>(a + b));
}

void add_flt(Stack* s, double a, double b) {
    s->push_back(std::make_unique<Int>(a + b));
}

void sub_int(Stack* s, int a, int b) {
    s->push_back(std::make_unique<Int>(a - b));
}

void sub_flt(Stack* s, double a, double b) {
    s->push_back(std::make_unique<Int>(a - b));
}

void mul_int(Stack* s, int a, int b) {
    s->push_back(std::make_unique<Int>(a + b));
}

void mul_flt(Stack* s, double a, double b) {
    s->push_back(std::make_unique<Int>(a * b));
}

void div_int(Stack* s, int a, int b) {
    s->push_back(std::make_unique<Flt>(a / static_cast<float>(b)));
}

void div_flt(Stack* s, double a, double b) {
    s->push_back(std::make_unique<Flt>(a / b));
}

void idiv(Stack* s, int a, int b) {
    s->push_back(std::make_unique<Int>(a / b));
}

void mod(Stack* s, int a, int b) {
    s->push_back(std::make_unique<Int>(a % b));
}

std::string type_to_string(TypeTag tag) {
    switch(tag) {
        case T_OBJ: return":Obj";
        case T_BOOL: return ":Bool";
        case T_INT: return ":Int";
        case T_FLT: return ":Flt";
        case T_STR: return ":Str";
        case T_ARR: return ":Arr";
        case T_EXE_ARR: return ":ExeArr";
        case T_PARAMS: return ":Params";
        case T_SYM: return ":Sym";
    }
}

void print_top(Stack* s) {
    if(s->size() > 0) {
        s->back()->print(std::cout);
        s->pop_back();
    }
}

void int_to_flt(Stack* s, std::unique_ptr<Obj> x) {
    s->push_back(std::make_unique<Flt>(dynamic_cast<Int*>(x.get())->i));
}

void type_to_symbol(Stack* s, std::unique_ptr<Obj> x) {
    s->push_back(std::make_unique<Sym>(type_to_string(x->tag)));
}

using UnaryFunc = std::function<void(Stack*, std::unique_ptr<Obj>)>;

void unary_op(Stack* s, UnaryFunc op) {
    if(s->size() == 0) return;
    auto x = std::move(s->back());
    s->pop_back();
    op(s, std::move(x));
}

using BinaryIntOp = std::function<void(Stack*, int, int)>;
using BinaryFltOp = std::function<void(Stack*, double, double)>;

void binary_arith_op(Stack* s, BinaryIntOp int_op, BinaryFltOp flt_op) {
    if(s->size() < 2) return;
    auto x2 = std::move(s->back());
    s->pop_back();
    auto x1 = std::move(s->back());
    s->pop_back();

    if(x1->tag == T_INT && x2->tag == T_INT) {
        int_op(s, dynamic_cast<Int*>(x1.get())->i, dynamic_cast<Int*>(x2.get())->i);
    } else if(x1->tag == T_FLT && x2->tag == T_FLT) {
        flt_op(s, dynamic_cast<Flt*>(x1.get())->f, dynamic_cast<Flt*>(x2.get())->f);
    } else if(x1->tag == T_INT && x2->tag == T_FLT) {
        flt_op(s, dynamic_cast<Int*>(x1.get())->i, dynamic_cast<Flt*>(x2.get())->f);
    } else if(x1->tag == T_FLT && x2->tag == T_INT) {
        flt_op(s, dynamic_cast<Flt*>(x1.get())->f, dynamic_cast<Int*>(x2.get())->i);
    }
}

void flt_undefined(Stack* s, double, double) {
    s->push_back(std::make_unique<Sym>("Err: Operation not defined for :Flt x :Flt"));
}

bool is_double(std::string& s) {
    try {
        std::stod(s);
    } catch(...) {
        return false;
    }
    return true;
}

bool is_integer(std::string& s) {
    return std::find_if(s.begin(), s.end(), [](char c) {
        return !std::isdigit(c);
    }) == s.end();
}

bool is_punct(char c) {
    switch(c) {
        case '{':
        case '}':
        case '[':
        case ']': return true;
        default: return false;
    }
}

enum TokenType {
    TOK_STR,
    TOK_SYM,
    TOK_EOF
};

TokenType scan(std::istringstream& iss, std::string& buffer) {
    char c;
    iss >> std::ws;
    buffer.clear();
    if(!iss.get(c)) return TokenType::TOK_EOF;
    if(c == '\"') {
        while(iss.get(c) && c != '\"') {
            buffer += c;
        }
        return TOK_STR;
    } else if(is_punct(c)) {
        buffer = std::string(1, c);
    } else {
        buffer += c;
        while(iss.get(c) && !std::isspace(c) && !is_punct(c)) {
            buffer += c;
        }
        iss.putback(c);
    }
    return TOK_SYM;
}

int main() {
    bool running = true;
    auto sim = Simulator();
    sim.functions = {
        {{"+"}, [](Stack* s) { binary_arith_op(s, add_int, add_flt); }},
        {{"-"}, [](Stack* s) { binary_arith_op(s, sub_int, sub_flt); }},
        {{"*"}, [](Stack* s) { binary_arith_op(s, mul_int, mul_flt); }},
        {{"/"}, [](Stack* s) { binary_arith_op(s, div_int, div_flt); }},
        {{"i/"}, [](Stack* s) { binary_arith_op(s, idiv, flt_undefined); }},
        {{"mod"}, [](Stack* s) { binary_arith_op(s, mod, flt_undefined); }},
        {{"int->flt"}, [](Stack* s) { unary_op(s, int_to_flt); }},
        {{"print"}, [](Stack* s) { print_top(s); }},
        {{"println"}, [](Stack* s) { print_top(s); std::cout << std::endl; }},
        {{"clear"}, [](Stack* s) { s->clear(); }},
        {{"type"}, [](Stack* s) { unary_op(s, type_to_symbol); }},
        {{"exit"}, [&running](Stack*) { running = false; }}
    };

    std::cout << "Postfux - " << VERSION << std::endl;

    while(running) {
        if(sim.stack.size() > 0) {
            sim.stack.back()->print(std::cout) << std::endl;
        }

        // Fetch user input
        char* buf = readline(">>> ");
        std::string input(buf);
        free(buf);
        if(input.size() > 0) add_history(input.c_str());

        // Split into tokens
        std::istringstream ss(input);
        TokenType tok;
        std::string token;
        while((tok = scan(ss, token)) != TOK_EOF) {
            if(tok == TOK_STR) {
                sim.push(std::make_unique<Str>(token));
            } else if(tok == TOK_SYM) {
                if(is_integer(token)) {
                    sim.push(std::make_unique<Int>(std::stoi(token)));
                } else if(is_double(token)) {
                    sim.push(std::make_unique<Flt>(std::stod(token)));
                } else {
                    sim.push(std::make_unique<Sym>(token));
                }
            }
        }
    }

    return 0;
}