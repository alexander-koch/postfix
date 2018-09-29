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
#include "lexer.hpp"

#define VERSION "v0.1.6"

using Stack = std::vector<std::unique_ptr<Obj>>;
using StackFunction = std::function<void(Stack* s)>;

class Simulator {
private:
    bool evaluate_on_push = true;
    int exe_arr = 0;
    int exe_begin;

public:
    Stack stack;
    std::map<std::string, StackFunction> functions;
    Dictionary dictionary;

    void define_symbol(std::string sym, StackFunction sf) {
        functions.emplace(sym, sf);
    }

    void push(std::unique_ptr<Obj> obj) {
        if(obj->tag == TypeTag::SYM) {
            auto sym = dynamic_cast<Sym*>(obj.get())->str;
            if(sym[0] == ':' || sym[sym.size()-1] == ':' || sym == "[") {
                stack.push_back(std::move(obj));
            } else if(sym == "{") {
                stack.push_back(std::move(obj));

                evaluate_on_push = false;
                if(exe_arr == 0) exe_begin = stack.size();
                exe_arr++;
            } else if(sym == "}") {
                exe_arr--;
                if(exe_arr < 0) {
                    // TODO throw an error

                } else if(exe_arr == 0) {
                    evaluate_on_push = true;

                    // Move range from stack into array
                    std::vector<std::unique_ptr<Obj>> arr;
                    auto start = std::next(stack.begin(), exe_begin);
                    std::move(start, stack.end(), std::back_inserter(arr));
                    stack.erase(std::prev(start), stack.end());

                    // Push newly created executable array onto the stack
                    stack.push_back(std::make_unique<ExeArr>(std::move(arr)));
                } else {
                    stack.push_back(std::move(obj));
                }
            } else {
                if(evaluate_on_push) {
                    if(sym[sym.size()-1] == '!') {
                        sym.pop_back();
                        if(stack.size() > 0) {
                            dictionary[sym] = std::move(stack.back());
                            stack.pop_back();
                        } else {
                            // TODO error handling
                        }
                    } else {
                        auto iter = functions.find(sym);
                        if(iter != functions.end()) {
                            iter->second(&stack);
                        } else {
                            auto iter = dictionary.find(sym);
                            if(iter != dictionary.end()) {
                                auto& obj = iter->second;
                                if(obj->tag == TypeTag::EXE_ARR) {
                                    auto old_dict = std::move(dictionary);

                                    auto exe_arr = dynamic_cast<ExeArr*>(obj.get());
                                    dictionary = exe_arr->dictionary.copy();

                                    // dictionary.print(std::cout);

                                    for(auto& x : exe_arr->vec) {
                                        push(x->copy());
                                    }

                                    // exe_arr->dictionary = std::move(dictionary);
                                    dictionary = std::move(old_dict);
                                } else {
                                    stack.push_back(obj->copy());
                                }
                            }
                        }
                    }
                } else {
                    stack.push_back(std::move(obj));
                }
            }
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
    s->push_back(std::make_unique<Int>(a * b));
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
        case TypeTag::OBJ: return ":Obj";
        case TypeTag::BOOL: return ":Bool";
        case TypeTag::INT: return ":Int";
        case TypeTag::FLT: return ":Flt";
        case TypeTag::STR: return ":Str";
        case TypeTag::ARR: return ":Arr";
        case TypeTag::EXE_ARR: return ":ExeArr";
        case TypeTag::PARAMS: return ":Params";
        case TypeTag::SYM: return ":Sym";
    }
}

void arr_close(Stack* s) {
    std::vector<std::unique_ptr<Obj>> arr;
    while(s->size() > 0
            && !(s->back()->tag == TypeTag::SYM
            && dynamic_cast<Sym*>(s->back().get())->str == "[")) {
            
        arr.push_back(std::move(s->back()));
        s->pop_back();
    }

    if(s->size() > 0
            && s->back()->tag == TypeTag::SYM
            && dynamic_cast<Sym*>(s->back().get())->str == "[") {
        
        s->pop_back();
        std::reverse(arr.begin(), arr.end());
        s->push_back(std::make_unique<Arr>(std::move(arr)));
    } else {
        // TODO error
    }
}

void sanitize_symbol(std::string& sym) {
    if(sym[0] == ':') {
        sym.erase(0, 1);
    }
    if(sym[sym.size()-1] == ':') {
        sym.pop_back();
    }
}

void store_symbol(Simulator* sim) {
    if(sim->stack.size() < 2) {
        // TODO error handling
    } else {
        auto val = std::move(sim->stack.back());
        sim->stack.pop_back();
        auto key = std::move(sim->stack.back());
        sim->stack.pop_back();

        if(key->tag != TypeTag::SYM) {
            // TODO error handling
        } else {
            auto dict_key = dynamic_cast<Sym*>(key.get())->str;
            sanitize_symbol(dict_key);        
            sim->dictionary[dict_key] = std::move(val);
        }
    }
}

void lam(Simulator* sim) {
    if(sim->stack.size() < 1) {
        // TODO error handling
    } else if(sim->stack.back()->tag != TypeTag::EXE_ARR) {
        // TODO error handling
    } else {
        auto exe_arr = dynamic_cast<ExeArr*>(sim->stack.back().get());
        exe_arr->add_dictionary(sim->dictionary.copy());
    }
}

std::unique_ptr<Sym> err(std::string err) {
    return std::make_unique<Sym>("Err: " + err);
}

void fun(Simulator* sim) {
    if(sim->stack.size() < 2) {
        sim->stack.push_back(err("Expected two elements"));
    } else {
        auto exe_arr_ptr = std::move(sim->stack.back());
        sim->stack.pop_back();
        auto key_ptr = std::move(sim->stack.back());
        sim->stack.pop_back();

        if(key_ptr->tag != TypeTag::SYM) {
            sim->stack.push_back(err("Expected a SYM found " + type_to_string(key_ptr->tag)));
        } else {
            auto key = dynamic_cast<Sym*>(key_ptr.get())->str;
            sanitize_symbol(key);

            auto exe_arr = dynamic_cast<ExeArr*>(exe_arr_ptr.get());
            exe_arr->add_dictionary(sim->dictionary.copy());
            //exe_arr->dictionary[key] = exe_arr_ptr;

            sim->dictionary[key] = std::move(exe_arr_ptr);

            std::shared_ptr<Obj> o = sim->dictionary[key];
            auto ref = dynamic_cast<ExeArr*>(o.get());
            ref->dictionary[key] = o;
        }
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

    if(x1->tag == TypeTag::INT && x2->tag == TypeTag::INT) {
        int_op(s, dynamic_cast<Int*>(x1.get())->i, dynamic_cast<Int*>(x2.get())->i);
    } else if(x1->tag == TypeTag::FLT && x2->tag == TypeTag::FLT) {
        flt_op(s, dynamic_cast<Flt*>(x1.get())->f, dynamic_cast<Flt*>(x2.get())->f);
    } else if(x1->tag == TypeTag::INT && x2->tag == TypeTag::FLT) {
        flt_op(s, dynamic_cast<Int*>(x1.get())->i, dynamic_cast<Flt*>(x2.get())->f);
    } else if(x1->tag == TypeTag::FLT && x2->tag == TypeTag::INT) {
        flt_op(s, dynamic_cast<Flt*>(x1.get())->f, dynamic_cast<Int*>(x2.get())->i);
    } else {
        // TODO error message
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

Simulator* rl_sim;

char* builtin_name_generator(const char* text, int state) {
    static std::vector<std::string> matches;
    static size_t match_index;

    if(state == 0) {
        matches.clear();
        match_index = 0;

        for(auto it : rl_sim->dictionary) {
            if(std::strncmp(it.first.c_str(), text, strlen(text)) == 0) {
                matches.push_back(it.first);
            }
        }
    }

    if(match_index >= matches.size()) {
        return nullptr;
    } else {
        return strdup(matches[match_index].c_str());
    }
}

char** builtin_name_completion(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, builtin_name_generator);
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
        {{"exit"}, [&running](Stack*) { running = false; }},
        {{"]"}, [](Stack* s) { arr_close(s); }},
        {{"stack"}, [&sim](Stack* s) { std::cout << sim << std::endl; }},
        {{"dict"}, [&sim](Stack* s) { sim.dictionary.print(std::cout); }},
        {{"!"}, [&sim](Stack* s) { store_symbol(&sim); }},
        {{"lam"}, [&sim](Stack* s) { lam(&sim); }},
        {{"fun"}, [&sim](Stack* s) { fun(&sim); }}
    };

    rl_sim = &sim;
    rl_attempted_completion_function = builtin_name_completion;

    std::cout << "PostFix - " << VERSION << std::endl;

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
        Lexer lexer(std::move(input));
        TokenType tok;
        std::string token;
        while((tok = lexer.next(token)) != TokenType::EOL) {
            if(tok == TokenType::STR) {
                sim.push(std::make_unique<Str>(token));
            } else if(tok == TokenType::SYM) {
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