#ifndef __LEXER_HPP__
#define __LEXER_HPP__

#include <iostream>
#include <string>
#include <clocale>
#include <codecvt>

enum class TokenType {
    STR,
    SYM,
    EOL // end of line
};

class Lexer {
public:
    unsigned int pos = 0;
    std::wstring input;
    std::locale loc;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    Lexer(std::string&& input) {
        std::setlocale(LC_ALL, "");
        this->input = converter.from_bytes(input);
    }

    bool peek(wchar_t& c);
    TokenType next(std::string& buffer);
};

bool is_punct(char c) {
    switch(c) {
        case '{':
        case '}':
        case '[':
        case ']': return true;
        default: return false;
    }
}

bool Lexer::peek(wchar_t& c) {
    if(pos >= input.size()) {
        return false;
    } else {
        c = input[pos];
        return true;
    }
}

TokenType Lexer::next(std::string& buffer) {
    wchar_t c;
    buffer.clear();

    // Skip all the spaces
    while(peek(c) && std::iswspace(c)) pos++;
    if(!peek(c)) return TokenType::EOL;
    pos++;

    if(c == '\"') {
        while(peek(c) && c != '\"') {
            buffer += converter.to_bytes(c);
            pos++;
        }
        pos++;
        return TokenType::STR;
    } else if(is_punct(c)) {
        buffer = converter.to_bytes(c);
    } else {
        buffer += converter.to_bytes(c);
        while(peek(c) && !std::iswspace(c) && !is_punct(c)) {
            buffer += converter.to_bytes(c);
            pos++;
        }
    }
    return TokenType::SYM;
}

#endif