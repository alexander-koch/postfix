#include "types.hpp"

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
        case TypeTag::NATIVE_SYM: return ":NativeSym";
    }
    throw std::logic_error("not implemented");
}

std::unique_ptr<Obj> PfixStack::pop() {
    if(this->empty()) throw std::runtime_error("Stack is empty and cannot be popped");
    auto x = std::move(this->back());
    this->pop_back();
    return x;
}

void PfixStack::pushInt(int i) {
    this->push_back(std::make_unique<Int>(i));
}

int PfixStack::popInt() {
    return dynamic_cast<Int*>(this->pop().get())->i;
}

void PfixStack::expect(TypeTag tag) {
    if(!(this->size() > 0 && this->back()->tag == tag)) {
        if(this->size() > 0) {
            throw std::runtime_error("Expected " + type_to_string(tag) + ", found " + type_to_string(this->back()->tag));
        } else {
            throw std::runtime_error("Expected type" + type_to_string(tag));
        }
    }
}

std::ostream& PfixDictionary::print(std::ostream& os) {
    std::cout << "{ ";
    for(auto& entry : *this) {
        std::cout << entry.first << ":";
        entry.second->print(os) << " ";
    }
    std::cout << "}" << std::endl;
    return os;
}

void PfixDictionary::define_native(const std::string& sym, PfixStackFunction sf) {
    this->insert(std::make_pair(sym, std::make_unique<NativeSym>(sf)));
}

std::ostream& operator<<(std::ostream& os, PfixDictionary& dictionary) {
    return dictionary.print(os);
}
