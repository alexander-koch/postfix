#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>

#include <readline/readline.h>
#include <readline/history.h>

#define VERSION "v0.1.1"

enum TypeTag {
    T_OBJ,
    T_BOOL,
    T_INT,
    T_FLT,
    T_STR,
    T_ARR,
    T_EXE_ARR,
    T_PARAMS,
    T_SYM
};

class Obj {
public:
    TypeTag tag = TypeTag::T_OBJ;
    virtual ~Obj() {}
    virtual std::ostream& print(std::ostream& os) = 0;
protected:
    Obj(TypeTag t) : tag(t) {}
};

class Bool : public Obj {
public:
    bool b;
    Bool(bool b) : Obj(T_BOOL), b(b) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << b;
        return os;
    }
};

class Int : public Obj {
public:
    int i;
    Int(int i) : Obj(T_INT), i(i) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << i;
        return os;
    }
};

class Flt : public Obj {
public:
    double f;
    Flt(double f) : Obj(T_FLT), f(f) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << f;
        return os;
    }
};

class Str : public Obj {
public:
    std::string str;
    Str(std::string& str) : Obj(T_STR), str(std::move(str)) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << str;
        return os;
    }
};

class Arr : public Obj {
public:
    std::vector<std::unique_ptr<Obj>> vec;
    Arr(std::vector<std::unique_ptr<Obj>> vec, bool is_executable=false)
        : Obj(is_executable ? T_EXE_ARR : T_ARR), vec(std::move(vec)) {}

    virtual std::ostream& print(std::ostream& os) override {
        std::for_each(vec.begin(), vec.end(), [&os](auto& t) {
            os << t.get();
        });
        return os;
    }
};

class Sym : public Obj {
public:
    std::string str;
    Sym(std::string& str) : Obj(T_SYM), str(std::move(str)) {}
    Sym(const char* c) : Obj(T_SYM), str(c) {}
    
    virtual std::ostream& print(std::ostream& os) override {
        os << str;
        return os;
    }
};

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
            if(sym[0] == ':' || sym[sym.size()-1] == ':') {
                stack.push_back(std::move(obj)); 
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

using UnaryFunc = std::function<std::unique_ptr<Obj>(std::unique_ptr<Obj>)>;
using ArithFunc = std::function<std::unique_ptr<Obj>(std::unique_ptr<Obj>, std::unique_ptr<Obj>)>;

std::unique_ptr<Obj> add_int(std::unique_ptr<Obj> a, std::unique_ptr<Obj> b) {
    return std::make_unique<Int>(dynamic_cast<Int*>(a.get())->i + dynamic_cast<Int*>(b.get())->i);
}

std::unique_ptr<Obj> add_flt(std::unique_ptr<Obj> a, std::unique_ptr<Obj> b) {
    return std::make_unique<Int>(dynamic_cast<Flt*>(a.get())->f + dynamic_cast<Flt*>(b.get())->f);
}

std::unique_ptr<Obj> sub_int(std::unique_ptr<Obj> a, std::unique_ptr<Obj> b) {
    return std::make_unique<Int>(dynamic_cast<Int*>(a.get())->i - dynamic_cast<Int*>(b.get())->i);
}

std::unique_ptr<Obj> sub_flt(std::unique_ptr<Obj> a, std::unique_ptr<Obj> b) {
    return std::make_unique<Int>(dynamic_cast<Flt*>(a.get())->f - dynamic_cast<Flt*>(b.get())->f);
}

std::unique_ptr<Obj> mul_int(std::unique_ptr<Obj> a, std::unique_ptr<Obj> b) {
    return std::make_unique<Int>(dynamic_cast<Int*>(a.get())->i * dynamic_cast<Int*>(b.get())->i);
}

std::unique_ptr<Obj> mul_flt(std::unique_ptr<Obj> a, std::unique_ptr<Obj> b) {
    return std::make_unique<Int>(dynamic_cast<Flt*>(a.get())->f * dynamic_cast<Flt*>(b.get())->f);
}

std::unique_ptr<Obj> div_int(std::unique_ptr<Obj> a, std::unique_ptr<Obj> b) {
    return std::make_unique<Flt>(dynamic_cast<Int*>(a.get())->i / static_cast<float>(dynamic_cast<Int*>(b.get())->i));
}

std::unique_ptr<Obj> div_flt(std::unique_ptr<Obj> a, std::unique_ptr<Obj> b) {
    return std::make_unique<Flt>(dynamic_cast<Flt*>(a.get())->f / dynamic_cast<Flt*>(b.get())->f);
}

std::unique_ptr<Obj> idiv(std::unique_ptr<Obj> a, std::unique_ptr<Obj> b) {
    return std::make_unique<Int>(dynamic_cast<Int*>(a.get())->i / dynamic_cast<Int*>(b.get())->i);
}

std::unique_ptr<Obj> mod(std::unique_ptr<Obj> a, std::unique_ptr<Obj> b) {
    return std::make_unique<Int>(dynamic_cast<Int*>(a.get())->i % dynamic_cast<Int*>(b.get())->i);
}

std::unique_ptr<Obj> int_to_flt(std::unique_ptr<Obj> x) {
    return std::make_unique<Flt>(dynamic_cast<Int*>(x.get())->i);
}

std::unique_ptr<Obj> type_to_symbol(std::unique_ptr<Obj> x) {
    std::string name;
    switch(x->tag) {
        case T_OBJ: name = ":Obj"; break;
        case T_BOOL: name = ":Bool"; break;
        case T_INT: name = ":Int"; break;
        case T_FLT: name = ":Flt"; break;
        case T_STR: name = ":Str"; break;
        case T_ARR: name = ":Arr"; break;
        case T_EXE_ARR: name = ":ExeArr"; break;
        case T_PARAMS: name = ":Params"; break;
        case T_SYM: name = ":Sym"; break;
    }
    return std::make_unique<Sym>(name);
}

void unary_op(Stack* s, UnaryFunc op) {
    if(s->size() == 0) return;
    auto x = std::move(s->back());
    s->pop_back();
    s->push_back(op(std::move(x)));
}

void binary_int_op(Stack* s, ArithFunc op) {
    if(s->size() < 2) return;
     auto x2 = std::move(s->back());
    s->pop_back();
    auto x1 = std::move(s->back());
    s->pop_back();

    if(x1->tag == T_INT && x2->tag == T_INT) {
        s->push_back(op(std::move(x1), std::move(x2)));
    }
}

void binary_arith_op(Stack* s, ArithFunc int_op, ArithFunc flt_op) {
    if(s->size() < 2) return;
    auto x2 = std::move(s->back());
    s->pop_back();
    auto x1 = std::move(s->back());
    s->pop_back();

    if(x1->tag == T_INT && x2->tag == T_INT) {
        s->push_back(int_op(std::move(x1), std::move(x2)));
    } else if(x1->tag == T_FLT && x2->tag == T_FLT) {
        s->push_back(flt_op(std::move(x1), std::move(x2)));
    } else if(x1->tag == T_INT && x2->tag == T_FLT) {
        s->push_back(flt_op(int_to_flt(std::move(x1)), std::move(x2)));
    } else if(x1->tag == T_FLT && x2->tag == T_INT) {
        s->push_back(flt_op(std::move(x1), int_to_flt(std::move(x2))));
    }
}

void print_top(Stack* s) {
    if(s->size() > 0) {
        s->back()->print(std::cout);
        s->pop_back();
    }
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

int main() {
    bool running = true;
    auto sim = Simulator();
    sim.functions = {
        {{"+"}, [](Stack* s) { binary_arith_op(s, add_int, add_flt); }},
        {{"-"}, [](Stack* s) { binary_arith_op(s, sub_int, sub_flt); }},
        {{"*"}, [](Stack* s) { binary_arith_op(s, mul_int, mul_flt); }},
        {{"/"}, [](Stack* s) { binary_arith_op(s, div_int, div_flt); }},
        {{"i/"}, [](Stack* s) { binary_int_op(s, idiv); }},
        {{"mod"}, [](Stack* s) { binary_int_op(s, mod); }},
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

        bool escape_string = false;
        std::string stringbuffer;

        std::istringstream ss(input);
        std::string buffer;
        while(std::getline(ss, buffer, ' ')) {
            if(buffer.empty()) continue;

            // Check for strings
            if(buffer[0] == '\"') {
                escape_string = true;
                buffer = buffer.substr(1);
            }

            auto pos = buffer.find_first_of("\"");
            if(pos != std::string::npos) {
                stringbuffer += buffer.substr(0, pos);
                if(pos < buffer.size()-1) buffer = buffer.substr(pos+1);
                escape_string = false;
                sim.push(std::make_unique<Str>(stringbuffer));
            }

            if(escape_string) {
                stringbuffer += buffer + " ";
                continue;
            }  

            if(is_integer(buffer)) {
                sim.push(std::make_unique<Int>(std::stoi(buffer)));
            } else if(is_double(buffer)) {
                sim.push(std::make_unique<Flt>(std::stod(buffer)));
            } else {
                sim.push(std::make_unique<Sym>(buffer));
            }
        }
    }

    return 0;
}