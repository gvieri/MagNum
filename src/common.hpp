#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <cstdint>
#include <map>

#include "types/number.hpp"
#include "types/boolean.hpp"
#include "types/string.hpp"
#include "types/void.hpp"

class Chunk; class Value;

struct Function {
    std::string name;
    int parameters = 0;
    Chunk* chunk; 
};

typedef Value (*Native)(Value* arguments, int parameters);

typedef enum {
    VAL_NUMBER,
    VAL_BOOLEAN,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_NATIVE,
    VAL_VOID
} Values;

class Value {
    public:
        struct {
            Number number;
            Boolean boolean;
            String string;

            Function function;
            Native native;

            Void null = Void::VOID();
        } value;

        Value() = default;

        Value(const Number &number) { this->memorized = VAL_NUMBER; this->value.number = number; }
        Value(const Boolean &boolean) { this->memorized = VAL_BOOLEAN; this->value.boolean = boolean; }
        Value(const String &string) { this->memorized = VAL_STRING; this->value.string = string; }

        Value(const Function &function) { this->memorized = VAL_FUNCTION; this->value.function = function; }
        Value(const Native &native) { this->memorized = VAL_NATIVE; this->value.native = native; }

        Value(const Void &nulled) { this->memorized = VAL_VOID; }

        Values index() { return this->memorized; }

    private:
        Values memorized = VAL_VOID;
};

class Chunk {
    public:
        std::vector<uint8_t> codes;
        std::vector<int> lines;
        std::vector<Value> values;

        void write(uint8_t byte, int line) {
            this->codes.push_back(byte);
            this->lines.push_back(line);
        }

        int add(Value value) {
            this->values.push_back(value);
            return this->values.size() - 1;
        }
};

enum Operations {
    OP_CONSTANT,  
    OP_TRUE, OP_FALSE,
    OP_VOID,
    OP_PLUS, OP_MINUS,
    OP_ADD, OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE, OP_MOD,
    OP_CONCATENATE,
    OP_AND, OP_OR, OP_NOT,
    OP_EQUALS, OP_NOT_EQUALS,
    OP_GREATER, OP_GREATER_EQUALS, OP_LESS, OP_LESS_EQUALS,
    OP_PRINT, OP_INPUT,
    OP_DECLARATION, 
    OP_VARIABLE, OP_LOCAL,
    OP_ASSIGNMENT_VARIABLE, OP_ASSIGNMENT_LOCAL, 
    OP_CONDITION, OP_JUMP, OP_LOOP,
    OP_CALL, OP_RETURN,
    OP_EMPTY, OP_POP, OP_EXIT
};

typedef enum {
    OPERANDS_ERROR,
    DECLARATION_ERROR,
    ASSIGNMENT_ERROR,
    CONDITION_ERROR,
    FUNCTION_ERROR,
    ARGUMENTS_ERROR,
    STACK_OVERFLOW_ERROR,
    RETURN_ERROR
} RUN_TIME_ERROR;

const std::map<RUN_TIME_ERROR, std::string> run_time_error = {
    { OPERANDS_ERROR, "The types of the operands does not match with the operator" },
    { DECLARATION_ERROR, "A variable or a function with the same identifier already exists" },
    { ASSIGNMENT_ERROR, "The referred variable or function has not been initialized yet" },
    { CONDITION_ERROR, "The condition must return a boolean" },
    { FUNCTION_ERROR, "The call must invoke a function" },
    { ARGUMENTS_ERROR, "The number of arguments passed does not match with the function ones" },
    { STACK_OVERFLOW_ERROR, "Function stack-overflow" },
    { RETURN_ERROR, "Can return only from function's bodies" }
};

typedef enum {
    SYNTAX_ERROR,
    CONSTANT_ERROR,
    LOOP_ERROR,
    JUMP_ERROR,
    EXPRESSION_ERROR,
    PARAMETER_ERROR,
    BLOCK_ERROR,
    IDENTIFIER_ERROR,
    SCOPE_ERROR,
    DEFINE_ERROR,
    EXPECTED_FUNCTION_ERROR,
    EXPECTED_PARAMETER_ERROR
} COMPILE_TIME_ERROR;

const std::map<COMPILE_TIME_ERROR, std::string> compile_time_error = {
    { SYNTAX_ERROR, "General syntax error" },
    { CONSTANT_ERROR, "Too many constants in one chunk" },
    { LOOP_ERROR, "The body of the loop contains too many instructions" },
    { JUMP_ERROR, "Too much statements to jump over" },
    { EXPRESSION_ERROR, "Expected an expression" },
    { PARAMETER_ERROR, "Expected end of arguments block" },
    { BLOCK_ERROR, "Expected the end of a block" },
    { IDENTIFIER_ERROR, "Expected an identifier" },
    { SCOPE_ERROR, "The identifier is already used" },
    { DEFINE_ERROR, "Functions can be defined only in global scope" },
    { EXPECTED_FUNCTION_ERROR, "Expected a function identifier" },
    { EXPECTED_PARAMETER_ERROR, "Expected a parameter identifier" }
};

#endif