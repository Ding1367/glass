#ifndef __BACKEND_HPP__
#define __BACKEND_HPP__
#include <memory>
#include <vector>
#include <variant>
#include <stdint.h>
#include <unordered_map>

namespace glass {
    class ASTNode;
    class ExprNode;

    enum class InstructionType : unsigned char {
        Push, // does not push a value, it decrements the sp
        Pop, // opposite
        AddrStack, // gets the base pointer of the stack - a number of bytes
        
        // load & store operations
        LoadByte,
        LoadHalf,
        LoadWord,
        LoadLong,
        LoadPtr,
        LoadImm,

        StrByte,
        StrHalf,
        StrWord,
        StrLong,
        StrPtr,

        Return,

        // operations
        Add, Sub, Mul, Div,

        Halt, // interactive only for REPL so it can exit without modifying stack

        // hints
        Symbol
    };

    struct Instruction {
        InstructionType type;

        struct R {
            unsigned char dst;
            unsigned char src;
        };
        struct I {
            unsigned char dst;
            uintptr_t value;
        };
        struct M {
            unsigned char dst;
            unsigned char base;
            uintptr_t index;
        };
        struct empty {};

        std::variant<R, I, M, empty, std::string> data = empty {};
    };

    class IRBuilder {
    public:
        std::vector<Instruction> ir = {};
        std::unordered_map<std::string, int> symbols = {};

        void feed(const std::shared_ptr<ASTNode> &node);
    private:
        bool clobbers[256];

        int find_free(){
            for (int i = 0; i < 256; i++){
                if (!clobbers[i]) return i;
            }
            return -1;
        }

        void reserve(unsigned char r){
            clobbers[r] = true;
        }
        
        void release(unsigned char r){
            clobbers[r] = false;
        }

        void emitSymbol(const std::string &str){
            symbols[str] = ir.size() + 1;
            ir.push_back(Instruction { .type = InstructionType::Symbol, .data = str });
        }
        void emitEmpty(InstructionType type){
            ir.push_back(Instruction { .type = type, .data = Instruction::empty {} });
        }
        void emitImm(InstructionType type, unsigned char dst, uintptr_t value){
            ir.push_back(Instruction { .type = type, .data = Instruction::I { .dst = dst, .value = value }});
        }
        void emit(InstructionType type, unsigned char dst, unsigned char src){
            ir.push_back(Instruction { .type = type, .data = Instruction::R { .dst = dst, .src = src } });
        }

        void loadExpr(const std::shared_ptr<ExprNode> &expr, unsigned char reg);
    };

    // this is literally a VM.
    class VM {
    public:
        std::vector<Instruction> program = {};
        int pc = 0;
        bool should_exit = false;
        char *stack;
        int sp;
        int base;
        uintptr_t registers[256];

        VM(){
            stack = new char[65536];
            reset();
        }

        void reset(){
            base = sp = 65536;
            pc = 0;
            should_exit = false;
            push_stack<int>(0);
        }

        ~VM(){
            delete[] stack;
        }

        void step();

        template <typename T>
        T get_stack(int offset){
            return *(T*)&stack[sp - offset];
        }

        template <typename T>
        T pop_stack(){
            T value = get_stack<T>(0);
            sp -= sizeof(T);
            return value;
        }

        template <typename T>
        void push_stack(T value){
            sp -= sizeof(T);
            *(T*)&stack[sp] = value;
        }
    };
}

#endif//__BACKEND_HPP__
