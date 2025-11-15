#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include "parser.hpp"
#include "backend.hpp"
#ifdef GLASS_USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>

#define init_readline() rl_readline_name = const_cast<char*>("glass");
#define glass_readline(prompt) readline(prompt)
#else
#define init_readline()
char *glass_readline(const char *prompt){
    std::cout << prompt;
    std::string line;
    if (std::getline(std::cin, line)){
        return strdup(line.c_str());
    }
    return NULL;
}
#endif

std::optional<uintptr_t> run_file(glass::VM &vm, const char *filename){
    using namespace glass;
    std::ostringstream file_buffer = {};

    {
        std::ifstream input(filename);
        if (!input){
            std::cerr << "glass: failed to open file " << filename << ": ";
            perror(NULL);
            return {};
        }
        char buffer[1024];
        while (!input.eof()){
            file_buffer.write(buffer, input.read(buffer, sizeof(buffer)).gcount());
        }
    }

    Parser parser(Lexer(file_buffer.str()));
    IRBuilder builder = {};
    while (parser.next_node()) {}
    for (auto &node : parser.nodes){
        builder.feed(node);
    }
    builder.finalize();
    vm.program = std::move(builder.ir);
    vm.pc = builder.symbols.at("main");
    while (!vm.should_exit)
        vm.step();
    return vm.registers[0];
}

int main(int argc, char *argv[]){
    if (argc < 2){
        std::cerr << "usage: glass [-i] FILES..." << std::endl;
        std::cerr << "\t-i\tenables interactive mode (REPL)" << std::endl;
        return EXIT_FAILURE;
    }

    using namespace glass;
    VM vm = {};

    bool i = false;
    bool error = false;
    int code = 0;

    for (int argi = 1; argi < argc; argi++){
        char *arg = argv[argi];
        if (*arg == '-'){
            if (arg[1] == 'i')
                i = true; // enable interactive mode
        } else {
            auto res = run_file(vm, arg);
            if (res.has_value())
                code = res.value();
            else
                error = true;
        }
    }

    if (!i)
        return error ? EXIT_FAILURE : code;
    init_readline();

    std::cout << "Glass v0.0.1 REPL" << std::endl;
    while (i){
        char *line = glass_readline("> ");
        if (!line) break;
        Lexer lexer(line);
        if (lexer.lookahead() == TokenType::EndOfFile) {
            free(line);
            continue;
        }
        if (lexer.lookahead() == TokenType::Identifier || lexer.lookahead() == TokenType::IntLiteral || lexer.lookahead() == TokenType::Minus){
            lexer = Lexer("return " + std::string(line) + ";");
        }
#ifdef GLASS_USE_READLINE
        add_history(line);
#endif
        free(line);
        Parser parser(std::move(lexer));
        IRBuilder builder = {};
        while (parser.next_node()) {}
        for (auto &node : parser.nodes){
            builder.feed(node);
        }
        vm.should_exit = false;
        vm.program = std::move(builder.ir);
        if (builder.symbols.find("main") != builder.symbols.cend())
            vm.pc = builder.symbols.at("main");
        else {
            vm.program.push_back(Instruction {
                .type = InstructionType::Halt
            });
            vm.pc = 0;
        }
        while (!vm.should_exit)
            vm.step();
        std::cout << "$ " << vm.registers[0] << std::endl;
    }
    return EXIT_SUCCESS;
}
