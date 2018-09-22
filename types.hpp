#ifndef __TYPES_HPP__
#define __TYPES_HPP__

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
    Sym(std::string str) : Obj(T_SYM), str(std::move(str)) {}
    Sym(const char* c) : Obj(T_SYM), str(c) {}
    
    virtual std::ostream& print(std::ostream& os) override {
        os << str;
        return os;
    }
};

#endif