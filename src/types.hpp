#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <stdexcept>
#include <memory>
#include <functional>
#include <algorithm>

enum class TypeTag {
    OBJ,
    BOOL,
    INT,
    FLT,
    STR,
    ARR,
    EXE_ARR,
    PARAMS,
    SYM,
    NATIVE_SYM,
};

bool is_type(const std::string& str);
std::string type_to_string(TypeTag tag);

class PfixStack;
class PfixDictionary;
class Obj;

using PfixStackFunction = std::function<void(PfixStack* s)>;
using PfixEntryPoint = void (*)(PfixDictionary* dict);

class PfixStack: public std::vector<std::unique_ptr<Obj>> {
public:
    std::unique_ptr<Obj> pop();
    void pushInt(int i);
    int popInt();
    void expect(TypeTag tag);
};

class PfixDictionary : public std::map<std::string, std::shared_ptr<Obj>> {
public:
    std::ostream& print(std::ostream& os);

    void define_native(const std::string& sym, PfixStackFunction sf);

    friend std::ostream& operator<<(std::ostream& os, PfixDictionary& dictionary);
};

class Obj {
public:
    TypeTag tag = TypeTag::OBJ;
    virtual ~Obj() = default;
    virtual std::ostream& print(std::ostream& os) = 0;
    virtual std::unique_ptr<Obj> copy() = 0;
protected:
    Obj(TypeTag t) : tag(t) {}
};

class Bool : public Obj {
public:
    bool b;
    Bool(bool b) : Obj(TypeTag::BOOL), b(b) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << (b ? "true" : "false");
        return os;
    }

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Bool>(b);
    }
};

class Int : public Obj {
public:
    int i;
    Int(int i) : Obj(TypeTag::INT), i(i) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << i;
        return os;
    }

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Int>(i);
    }
};

class Flt : public Obj {
public:
    double f;
    Flt(double f) : Obj(TypeTag::FLT), f(f) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << f;
        return os;
    }

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Flt>(f);
    }
};

class Str : public Obj {
public:
    std::string str;
    Str(std::string& str) : Obj(TypeTag::STR), str(std::move(str)) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << str;
        return os;
    }

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Str>(str);
    }
};

class Arr : public Obj {
public:
    virtual ~Arr() override = default;

    std::deque<std::unique_ptr<Obj>> vec;

    Arr(std::vector<std::unique_ptr<Obj>>&& vec)
        : Obj(TypeTag::ARR) {

        std::move(vec.begin(), vec.end(), std::back_inserter(this->vec));
    }

    Arr(std::deque<std::unique_ptr<Obj>>&& vec)
        : Obj(TypeTag::ARR), vec(std::move(vec)) {}

    virtual std::ostream& print(std::ostream& os) override {
        bool comma = false;
        os << "[";
        std::for_each(vec.begin(), vec.end(), [&os, &comma](auto& t) {
            if(comma) os << ", ";
            t->print(os);
            comma = true;
        });
        os << "]";
        return os;
    }

    virtual std::unique_ptr<Obj> copy() override {
        std::deque<std::unique_ptr<Obj>> copy_vec;
        std::transform(vec.begin(), vec.end(), std::back_inserter(copy_vec), [](auto& x) {
            return x->copy();
        });
        return std::make_unique<Arr>(std::move(copy_vec));
    }
};

class ExeArr : public Arr {
public:
    PfixDictionary dictionary;

    ExeArr(std::vector<std::unique_ptr<Obj>>&& vec, PfixDictionary dictionary = PfixDictionary())
        : Arr(std::move(vec)), dictionary(dictionary) {
        this->tag = TypeTag::EXE_ARR;
    }

    void add_dictionary(PfixDictionary dictionary) {
        this->dictionary = dictionary;
    }

    virtual std::unique_ptr<Obj> copy() override {
        std::vector<std::unique_ptr<Obj>> copy_vec;
        std::transform(vec.begin(), vec.end(), std::back_inserter(copy_vec), [](auto& x) {
            return x->copy();
        });

        return std::make_unique<ExeArr>(std::move(copy_vec), dictionary);
    }
};

class Params : public Obj {
public:
    using Parameters = std::vector<std::pair<std::string, std::string>>;
    using ReturnTypes = std::vector<std::string>;

    Parameters params;
    ReturnTypes ret_types;

    Params(const Parameters& params, const ReturnTypes& ret_types)
        : Obj(TypeTag::PARAMS),
        params(params),
        ret_types(ret_types) {}

    Params(Parameters&& params, ReturnTypes&& ret_types)
        : Obj(TypeTag::PARAMS),
        params(std::move(params)),
        ret_types(std::move(ret_types)) {}

    virtual std::ostream& print(std::ostream& os) override {
        bool comma = false;
        os << "(";
        std::for_each(params.begin(), params.end(), [&os, &comma](auto& t) {
            if(comma) os << ", ";
            //t->print(os);
            os << t.first << " " << t.second;
            comma = true;
        });

        if(ret_types.size() > 0) os << " ->";

        std::for_each(ret_types.begin(), ret_types.end(), [&os](auto& t) {
            os << " " << t;
        });
        os << ")";
        return os;
    }

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Params>(params, ret_types);
    }
};

class Sym : public Obj {
public:
    std::string str;
    Sym(const std::string& str) : Obj(TypeTag::SYM), str(str) {}
    // Sym(const char* c) : Obj(TypeTag::SYM), str(c) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << str;
        return os;
    }

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Sym>(str);
    }
};

class NativeSym : public Obj {
public:
    PfixStackFunction function;

    NativeSym(PfixStackFunction function)
        : Obj(TypeTag::NATIVE_SYM), function(function) {}

    virtual std::ostream& print(std::ostream& os) override {
        std::cout << "native";
        return os;
    }

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<NativeSym>(function);
    }
};

#endif
