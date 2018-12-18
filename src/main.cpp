#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>

#include "types.hpp"
#include "lexer.hpp"

#define VERSION "v0.1.9"

#include "types.hpp"
#include "interpreter.hpp"

PfixInterpreter* rl_interp;

char* builtin_name_generator(const char* text, int state) {
    static std::vector<std::string> matches;
    static size_t match_index = 0;

    if(state == 0) {
        matches.clear();
        match_index = 0;

        for(auto it : rl_interp->dictionary) {
            if(std::strncmp(it.first.c_str(), text, strlen(text)) == 0) {
                matches.push_back(it.first);
            }
        }
    }

    if(match_index >= matches.size()) {
        return nullptr;
    } else {
        return strdup(matches[match_index++].c_str());
    }
}

char** builtin_name_completion(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, builtin_name_generator);
}

int main() {
    bool running = true;
    auto interp = PfixInterpreter();
    interp.load_builtins();

    rl_interp = &interp;
    rl_attempted_completion_function = builtin_name_completion;

    std::cout << "PostFix - " << VERSION << std::endl;
    std::cout << "Type ':exit' or Ctrl-D to exit" << std::endl;

    std::string prompt = ">>> ";
    bool exe_arr = false;
    size_t last = 0;
    while(running) {
        
        // Fetch user input
        char* buf = readline(prompt.c_str());
        if(buf == NULL) break;
        std::string input(buf);
        free(buf);
        if(input.size() > 0) add_history(input.c_str());

        // Split into tokens
        Lexer lexer(std::move(input));
        TokenType tok;
        std::string token;
        while((tok = lexer.next(token)) != TokenType::EOL) {
            switch(tok) {
                case TokenType::STR:
                    interp.push(std::make_unique<Str>(token));
                    break;
                case TokenType::BOOL:
                    interp.push(std::make_unique<Bool>(token == "true"));
                    break;
                case TokenType::INT:
                    interp.push(std::make_unique<Int>(std::stoi(token)));
                    break;
                case TokenType::FLT:
                    interp.push(std::make_unique<Flt>(std::stod(token)));
                    break;
                case TokenType::SYM:
                    interp.push(std::make_unique<Sym>(token));
                    break;
                case TokenType::EOL: break;
            }
        }

        if(interp.stack.size() <= last) {
            prompt = ">>> ";
            exe_arr = false;
            last = 0;
            continue;
        }
        
        if(interp.stack.size() > 0 && interp.stack.back()->tag == TypeTag::SYM) {
            auto obj = dynamic_cast<Sym*>(interp.stack.back().get());
            if(obj->str == "{") {
                prompt = "... ";
                last = interp.stack.size();
                exe_arr = true;
            }

            sanitize_symbol(obj->str);
            if(obj->str == "exit") {
                running = false;
            }
        }

        if(interp.stack.size() > 0 && !exe_arr) {
            interp.stack.back()->print(std::cout) << std::endl;
        }

    }

    return 0;
}
