#include "../src/pfix_ffi.hpp"

int fib(int n) {
    if(n <= 1) return 1;
    else {
        return fib(n-2) + fib(n-1);
    }
}

void PfixFib(PfixStack* s) {
    auto x = s->popInt();
    s->pushInt(fib(x));
}

extern "C" {

void PfixInitExample(PfixNativeDictionary* dict) {
    std::cout << "Hello" << std::endl;
    dict->insert(std::make_pair("fib", PfixFib));
}

}