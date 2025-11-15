#include "parser.hpp"

namespace glass {
    bool Parser::next_node() {
        Token tok = lex.lookahead();
        if (tok == TokenType::EndOfFile) return false;
        if (tok == TokenType::FuncKeyword){
            lex.next();
            auto decl = std::make_shared<FuncDeclNode>();
            decl->name = expect(TokenType::Identifier, "identifier");
            expect(TokenType::OpenParentheses, "open parentheses to start parameter list");
            while (!test(TokenType::CloseParentheses)){
                break;
            }
            lex.next();
            decl->block = parse_block_expr();
            decl->block->parent = decl;
            push(decl);
        } else if (tok == TokenType::ReturnKeyword){
            lex.next();
            auto ret_stmt = std::make_shared<RetStmt>();
            ret_stmt->expr = parse_expr();
            ret_stmt->expr->parent = ret_stmt;
            expect(TokenType::Semicolon, "semicolon");
            push(ret_stmt);
        }
        return true;
    }

    struct Precedence {
        int lbp, rbp;
    };

    static Precedence precedence_tbl[5] = {
        {3, 3},
        {10, 10},
        {10, 10},
        {20, 20},
        {20, 20},
    };

    std::shared_ptr<ExprNode> Parser::parse_expr(int min_lbp){
        std::shared_ptr<ExprNode> expr;
        if (test(TokenType::IntLiteral)){
            expr = std::make_shared<LiteralExpr>(lex.next());
        } else if (test(TokenType::Identifier)){
            expr = std::make_shared<IdentExpr>(lex.next());
        } else if (test(TokenType::Minus)){
            auto unary = std::make_shared<UnaryExpr>();
            unary->op = lex.next();
            unary->expr = parse_expr(100);
            unary->expr->parent = unary;
            expr = unary;
        } else if (test(TokenType::OpenCurly)){
            expr = parse_block_expr();
        } else {
            std::cerr << "unexpected token" << std::endl;
            exit(EXIT_FAILURE);
        }
        while (1){
            Token tok = lex.lookahead();
            if (tok == TokenType::OpenParentheses){
                lex.next();
                lex.next();
                auto call = std::make_shared<FuncCallExpr>();
                call->func = expr;
                expr->parent = call;
                call->parameters = {};
                expr = call;
                continue;
            }
            int index = (int)tok.type - (int)TokenType::Equal;
            if (index >= sizeof(precedence_tbl)/sizeof(Precedence) || index < 0) break;
            if (precedence_tbl[index].lbp <= min_lbp) break;
            auto bin = std::make_shared<BinaryExpr>();
            bin->op = lex.next();
            bin->lhs = expr;
            bin->lhs->parent = bin;
            bin->rhs = parse_expr(precedence_tbl[index].rbp);
            bin->rhs->parent = bin;
            expr = bin;
        }

        return expr;
    }

}
