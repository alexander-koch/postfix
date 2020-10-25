#ifndef __LEXER_HPP__
#define __LEXER_HPP__

#include <iostream>
#include <string>
#include <locale>
#include <clocale>
#include <codecvt>
#include <algorithm>

enum class TokenType {
    STR,
    SYM,
    BOOL,
    INT,
    FLT,
    EOL // end of line
};

class Lexer {
public:
    unsigned int pos = 0;
    std::wstring input;
    std::locale loc;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    Lexer(std::string&& input);
    bool peek(wchar_t& c);
    TokenType next(std::string& buffer);
};

#endif
