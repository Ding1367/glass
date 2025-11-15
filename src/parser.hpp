#ifndef __PARSER_HPP__
#define __PARSER_HPP__
#include <vector>
#include <sstream>
#include <variant>
#include <iostream>
#include <stdint.h>
#include "lexer.hpp"

namespace glass {
    struct ASTNode {
        virtual ~ASTNode() = default;
        std::shared_ptr<ASTNode> parent = nullptr;
    };

    struct ExprNode : ASTNode {
        virtual ~ExprNode() = default; 
    };

    struct IdentExpr : ExprNode {
        ~IdentExpr() override = default;

        explicit IdentExpr(Token tok) : ident(tok){}
        Token ident;
    };

    struct BlockExpr : ExprNode {
        ~BlockExpr() override = default;
        std::vector<std::shared_ptr<ASTNode>> nodes = {};
    };

    struct DeclNode : ASTNode {
        virtual ~DeclNode() = default;

        size_t pos;
        Token name;
        Token type;
    };

    struct FuncDeclNode : DeclNode {
        ~FuncDeclNode() override = default;
        std::shared_ptr<BlockExpr> block = nullptr;
    };

    struct FuncCallExpr : ExprNode {
        ~FuncCallExpr() override = default;

        std::vector<std::shared_ptr<ExprNode>> parameters = {};
        std::shared_ptr<ExprNode> func;
    };

    struct LiteralExpr : ExprNode {
        ~LiteralExpr() override = default;
        explicit LiteralExpr(Token lit) : lit(std::move(lit)) {}
        Token lit;
    };

    struct BinaryExpr : ExprNode {
        ~BinaryExpr() override = default;
        std::shared_ptr<ExprNode> lhs, rhs;
        Token op;
    };

    struct UnaryExpr : ExprNode {
        ~UnaryExpr() override = default;
        Token op;
        std::shared_ptr<ExprNode> expr;
    };

    struct RetStmt : ASTNode {
        ~RetStmt() override = default;

        std::shared_ptr<ExprNode> expr;
    };

    class Parser {
    public:
        explicit Parser(Lexer &&lex) : lex(std::move(lex)){}
        std::vector<std::shared_ptr<ASTNode>> nodes = {};

        bool test(TokenType type){
            return lex.lookahead() == type;
        }

        Token expect(TokenType type, const std::string &name){
            if (test(type))
                return lex.next();
            error("expected {}, got type {}", name, (int)lex.lookahead().type);
            return {};
        }

        bool next_node();

        std::shared_ptr<ExprNode> parse_expr(int min_lbp = 0);
        std::shared_ptr<BlockExpr> parse_block_expr(){
            expect(TokenType::OpenCurly, "open curly brace to start block");
            auto block = std::make_shared<BlockExpr>();
            scope = block;
            while (!test(TokenType::CloseCurly)){
                next_node();
            }
            lex.next();
            scope = std::dynamic_pointer_cast<BlockExpr>(scope->parent);
            return block;
        }
    private:
        Lexer lex;
        std::shared_ptr<BlockExpr> scope = nullptr;
        
        void push(std::shared_ptr<ASTNode> node){
            node->parent = scope;
            if (!scope)
                nodes.push_back(node);
            else
                scope->nodes.push_back(node);
        }

        template <typename ...Args>
        void error(const std::string &fmt, Args... args){
            error(lex.get_pos(), fmt, std::forward<Args>(args)...);
        }

        template <typename T>
        void error_helper(std::ostringstream &builder, const char *&ptr, T t){
            const char *next = strchr(ptr, '{');
            if (!next){
                builder << ptr;
                return;
            }
            size_t len = next - ptr;
            std::string segment(ptr, len);
            const char *end = next;
            while (*end != '}')
                end++;
            ptr = ++end;
            builder << segment << t;
        }

        template <typename ...Args>
        void error(SourceLocation loc, const std::string &fmt, Args... args){
            std::ostringstream builder = {};
            builder << loc.line << ":" << loc.col << ": ";
            
            const char *ptr = fmt.c_str();
            
            if constexpr (sizeof...(args) > 0)
                (error_helper(builder, ptr, args), ...);
            else
                builder << fmt;

            std::cerr << builder.str() << std::endl;
            exit(EXIT_FAILURE);
        }
    };
}

#endif//__PARSER_HPP__
