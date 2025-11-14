#include <iostream>
#include <sstream>
#include <fstream>
#include "parser.hpp"
#include "backend.hpp"

int main(){
    using namespace glass;

    std::string line;
    while (true){
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        Lexer lexer(line);
        Parser parser(std::move(lexer));
        IRBuilder builder = {};
        while (parser.next_node()) {}
        for (auto &node : parser.nodes){
            builder.feed(node);
        }
        VM vm = {};
        vm.program = std::move(builder.ir);
        if (builder.symbols.find("main") != builder.symbols.cend())
            vm.pc = builder.symbols.at("main");
        else {
            vm.program.push_back(Instruction {
                .type = InstructionType::Halt
            });
        }
        while (!vm.should_exit)
            vm.step();
        std::cout << "$ " << vm.registers[0] << std::endl;
    }
    return EXIT_SUCCESS;
}
