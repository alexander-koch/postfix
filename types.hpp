#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <iostream>

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
    virtual std::unique_ptr<Obj> copy() = 0;
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

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Bool>(b);
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

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Int>(i);
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

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Flt>(f);
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

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Str>(str);
    }
};

class Arr : public Obj {
public:
    std::vector<std::unique_ptr<Obj>> vec;
    Arr(std::vector<std::unique_ptr<Obj>> vec, bool is_executable=false)
        : Obj(is_executable ? T_EXE_ARR : T_ARR), vec(std::move(vec)) {}

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
        std::transform(vec.begin(), vec.end(), copy_vec.begin(), [](auto& x) {
            return x->copy();
        });
        return std::make_unique<Arr>(std::move(copy_vec));
    }
};

class Sym : public Obj {
public:
    std::string str;
    Sym(std::string str) : Obj(T_SYM), str(std::move(str)) {}
    Sym(const char* c) : Obj(T_SYM), str(c) {}
    
    virtual std::ostream& print(std::ostream& os) override {
        os << str;
        return os;
    }

    virtual std::unique_ptr<Obj> copy() override {
        return std::make_unique<Sym>(str);
    }
};

#endif