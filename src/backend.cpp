#include "backend.hpp"
#include "parser.hpp"

#define get(i, x) std::get<Instruction::x>(i.data)

namespace glass {
    void VM::step(){
#define vm_case(name) case InstructionType::name:
        const Instruction &ins = program[pc++];
        switch (ins.type){

        vm_case(Symbol) { break; }

        vm_case(Halt){
            should_exit = true;
            break;
        }

        vm_case(Push){
            sp -= get(ins, I).value;
            break;
        }

        vm_case(Pop){
            sp += get(ins, I).value;
            break;
        }

        vm_case(AddrStack){
            registers[get(ins, I).dst] = (uintptr_t)&stack[base - get(ins, I).value];
            break;
        }

        vm_case(Return){
            pc = pop_stack<int>();
            if (pc == 0) {
                should_exit = true;
                break;
            }
            base = pop_stack<int>();
            break;
        }

        vm_case(LoadImm){
            registers[get(ins, I).dst] = get(ins, I).value;
            break;
        }

#define load_case(name, type) \
        vm_case(name){ \
            registers[get(ins, M).dst] = *(type*)(registers[get(ins, M).base] + get(ins, M).index); \
            break; \
        }

#define str_case(name, type) \
        vm_case(name){ \
            *(type*)(registers[get(ins, M).base] + get(ins, M).index) = registers[get(ins, M).dst]; \
            break; \
        }

#define op_case(name, op) \
        vm_case(name){ \
            registers[get(ins, R).dst] = registers[get(ins, R).dst] op registers[get(ins, R).src]; \
            break; \
        }

        op_case(Add, +)
        op_case(Sub, -)
        op_case(Div, /)
        op_case(Mul, *)

        load_case(LoadByte, char)
        load_case(LoadHalf, uint16_t)
        load_case(LoadWord, uint32_t)
        load_case(LoadLong, uint64_t)
        load_case(LoadPtr,  uintptr_t)
         str_case(StrByte,  char)
         str_case(StrHalf,  uint16_t)
         str_case(StrWord,  uint32_t)
         str_case(StrLong,  uint64_t)
         str_case(StrPtr,   uintptr_t)
    
#undef load_case
#undef str_case
#undef op_case
    }
#undef vm_case
    }

    void IRBuilder::feed(const std::shared_ptr<ASTNode> &node){
        if (auto funcDecl = std::dynamic_pointer_cast<FuncDeclNode>(node)){
            emitSymbol(funcDecl->name.value);
            for (auto &node : funcDecl->block->nodes)
                feed(node);
        }
        if (auto retStmt = std::dynamic_pointer_cast<RetStmt>(node)){
            reserve(0);
            loadExpr(retStmt->expr, 0);
            release(0);
            emitEmpty(InstructionType::Return);
        }
    }

    void IRBuilder::loadExpr(const std::shared_ptr<ExprNode> &expr, unsigned char reg){
        if (auto lit = std::dynamic_pointer_cast<LiteralExpr>(expr)){
            emitImm(InstructionType::LoadImm, reg, strtoull(lit->lit.value.c_str(), NULL, 10));
        } else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)){
            int rhs_reg = find_free();
            reserve(rhs_reg);
            loadExpr(bin->lhs, reg);
            loadExpr(bin->rhs, rhs_reg);
            release(rhs_reg);
            if (bin->op.type == TokenType::Plus){
                emit(InstructionType::Add, reg, rhs_reg);
            } else if (bin->op.type == TokenType::Minus){
                emit(InstructionType::Sub, reg, rhs_reg);
            } else if (bin->op.type == TokenType::Slash){
                emit(InstructionType::Div, reg, rhs_reg);
            } else if (bin->op.type == TokenType::Asterisk){
                emit(InstructionType::Mul, reg, rhs_reg);
            }
        }
    }

}
