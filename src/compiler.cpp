#include <iostream>
#include <sstream>
#include <fstream>
#include "parser.hpp"
#include "backend.hpp"

int main(){
    using namespace glass;

    std::ostringstream file_buffer = {};

    {
        std::ifstream input("main.gls");
        if (!input){
            perror("failed to open file main.gls");
            return EXIT_FAILURE;
        }
        char buffer[1024];
        while (!input.eof()){
            file_buffer.write(buffer, input.read(buffer, sizeof(buffer)).gcount());
        }
    }

    Lexer lexer(file_buffer.str());
    Parser parser(std::move(lexer));
    IRBuilder builder = {};
    while (parser.next_node()) {}
    for (auto &node : parser.nodes){
        builder.feed(node);
    }
    builder.finalize();
    VM vm = {};
    vm.program = std::move(builder.ir);
    vm.pc = builder.symbols["main"];
    while (!vm.should_exit)
        vm.step();
    return vm.registers[0];
}
