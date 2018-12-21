#include "interpreter.hpp"

void param_list_close(PfixStack* s);

void arr_close(PfixStack* s) {
    std::vector<std::unique_ptr<Obj>> arr;
    while(s->size() > 0
            && !(s->back()->tag == TypeTag::SYM
            && dynamic_cast<Sym*>(s->back().get())->str == "[")) {

        arr.push_back(s->pop());
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

void param_list_close(PfixStack* s) {
    std::vector<std::pair<std::string, std::string>> params;
    std::vector<std::string> ret_types;
    std::vector<std::string> buffer;

    while(s->size() > 0
            && !(s->back()->tag == TypeTag::SYM
            && dynamic_cast<Sym*>(s->back().get())->str == "(")) {
        s->expect(TypeTag::SYM);
        buffer.push_back(dynamic_cast<Sym*>(s->pop().get())->str);
    }

    if(s->size() > 0
            && s->back()->tag == TypeTag::SYM
            && dynamic_cast<Sym*>(s->back().get())->str == "(") {

        s->pop_back();

        bool ret = false;
        for(auto it = buffer.rbegin(); it != buffer.rend(); it++) {
            if(*it == "->") {
                ret = true;
                continue;
            }

            if(is_type(*it)) {
                if(ret) ret_types.push_back(*it);
                else {
                    if(params.empty() || is_type(params.back().first)) {
                        throw std::runtime_error("Type names have to follow variable names");
                    }
                    auto name = params.back().first;
                    params.pop_back();
                    params.emplace_back(name, *it);
                }
            } else {
                if(ret) {
                    throw std::runtime_error("There must only be types after ->");
                }
                params.emplace_back(*it, ":Obj");
            }
        }

        s->push_back(std::make_unique<Params>(std::move(params), std::move(ret_types)));
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

void store_symbol(PfixInterpreter* interp) {
    if(interp->stack.size() < 2) {
        // TODO error handling
    } else {
        auto val = interp->stack.pop();
        auto key = interp->stack.pop();

        if(key->tag != TypeTag::SYM) {
            throw std::runtime_error("Expected a symbol first");
        } else {
            auto dict_key = dynamic_cast<Sym*>(key.get())->str;
            sanitize_symbol(dict_key);
            interp->dictionary[dict_key] = std::move(val);
        }
    }
}

void lam(PfixInterpreter* interp) {
    if(interp->stack.size() < 1) {
        // TODO error handling
    } else if(interp->stack.back()->tag != TypeTag::EXE_ARR) {
        // TODO error handling
    } else {
        auto exe_arr = dynamic_cast<ExeArr*>(interp->stack.back().get());
        exe_arr->add_dictionary(interp->dictionary);//.copy());
    }
}

void fun(PfixInterpreter* interp) {
    if(interp->stack.size() < 2) {
        throw std::runtime_error("Expected two elements");
    } else {
        auto exe_arr_ptr = interp->stack.pop();
        auto key_or_param = interp->stack.pop();

        if(key_or_param->tag != TypeTag::SYM && key_or_param->tag != TypeTag::PARAMS) {
            throw std::runtime_error("Expected a SYM / PARAM found " + type_to_string(key_or_param->tag));
        } else {
            std::string key;
            std::unique_ptr<Obj> key_ptr = NULL;
            Params::Parameters parameters;

            if(key_or_param->tag == TypeTag::PARAMS) {
                key_ptr = interp->stack.pop();
                auto params = dynamic_cast<Params*>(key_or_param.get());

                parameters = params->params;
            } else {
                key_ptr = std::move(key_or_param);
            }

            if(key_ptr->tag == TypeTag::SYM) {
                key = dynamic_cast<Sym*>(key_ptr.get())->str;
                sanitize_symbol(key);
            }

            auto exe_arr = dynamic_cast<ExeArr*>(exe_arr_ptr.get());
            exe_arr->add_dictionary(interp->dictionary);

            // Hijack executable array
            if(!parameters.empty()) {
                for(auto& it : parameters) {
                    exe_arr->vec.push_front(std::make_unique<Sym>("!"));
                    exe_arr->vec.push_front(std::make_unique<Sym>(":"+it.first));  
                }
            }

            interp->dictionary[key] = std::move(exe_arr_ptr);

            std::shared_ptr<Obj> o = interp->dictionary[key];
            auto ref = dynamic_cast<ExeArr*>(o.get());
            ref->dictionary[key] = o;
        }
    }
}

void print_top(PfixStack* s) {
    if(s->size() > 0) {
        s->back()->print(std::cout);
        s->pop_back();
    }
}

void int_to_flt(PfixStack* s, std::unique_ptr<Obj> x) {
    s->push_back(std::make_unique<Flt>(dynamic_cast<Int*>(x.get())->i));
}

void type_to_symbol(PfixStack* s, std::unique_ptr<Obj> x) {
    s->push_back(std::make_unique<Sym>(type_to_string(x->tag)));
}

using UnaryFunc = std::function<void(PfixStack*, std::unique_ptr<Obj>)>;

void unary_op(PfixStack* s, UnaryFunc op) {
    if(s->size() > 0) {
        op(s, s->pop());
    }
}

void binary_int_op(PfixStack* s, std::function<int(int, int)> op) {
    if(s->size() < 2) {
        auto x2 = s->pop();
        dynamic_cast<Int*>(s->back().get())->i =
            op(dynamic_cast<Int*>(s->back().get())->i, dynamic_cast<Int*>(x2.get())->i);
    }
}

void binary_arith_op(PfixStack* s,
        std::function<int(int, int)> int_op,
        std::function<double(double, double)> flt_op,
        bool int_ret_expect_float=false) {
    if(s->size() < 2) return;
    auto x2 = s->pop();
    auto x1 = s->back().get();

    if((x1->tag != TypeTag::INT && x1->tag != TypeTag::FLT)
        || (x2->tag != TypeTag::INT && x2->tag != TypeTag::FLT)) {
        // Error
        throw std::runtime_error("Invalid binary arithmetic operation");
    } else if(x1->tag == TypeTag::INT && x2->tag == TypeTag::INT) {
        if(int_ret_expect_float) {
            s->back() = std::make_unique<Flt>(flt_op(dynamic_cast<Int*>(x1)->i, dynamic_cast<Int*>(x2.get())->i));
        } else {
            dynamic_cast<Int*>(x1)->i =
                int_op(dynamic_cast<Int*>(x1)->i, dynamic_cast<Int*>(x2.get())->i);
        }
    } else {
        double res = 0.0;
        if(x1->tag == TypeTag::FLT && x2->tag == TypeTag::FLT) {
            res = flt_op(dynamic_cast<Flt*>(x1)->f, dynamic_cast<Flt*>(x2.get())->f);
        } else if(x1->tag == TypeTag::INT && x2->tag == TypeTag::FLT) {
            res = flt_op(dynamic_cast<Int*>(x1)->i, dynamic_cast<Flt*>(x2.get())->f);
        } else if(x1->tag == TypeTag::FLT && x2->tag == TypeTag::INT) {
            res = flt_op(dynamic_cast<Flt*>(x1)->f, dynamic_cast<Int*>(x2.get())->i);
        }
        s->back() = std::make_unique<Flt>(res);
    }
}

void binary_logical_op(PfixStack* s, std::function<bool(bool, bool)> bool_op) {
    if(s->size() < 1) return;
    auto x = s->pop();

    if(x->tag == TypeTag::ARR) {
        // Multiple boolean values
    } else if(x->tag == TypeTag::BOOL) {
        if(s->size() < 1) return;
        dynamic_cast<Bool*>(s->back().get())->b =
            bool_op(dynamic_cast<Bool*>(x.get())->b,
                dynamic_cast<Bool*>(s->back().get())->b);
    } else {
        // TODO error handling
    }
}

void add_op(PfixStack* s) {
    if(s->size() < 2) return;
    if(s->back()->tag == TypeTag::STR) {
        auto x2 = s->pop();
        auto x1 = s->back().get();
        if(x1->tag != TypeTag::STR) {
            // TODO erro
        } else {
            dynamic_cast<Str*>(x1)->str.append(dynamic_cast<Str*>(x2.get())->str);
        }
    } else {
        binary_arith_op(s, std::plus<int>(), std::plus<double>());
    }
}

void load_library(PfixInterpreter* interp, PfixStack* s) {
    auto x = std::move(s->back());
    s->pop_back();
    auto path = dynamic_cast<Str*>(x.get())->str;

    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if(handle == NULL) {
        throw std::runtime_error("Could not open " + path);
    }

    const size_t period_idx = path.rfind('.');
    if(std::string::npos != period_idx) {
        path.erase(period_idx);
        path[0] = std::toupper(path[0]);
    }

    auto method_name = "PfixInit" + path;
    auto method = (PfixEntryPoint)dlsym(handle, method_name.c_str());

    if(method != NULL) {
        method(&interp->dictionary);
    }
}

bool PfixInterpreter::evaluate_dictionary(std::string& sym) {
    auto iter = dictionary.find(sym);
    if(iter == dictionary.end()) return false;
    auto& obj = iter->second;

    // If we found an executable array, use caution
    // Native symbol, call method
    // Otherwise just push the obj on the stack
    if(obj->tag == TypeTag::EXE_ARR) {
        // Set the new dictionary
        auto old_dict = std::move(dictionary);
        auto exe_arr = dynamic_cast<ExeArr*>(obj.get());
        dictionary = exe_arr->dictionary;

        for(auto& x : exe_arr->vec) {
            push(x->copy());
        }

        // Reset
        dictionary = std::move(old_dict);
    } else if(obj->tag == TypeTag::NATIVE_SYM) {
        auto nsym = dynamic_cast<NativeSym*>(obj.get());
        nsym->function(&stack);
    } else {
        stack.push_back(obj->copy());
    }
    return true;
}

void PfixInterpreter::evaluate_symbol(std::string& sym) {
    if(sym[sym.size()-1] == '!') {
        sym.pop_back();
        if(stack.size() > 0) {
            dictionary[sym] = stack.pop();
        } else {
            throw std::runtime_error("No symbol on the stack");
        }
    } else {
        evaluate_dictionary(sym);
    }
}

constexpr int divides_int(double lhs, double rhs) {
    return (int)lhs / (int)rhs;
}

void PfixInterpreter::load_builtins() {
     std::map<std::string, PfixStackFunction> builtins = {
        {{"+"}, [](PfixStack* s) { add_op(s); }},
        {{"-"}, [](PfixStack* s) { binary_arith_op(s, std::minus<int>(), std::minus<double>()); }},
        {{"*"}, [](PfixStack* s) { binary_arith_op(s, std::multiplies<int>(), std::multiplies<double>()); }},
        {{"/"}, [](PfixStack* s) { binary_arith_op(s, std::divides<double>(), std::divides<double>(), true); }},
        {{"i/"}, [](PfixStack* s) { binary_arith_op(s, std::divides<int>(), divides_int); }},
        {{"mod"}, [](PfixStack* s) { binary_int_op(s, std::modulus<int>()); }},
        {{"and"}, [](PfixStack* s) { binary_logical_op(s, std::logical_and<bool>()); }},
        {{"or"}, [](PfixStack* s) { binary_logical_op(s, std::logical_or<bool>()); }},
        {{"int->flt"}, [](PfixStack* s) { unary_op(s, int_to_flt); }},
        {{"print"}, [](PfixStack* s) { print_top(s); }},
        {{"println"}, [](PfixStack* s) { print_top(s); std::cout << std::endl; }},
        {{"clear"}, [](PfixStack* s) { s->clear(); }},
        {{"type"}, [](PfixStack* s) { unary_op(s, type_to_symbol); }},
        {{"]"}, [](PfixStack* s) { arr_close(s); }},
        {{")"}, [](PfixStack* s) { param_list_close(s); }},
        {{"stack"}, [this](PfixStack* s) { std::cout << *this << std::endl; }},
        {{"dict"}, [this](PfixStack* s) { this->dictionary.print(std::cout); }},
        {{"!"}, [this](PfixStack* s) { store_symbol(this); }},
        {{"lam"}, [this](PfixStack* s) { lam(this); }},
        {{"fun"}, [this](PfixStack* s) { fun(this); }},
        {{"load-library"}, [this](PfixStack* s) { load_library(this, s); }}
    };

    for(auto it : builtins) {
        dictionary.emplace(it.first, std::make_unique<NativeSym>(it.second));
    }
}

void PfixInterpreter::push(std::unique_ptr<Obj> obj) {
    if(obj->tag == TypeTag::SYM) {
        auto sym = dynamic_cast<Sym*>(obj.get())->str;
        if(sym[0] == ':' || sym[sym.size()-1] == ':' || sym == "[" || sym == "->") {
            stack.push_back(std::move(obj));
        } else if(sym == "(") {
            stack.push_back(std::move(obj));
            evaluate_on_push = false;
        } else if(sym == ")") {
            evaluate_on_push = true;
            param_list_close(&stack);    
        } else if(sym == "{") {
            stack.push_back(std::move(obj));

            evaluate_on_push = false;
            if(exe_arr == 0) exe_begin = stack.size();
            exe_arr++;
        } else if(sym == "}") {
            exe_arr--;
            if(exe_arr < 0) {
                throw std::runtime_error("Closing executable array without beginning");
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
            if(evaluate_on_push) evaluate_symbol(sym);
            else stack.push_back(std::move(obj));
        }
    } else {
        stack.push_back(std::move(obj));
    }
}

std::ostream& operator<<(std::ostream& os, PfixInterpreter& interp) {
    bool comma = false;
    os << "[";
    std::for_each(interp.stack.begin(), interp.stack.end(), [&os, &comma](auto& x) {
        if(comma) os << ", ";
        x->print(os);
        comma = true;
    });
    os << "]";
    return os;
}