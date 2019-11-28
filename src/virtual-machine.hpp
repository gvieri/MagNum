#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <cstdint>

#include "common.hpp"
#include "compiler.hpp"
#include "symbol-table.hpp"
#include "standard-library.hpp"

#define FRAME_MAX 1000                       
#define STACK_MAX 1024

template<class TYPE>
class Stack {
    public:
        TYPE stack[STACK_MAX], *topper;

        Stack() { this->topper = this->stack; }

        void push(TYPE value) { *this->topper = value; this->topper++; }

        TYPE pop() { this->topper--; return *this->topper; }
        TYPE top() { return this->topper[-1]; }
        TYPE peek(int index) { return this->topper[- 1 - index]; }

        void remove(int index) {
            for(int i = -1; i < index - 1; i++)
                this->topper[i - index] = this->topper[i - index + 1];

            this->topper--;
        }
}; 

typedef struct {
    Function* function;
    uint8_t* pc; 
    Value* slots;
} Frame;

class VM {
    public:
        typedef enum {            
            INTERPRET_OK,           
            INTERPRET_COMPILE_ERROR,
            INTERPRET_RUNTIME_ERROR 
        } Interpret; 

        VM() {
            this->table.insert("number", number);
            this->table.insert("string", string);
            this->table.insert("length", length);
        }

        Interpret interpret(std::string source) {
            Function* function = this->parser.compile(source);
            if(function == NULL) return INTERPRET_COMPILE_ERROR;
            
            Frame* frame = &this->frames[this->count++];      
            frame->function = function;                          
            frame->pc = &(function->chunk->codes.front());                    
            frame->slots = this->stack.stack;
            
            return this->run();
        }

    private:
        Frame frames[FRAME_MAX]; int count = 0;
        Parser parser; Stack<Value> stack;
        Table table;

        Interpret run() {   
            Frame* frame = &this->frames[this->count - 1];

            #define READ_BYTE() (*frame->pc++)
            #define READ_SHORT() (frame->pc += 2, (uint16_t)((frame->pc[-2] << 8) | frame->pc[-1]))
            #define READ_CONSTANT() (frame->function->chunk->values[READ_BYTE()])

            #define OPERATOR(type, tag) \
                    type right = this->stack.pop().value.tag; \
                    type left = this->stack.pop().value.tag; \

            #define BINARY_OP(type, tag, operator) do { \
                    OPERATOR(type, tag); \
                    this->stack.push(left operator right); \
                } while(false) 

            #define COMPARISON_OP(type, tag, operator) do { \
                    OPERATOR(type, tag); \
                    Boolean result = left operator right; \
                    this->stack.push(result); \
                } while(false)

            while(true) {       
                uint8_t instruction = READ_BYTE();

                switch(instruction) {
                    case OP_CONSTANT: {
                        Value constant = READ_CONSTANT();
                        this->stack.push(constant);                     
                        break;
                    }

                    case OP_TRUE: this->stack.push(Boolean::TRUE()); break; case OP_FALSE: this->stack.push(Boolean::FALSE()); break;

                    case OP_VOID: this->stack.push(Void::VOID()); break;

                    case OP_PLUS:  
                        if(this->stack.top().index() != VAL_NUMBER && this->stack.top().index() != VAL_STRING) return this->runtime(OPERANDS_ERROR); else break;
                    
                    case OP_MINUS: { 
                        if(this->stack.top().index() == VAL_NUMBER) { Number value = this->stack.pop().value.number; this->stack.push(-value); break; }
                        if(this->stack.top().index() == VAL_STRING) { String value = this->stack.pop().value.string; this->stack.push(-value); break; } 

                        return this->runtime(OPERANDS_ERROR);
                    }

                    case OP_ADD: {  
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) { BINARY_OP(Number, number, +); break; }
                        if(this->stack.top().index() == VAL_STRING && this->stack.peek(1).index() == VAL_STRING) { BINARY_OP(String, string, +); break; }

                        return this->runtime(OPERANDS_ERROR);
                    }

                    case OP_SUBTRACT:  
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) BINARY_OP(Number, number, -);
                        else return this->runtime(OPERANDS_ERROR); break;
                    
                    case OP_MULTIPLY:   
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) BINARY_OP(Number, number, *); 
                        else return this->runtime(OPERANDS_ERROR); break;

                    case OP_DIVIDE:   
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) BINARY_OP(Number, number, /); 
                        else return this->runtime(OPERANDS_ERROR); break;
                    
                    case OP_MOD: 
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) BINARY_OP(Number, number, %);
                        else return this->runtime(OPERANDS_ERROR); break;  
                    
                    case OP_CONCATENATE:
                        if(this->stack.top().index() == VAL_STRING && this->stack.peek(1).index() == VAL_STRING) {
                            String right = this->stack.pop().value.string;
                            String left = this->stack.pop().value.string;
                            this->stack.push(String::CONCATENATE(left, right));
                        }
                        else return this->runtime(OPERANDS_ERROR); break;  

                    case OP_AND:
                        if(this->stack.top().index() == VAL_BOOLEAN && this->stack.peek(1).index() == VAL_BOOLEAN) BINARY_OP(Boolean, boolean, &&);
                        else return this->runtime(OPERANDS_ERROR); break; 

                    case OP_OR:
                        if(this->stack.top().index() == VAL_BOOLEAN && this->stack.peek(1).index() == VAL_BOOLEAN) BINARY_OP(Boolean, boolean, ||);
                        else return this->runtime(OPERANDS_ERROR); break; 

                    case OP_NOT:
                        if(this->stack.top().index() == VAL_BOOLEAN) { Boolean value = this->stack.pop().value.boolean; this->stack.push(!value); } 
                        else return this->runtime(OPERANDS_ERROR); 
                        
                        break;

                    case OP_EQUALS: {
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) { COMPARISON_OP(Number, number, ==); break; }
                        if(this->stack.top().index() == VAL_BOOLEAN && this->stack.peek(1).index() == VAL_BOOLEAN) { COMPARISON_OP(Boolean, boolean, ==); break; }
                        if(this->stack.top().index() == VAL_STRING && this->stack.peek(1).index() == VAL_STRING) { COMPARISON_OP(String, string, ==); break; }
                        if(this->stack.top().index() == VAL_VOID && this->stack.peek(1).index() == VAL_VOID) { COMPARISON_OP(Void, null, ==); break; }

                        this->stack.pop(); this->stack.pop();
                        this->stack.push(Boolean::FALSE()); break;
                    }

                    case OP_NOT_EQUALS: {
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) { COMPARISON_OP(Number, number, !=); break; }
                        if(this->stack.top().index() == VAL_BOOLEAN && this->stack.peek(1).index() == VAL_BOOLEAN) { COMPARISON_OP(Boolean, boolean, !=); break; }
                        if(this->stack.top().index() == VAL_STRING && this->stack.peek(1).index() == VAL_STRING) { COMPARISON_OP(String, string, !=); break; }
                        if(this->stack.top().index() == VAL_VOID && this->stack.peek(1).index() == VAL_VOID) { COMPARISON_OP(Void, null, !=); break; }

                        this->stack.pop(); this->stack.pop();
                        this->stack.push(Boolean::TRUE()); break;
                    }

                    case OP_GREATER: {
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) { COMPARISON_OP(Number, number, >); break; }
                        if(this->stack.top().index() == VAL_STRING && this->stack.peek(1).index() == VAL_STRING) { COMPARISON_OP(String, string, >); break; }

                        return this->runtime(OPERANDS_ERROR);
                    }

                    case OP_LESS: {
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) { COMPARISON_OP(Number, number, <); break; }
                        if(this->stack.top().index() == VAL_STRING && this->stack.peek(1).index() == VAL_STRING) { COMPARISON_OP(String, string, <); break; }

                        return this->runtime(OPERANDS_ERROR);
                    }

                     case OP_GREATER_EQUALS: {
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) { COMPARISON_OP(Number, number, >=); break; }
                        if(this->stack.top().index() == VAL_STRING && this->stack.peek(1).index() == VAL_STRING) { COMPARISON_OP(String, string, >=); break; }

                        return this->runtime(OPERANDS_ERROR);
                    }

                    case OP_LESS_EQUALS: {
                        if(this->stack.top().index() == VAL_NUMBER && this->stack.peek(1).index() == VAL_NUMBER) { COMPARISON_OP(Number, number, <=); break; }
                        if(this->stack.top().index() == VAL_STRING && this->stack.peek(1).index() == VAL_STRING) { COMPARISON_OP(String, string, <=); break; }

                        return this->runtime(OPERANDS_ERROR);
                    }

                    case OP_PRINT: {  
                        switch(this->stack.top().index()) {
                            case VAL_NUMBER: { Number number = this->stack.pop().value.number; printf("%s\n", number.get().c_str()); break; }
                            case VAL_BOOLEAN: { Boolean boolean = this->stack.pop().value.boolean; printf("%s\n", boolean.get().c_str()); break; }
                            case VAL_STRING: { String string = this->stack.pop().value.string; printf("%s\n", string.get().c_str()); break; }

                            case VAL_FUNCTION: { Function function = this->stack.pop().value.function; printf("<Function object: `%s`>\n", function.name.c_str()); break; }
                            case VAL_NATIVE: {  this->stack.pop(); printf("<Standard library function>\n"); break;}

                            case VAL_VOID: { this->stack.pop(); printf("%s\n", Void::VOID().get().c_str()); break; }
                        }

                        break; 
                    }

                    case OP_INPUT: {
                        std::string data; std::cin >> data;
                        String input = data;
                        this->stack.push(input); 
                        break;
                    }

                    case OP_DECLARATION: {
                        std::string identifier = READ_CONSTANT().value.string.get();
                        if(this->table.insert(identifier, this->stack.pop()) == false) return this->runtime(DECLARATION_ERROR);
                        break;
                    }

                    case OP_VARIABLE: {
                        std::string identifier = READ_CONSTANT().value.string.get();
                        Value value = this->table.get(identifier);
                        this->stack.push(value);
                        break;
                    }

                    case OP_ASSIGNMENT_VARIABLE: {
                        std::string identifier = READ_CONSTANT().value.string.get();
                        if(this->table.set(identifier, this->stack.top()) == false) return this->runtime(ASSIGNMENT_ERROR);
                        break;
                    }

                    case OP_LOCAL: {
                        uint8_t slot = READ_BYTE();    
                        this->stack.push(frame->slots[slot]);
                        break;
                    }

                    case OP_ASSIGNMENT_LOCAL: {
                        uint8_t slot = READ_BYTE();
                        frame->slots[slot] = this->stack.top();  
                        break;
                    }

                    case OP_CONDITION: {
                        uint16_t offset = READ_SHORT();

                        if(this->stack.top().index() == VAL_BOOLEAN) {
                            Boolean condition = this->stack.top().value.boolean;
                            if(!condition.boolean) frame->pc += offset;
                            break;
                        }

                        return this->runtime(CONDITION_ERROR);
                    }

                    case OP_JUMP: {
                        uint16_t offset = READ_SHORT();
                        frame->pc += offset;               
                        break;
                    }

                    case OP_LOOP: {
                        uint16_t offset = READ_SHORT();
                        frame->pc -= offset;                                       
                        break;
                    }

                    case OP_CALL: {
                        if(this->count == FRAME_MAX) return this->runtime(STACK_OVERFLOW_ERROR);

                        int arguments = READ_BYTE();
                        
                        Value function = this->stack.peek(arguments);

                        if(this->call(function, arguments) == false) return INTERPRET_RUNTIME_ERROR;

                        frame = &this->frames[this->count - 1];
                        
                        break;
                    }

                    case OP_RETURN: {        
                        if(this->count == 1) return this->runtime(RETURN_ERROR);

                        Value returned = this->stack.pop();
                        
                        this->count--;                             

                        this->stack.topper = frame->slots;        
                        this->stack.push(returned);                         

                        frame = &this->frames[this->count - 1];

                        break;
                    }

                    case OP_POP: 
                        this->stack.pop(); 
                        break;

                    case OP_EXIT: return INTERPRET_OK;            
                }                                   
            }                                     

            #undef READ_BYTE
            #undef READ_SHORT
            #undef READ_CONSTANT
            #undef OPERATOR
            #undef BINARY_OP
            #undef COMPARISON_OP    
        }   

        bool call(Value constant, int arguments) {
            switch(constant.index()) {
                case VAL_FUNCTION: {
                    Function* function = &constant.value.function;

                    if(function->parameters != arguments) { this->runtime(ARGUMENTS_ERROR); return false; } 

                    Frame* frame = &this->frames[this->count++];      
                    frame->function = function;       
                    frame->pc = &(function->chunk->codes.front());                    
                    frame->slots = this->stack.topper - arguments - 1;

                    this->stack.remove(arguments);

                    return true;
                }

                case VAL_NATIVE: {
                    Native native = constant.value.native;

                    Value result = native(this->stack.topper - arguments, arguments);
                    this->stack.topper -= arguments + 1;                            
                    this->stack.push(result);

                    return true;
                }
            }

            this->runtime(FUNCTION_ERROR); return false;
        }

        Interpret runtime(RUN_TIME_ERROR error) {
            std::string message = run_time_error.find(error)->second;

            for(int i = this->count - 1; i >= 0; i--) {                 
                Frame* frame = &this->frames[i];                            
                Function* function = frame->function;                                                                   
                size_t instruction = frame->pc - &(function->chunk->codes.front()) - 1;   
                fprintf(stderr, "[line %d] ", function->chunk->lines[instruction]);  

                if(i == 0) fprintf(stderr, "RUN-TIME ERROR in script: ");                                                                                  
                else fprintf(stderr, "in `%s` ->\n", function->name.c_str());
            } 

            fprintf(stderr, "%s", message.c_str());

            return INTERPRET_RUNTIME_ERROR;
        }
};

#endif