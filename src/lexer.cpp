#include "lexer.hpp"
#include <sstream>
#include <ctype.h>

static const std::vector<std::string> reserved = {
    "func",
    "return",
    "let"
};

static constexpr const char *symbols = "():;{}=+-/*";

auto glass::Lexer::next() -> Token {
    if (lookahead_buf.has_value()){
        Token tok = lookahead_buf.value();
        lookahead_buf = {};
        return tok;
    }
    while (isspace(peek_eof()))
        advance();
    Token tok;
    tok.pos = pos;
    if (!peek().has_value())
        return tok;
    char n = advance().value();
    if (isalnum(n) || n == '_' || n == '$'){
        tok.value = read_word(n);
        tok.type = isalpha(n) ? TokenType::Identifier : TokenType::IntLiteral;
        auto it = std::find(reserved.cbegin(), reserved.cend(), tok.value);
        if (it != reserved.cend()){
            tok.type = (TokenType)(it - reserved.cbegin() + (int)TokenType::FuncKeyword);
        }
    } else if (const char *symptr = strchr(symbols, n); symptr && *symptr) {
        tok.value = std::string() + n;
        tok.type = (TokenType)(symptr - symbols + (int)TokenType::OpenParentheses);
    } else {
        tok.value = std::string() + n;
        tok.type = TokenType::Illegal;
    }
    return tok;
}

std::string glass::Lexer::read_word(char init) {
    std::ostringstream buf = {};
    buf << init;
    int ch;
    while (ch = peek_eof(), isalnum(ch) || ch == '_' || ch == '$')
        buf << advance().value();
    return buf.str();
}

