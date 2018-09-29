#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <iostream>

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
    Dictionary copy() {
        Dictionary dict;
        for(auto& entry : *this) {
            dict.emplace(entry.first, entry.second);//->copy());
        }
        return dict;
    }

    std::ostream& print(std::ostream& os) {
        std::cout << "{ ";
        for(auto& entry : *this) {
            std::cout << entry.first << ":";
            entry.second->print(os) << " ";
        }
        std::cout << "}" << std::endl;
        return os;
    }
};

class Bool : public Obj {
public:
    bool b;
    Bool(bool b) : Obj(TypeTag::BOOL), b(b) {}

    virtual std::ostream& print(std::ostream& os) override {
        os << b;
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
    Arr(std::vector<std::unique_ptr<Obj>> vec)
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

    ExeArr(std::vector<std::unique_ptr<Obj>> vec, Dictionary&& dictionary = Dictionary())
        : Arr(std::move(vec)), dictionary(std::move(dictionary)) {
        this->tag = TypeTag::EXE_ARR;
    }

    void add_dictionary(Dictionary&& dictionary) {
        this->dictionary = std::move(dictionary);
    }

    virtual std::unique_ptr<Obj> copy() override {
        std::vector<std::unique_ptr<Obj>> copy_vec;
        std::transform(vec.begin(), vec.end(), std::back_inserter(copy_vec), [](auto& x) {
            return x->copy();
        });

        return std::make_unique<ExeArr>(std::move(copy_vec), dictionary.copy());
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