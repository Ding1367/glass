#ifndef __BACKEND_HPP__
#define __BACKEND_HPP__
#include <memory>
#include <vector>
#include <variant>
#include <stdint.h>
#include <unordered_map>
#include <string>

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

        // operations
        Add, Sub, Mul, Div,

        Halt, // interactive only for REPL so it can exit without modifying stack

        // control flow
        Call,
        Return,

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
        struct B {
            unsigned char cond;
            uintptr_t new_pc;
        };
        struct empty {};

        std::variant<R, I, M, B, empty, std::string> data = empty {};
    };

    class IRBuilder {
    public:
        std::vector<Instruction> ir = {};
        std::unordered_map<std::string, int> symbols = {};

        void feed(const std::shared_ptr<ASTNode> &node);
        void finalize(){
            for (const Pending &pending : pending_list){
                Instruction &ins = ir[pending.pos];
                auto &data = ins.data;
                // I?
                if (data.index() == 1){
                    std::get<Instruction::I>(data).value = symbols[pending.what];
                } else if (data.index() == 3) { // B?
                    std::get<Instruction::B>(data).new_pc = symbols[pending.what];
                }
            }
        }
    private:
        bool clobbers[256];

        struct Pending {
            uintptr_t pos;
            std::string what;
        };

        std::vector<Pending> pending_list = {};

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
        void emitImm(InstructionType type, unsigned char dst, const std::string &value){
            ir.push_back(Instruction { .type = type, .data = Instruction::I { .dst = dst, .value = 0 } });
            pending_list.push_back(Pending { .pos = ir.size() - 1, .what = value });
        }
        void emit(InstructionType type, unsigned char dst, unsigned char src){
            ir.push_back(Instruction { .type = type, .data = Instruction::R { .dst = dst, .src = src } });
        }
        void emitCtrl(InstructionType type, uintptr_t value, unsigned char cond = 255){
            ir.push_back(Instruction { .type = type, .data = Instruction::B { .cond = cond, .new_pc = value } });
        }
        void emitCtrl(InstructionType type, const std::string &value, unsigned char cond = 255){
            ir.push_back(Instruction { .type = type, .data = Instruction::B { .cond = cond, .new_pc = 0 } });
            pending_list.push_back(Pending { .pos = ir.size() - 1, .what = value });
        }


        uintptr_t loadExpr(const std::shared_ptr<ExprNode> &expr, int reg);
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
