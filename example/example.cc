#include "../src/types.hpp"

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

void PfixInitExample(PfixDictionary* dict) {
    std::cout << "Hello" << std::endl;
    //auto nsym = std::make_unique<NativeSym>(PfixFib);

    dict->emplace("fib", std::make_unique<NativeSym>(PfixFib));
}

}