#include <iostream>
#include <fstream>
#include <streambuf>

#include "src/virtual-machine.hpp"

void execute(std::string path) {
    std::ifstream file(path);
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    VM::Interpret result = VM().interpret(source);

    if(result == VM::Interpret::INTERPRET_COMPILE_ERROR) exit(65);
    if(result == VM::Interpret::INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, char* argv[]) { if(argc == 2) { std::string file = std::string(argv[1]); execute(file); } }