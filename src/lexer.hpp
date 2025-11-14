#ifndef __LEXER_HPP__
#define __LEXER_HPP__
#include <optional>
#include <string>
#include <vector>

namespace glass {
    enum class TokenType {
        Illegal = -1,
        EndOfFile,

        // reserved keywords
        FuncKeyword, ReturnKeyword, LetKeyword,

        // symbols
        OpenParentheses, CloseParentheses, Colon, Semicolon, OpenCurly, CloseCurly,
        // operators
        Equal, Plus, Minus, Slash, Asterisk,

        // literals
        Identifier, IntLiteral
    };

    struct Token {
        std::string value = {};
        TokenType type = TokenType::EndOfFile;

        size_t pos;

        bool operator==(TokenType other){
            return type == other;
        }
        bool operator!=(TokenType other){
            return type != other;
        }
    };

    class Lexer {
    public:
        explicit Lexer(std::string s) : input(std::move(s)) {
            length = input.length();
        }

        Token lookahead(){
            if (!lookahead_buf.has_value())
                lookahead_buf = next();
            return *lookahead_buf;
        }

        Token next();
    private:
        std::string input;
        size_t length;
        size_t pos = 0;
        std::optional<Token> lookahead_buf = {};

        std::string read_word(char init);

        std::optional<char> peek(){
            if (pos >= length)
                return {};
            return input[pos];
        }

        int peek_eof(){
            return peek().value_or(EOF);
        }

        std::optional<char> advance(){
            if (pos >= length)
                return {};
            return input[pos++];
        }
    };

}

#endif//__LEXER_HPP__

