#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <iostream>
#include <string>
#include <vector>

enum class TypeTag {
    OBJ,
    BOOL,
    INT,
    FLT,
    STR,
    ARR,
    EXE_ARR,
    PARAMS,
    SYM
};

bool is_type(const std::string& str) {
    return (str[0] == ':' || str[str.size()-1] == ':');// && std::isupper(str[1]);
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

class Obj {
public:
    TypeTag tag = TypeTag::OBJ;
    virtual ~Obj() = default;
    virtual std::ostream& print(std::ostream& os) = 0;
    virtual std::unique_ptr<Obj> copy() = 0;
protected:
    Obj(TypeTag t) : tag(t) {}
};

class Dictionary : public std::map<std::string, std::shared_ptr<Obj>> {
public:
    std::ostream& print(std::ostream& os) {
        std::cout << "{ ";
        for(auto& entry : *this) {
            std::cout << entry.first << ":";
            entry.second->print(os) << " ";
        }
        std::cout << "}" << std::endl;
        return os;
    }

    friend std::ostream& operator<<(std::ostream& os, Dictionary& dictionary) {
        return dictionary.print(os);
    }
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

    std::vector<std::unique_ptr<Obj>> vec;
    Arr(std::vector<std::unique_ptr<Obj>>&& vec)
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
        std::vector<std::unique_ptr<Obj>> copy_vec;
        std::transform(vec.begin(), vec.end(), std::back_inserter(copy_vec), [](auto& x) {
            return x->copy();
        });
        return std::make_unique<Arr>(std::move(copy_vec));
    }
};

class ExeArr : public Arr {
public:
    Dictionary dictionary;

    ExeArr(std::vector<std::unique_ptr<Obj>>&& vec, Dictionary dictionary = Dictionary())
        : Arr(std::move(vec)), dictionary(dictionary) {
        this->tag = TypeTag::EXE_ARR;
    }

    void add_dictionary(Dictionary dictionary) {
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
    Sym(std::string str) : Obj(TypeTag::SYM), str(std::move(str)) {}
    Sym(const char* c) : Obj(TypeTag::SYM), str(c) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << str;
        return os;
    }

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Sym>(str);
    }
};

#endif
