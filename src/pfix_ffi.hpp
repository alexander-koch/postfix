#ifndef __PFIX_FFI_HPP__
#define __PFIX_FFI_HPP__

#include "types.hpp"
#include <map>

class PfixStack: public std::vector<std::unique_ptr<Obj>> {
public:
    std::unique_ptr<Obj> pop() {
        auto x = std::move(this->back());
        this->pop_back();
        return x;
    }

    void pushInt(int i) {
        this->push_back(std::make_unique<Int>(i));
    }

    int popInt() {
        return dynamic_cast<Int*>(this->pop().get())->i;
    }

    void expect(TypeTag tag) {
        if(!(this->size() > 0 && this->back()->tag == tag)) {
            if(this->size() > 0) {
                throw std::runtime_error("Expected " + type_to_string(tag) + ", found " + type_to_string(this->back()->tag));
            } else {
                throw std::runtime_error("Expected type" + type_to_string(tag));
            }
        }
    }
};

using PfixStackFunction = std::function<void(PfixStack* s)>;
using PfixNativeDictionary = std::map<std::string, PfixStackFunction>;

using PfixEntryPoint = void (*)(PfixNativeDictionary* dict);

#endif