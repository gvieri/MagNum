#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <cstdlib>
#include <cstdint>

#include "common.hpp"
#include "virtual-machine.hpp" 
#include "tokenizer.hpp"

#define GLOBAL_SCOPE 0

class Compiler {
    public:
        Function* function = new Function();

        typedef struct {
            Token local;
            int depth = 0;
        } Local;

        Local locals[UINT8_MAX + 1];
        int count = 0, depths = 0;

        Compiler() { this->function->chunk = new Chunk(); }

        ~Compiler() { delete function; }

        void add(Token variable) {
            Local* local = &this->locals[this->count++];
            local->local = variable;                                    
            local->depth = this->depths; 
        }
};

enum Precedences {                  
    PREC_NONE,                    
    PREC_ASSIGNMENT,       
    PREC_OR, PREC_AND,           
    PREC_EQUALITY, PREC_COMPARISON, 
    PREC_TERM, PREC_FACTOR,          
    PREC_UNARY,          
    PREC_CALL,       
    PREC_PRIMARY                  
};

enum Functions {
    FUNC_NULL,
    FUNC_NUMBER, FUNC_STRING, FUNC_LITELAR,
    FUNC_VARIABLE,
    FUNC_UNARY, FUNC_BINARY,
    FUNC_GROUPING, FUNC_INPUT,
    FUNC_CALL
};

typedef struct {        
    Functions prefix, infix;        
    Precedences precedence;
    Types type;
} Rule; 

Rule rules[TOKEN_NUMBER] = {
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, END_OF_FILE },
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, END_OF_LINE },    
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, ERROR },       
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, EMPTY },  
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, EXIT },          

    { FUNC_VARIABLE,     FUNC_NULL,     PREC_NONE, IDENTIFIER },     

    { FUNC_NUMBER,   FUNC_NULL,     PREC_NONE, NUMBER },       
    { FUNC_STRING,   FUNC_NULL,     PREC_NONE, STRING },             
    { FUNC_LITELAR,  FUNC_NULL,     PREC_NONE, VOID },      

    { FUNC_LITELAR,  FUNC_NULL,     PREC_NONE, BOOLEAN_TRUE },                                      
    { FUNC_LITELAR,  FUNC_NULL,     PREC_NONE, BOOLEAN_FALSE },       

    { FUNC_UNARY,    FUNC_BINARY,   PREC_TERM, PLUS },                         
    { FUNC_UNARY,    FUNC_BINARY,   PREC_TERM, MINUS },       
    { FUNC_NULL,     FUNC_BINARY,   PREC_FACTOR, ASTERISK },        
    { FUNC_NULL,     FUNC_BINARY,   PREC_FACTOR, SLASH },    
    { FUNC_NULL,     FUNC_BINARY,   PREC_FACTOR, MOD },

    { FUNC_NULL,     FUNC_BINARY,   PREC_TERM, CONCATENATE },      

    { FUNC_NULL,     FUNC_BINARY,   PREC_AND, AND },        
    { FUNC_NULL,     FUNC_BINARY,   PREC_OR, OR },      
    { FUNC_UNARY,    FUNC_NULL,     PREC_TERM, NOT },        

    { FUNC_NULL,     FUNC_BINARY,   PREC_EQUALITY, EQUALS },      
    { FUNC_NULL,     FUNC_BINARY,   PREC_EQUALITY, NOT_EQUALS },     

    { FUNC_NULL,     FUNC_BINARY,   PREC_COMPARISON, GREATER },       
    { FUNC_NULL,     FUNC_BINARY,   PREC_COMPARISON, LESS },             
    { FUNC_NULL,     FUNC_BINARY,   PREC_COMPARISON, GREATER_EQUALS },                
    { FUNC_NULL,     FUNC_BINARY,   PREC_COMPARISON, LESS_EQUALS },      

    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, COMMA },      
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, COLON },     

    { FUNC_GROUPING, FUNC_CALL,     PREC_CALL, OPEN_PARENTHESES },       
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, CLOSE_PARENTHESES },      

    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, OPEN_BRACES },     
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, CLOSE_BRACES },      
                                        
    { FUNC_NULL,     FUNC_NULL,     PREC_ASSIGNMENT, ASSIGN },       
    { FUNC_NULL,     FUNC_NULL,     PREC_ASSIGNMENT, PLUS_ASSIGN },     
    { FUNC_NULL,     FUNC_NULL,     PREC_ASSIGNMENT, MINUS_ASSIGN },      
    { FUNC_NULL,     FUNC_NULL,     PREC_ASSIGNMENT, ASTERISK_ASSIGN },   
    { FUNC_NULL,     FUNC_NULL,     PREC_ASSIGNMENT, SLASH_ASSIGN }, 
    { FUNC_NULL,     FUNC_NULL,     PREC_ASSIGNMENT, MOD_ASSIGN },     

    { FUNC_NULL,     FUNC_NULL,     PREC_ASSIGNMENT, INCREMENT }, 
    { FUNC_NULL,     FUNC_NULL,     PREC_ASSIGNMENT, DECREMENT },     

    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, SET },      
    { FUNC_INPUT,     FUNC_NULL,     PREC_PRIMARY, GET },                      
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, PRINT },        
                                    
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, IF },      
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, ELSE },        
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, WHILE },       
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, FOR },                                         

    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, DEFINE },   
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, INVOKE },         
    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, RETURN },       

    { FUNC_NULL,     FUNC_NULL,     PREC_NONE, SEMICOLON },      
};

class Parser {
    public:
        Function* compile(std::string source) {
            this->tokenizer = new Tokenizer(source);
            this->compiler = new Compiler();
            
            this->error = false; this->panic = false;

            this->advance();
 
            while(!this->match(END_OF_FILE) && this->error == false) this->statement();

            this->emit_byte(OP_EXIT);

            return (this->error) ? NULL : this->compiler->function;
        }

        ~Parser() { delete this->tokenizer; delete this->compiler; }

    private:
        Tokenizer* tokenizer; Compiler* compiler;
        Token current, previous;
        bool error, panic, assignment;

        void problem(Token* token, COMPILE_TIME_ERROR error) {
            if(this->panic) return; 

            std::string message = compile_time_error.find(error)->second;
            this->panic = true; this->error = true;

            if(token->type == ERROR) fprintf(stderr, "[line %d] SYNTAX ERROR in script: %s\n", token->line, token->content.c_str());
            if(token->type != ERROR) fprintf(stderr, "[line %d] COMPILE-TIME ERROR in script: %s\n", token->line, message.c_str());
        }

        void advance() {
            this->previous = this->current;

            while(true) {
                this->current = this->tokenizer->next();
                if(this->current.type != ERROR) break;

                this->problem(&this->current, SYNTAX_ERROR); 
                break;
            }
        }

        void consume(Types type, COMPILE_TIME_ERROR error = SYNTAX_ERROR) {
            if(this->current.type == type) this->advance();
            else this->problem(&this->current, error);
        }

        bool match(Types type) {
            if(this->check(type) == false) return false;
            this->advance();
            return true;
        }

        bool check(Types type) { return this->current.type == type; }

        uint8_t constant(Value value) {
            int constant = this->compiler->function->chunk->add(value);

            if (constant > UINT8_MAX) {                       
                this->problem(&this->previous, CONSTANT_ERROR);      
                return 0;                                       
            }

            return (uint8_t)constant; 
        }

        void emit_byte(uint8_t byte) { this->compiler->function->chunk->write(byte, this->previous.line); } void emit_constant(Value value) { emit_byte(OP_CONSTANT); emit_byte(constant(value)); }

        int emit_jump(uint8_t instruction) { 
            this->emit_byte(instruction);
            this->emit_byte(0xff); this->emit_byte(0xff);
            return this->compiler->function->chunk->codes.size() - 2;
        }

        void emit_loop(int start) {                    
            this->emit_byte(OP_LOOP);

            int offset = this->compiler->function->chunk->codes.size() - start + 2;    
            if (offset > UINT16_MAX) this->problem(&this->current, LOOP_ERROR);

            this->emit_byte((offset >> 8) & 0xff); this->emit_byte(offset & 0xff);                               
        }   

        void patch(int offset) {
            int jump = this->compiler->function->chunk->codes.size() - offset - 2;
            if(jump > UINT16_MAX) this->problem(&this->current, JUMP_ERROR);

            this->compiler->function->chunk->codes[offset] = (jump >> 8) & 0xff;
            this->compiler->function->chunk->codes[offset + 1] = jump & 0xff;
        }

        void precedence(Precedences pre) {
            this->advance();

            Functions prefix = rules[this->previous.type].prefix;

            if (prefix == FUNC_NULL) {                               
                this->problem(&this->previous, EXPRESSION_ERROR);                             
                return;                                                  
            }

            this->assignment = pre <= PREC_ASSIGNMENT;   
            casing(prefix); 

            while(pre <= rules[this->current.type].precedence) {
                this->advance();
                Functions infix = rules[this->previous.type].infix;     
                casing(infix);  
            }
        }

        void casing(Functions func) {
            switch(func) {
                case FUNC_NUMBER: this->number(); break;
                case FUNC_STRING: this->string(); break;
                case FUNC_LITELAR: this->literal(); break;

                case FUNC_VARIABLE: this->variable(); break;

                case FUNC_UNARY: this->unary(); break;
                case FUNC_BINARY: this->binary(); break;
                case FUNC_GROUPING: this->grouping(); break;

                case FUNC_INPUT: this->emit_byte(OP_INPUT); break;

                case FUNC_CALL: this->call(); break;
            }
        }

        void number() { Number value(this->previous.content); this->emit_constant(value); } void string() { String value(this->previous.content); this->emit_constant(value); }

        void literal() {
            switch(this->previous.type) {
                case BOOLEAN_TRUE: this->emit_byte(OP_TRUE); break;
                case BOOLEAN_FALSE: this->emit_byte(OP_FALSE); break;
                case VOID: this->emit_byte(OP_VOID); break;
            }
        }
 
        void variable() {
            #define ASSIGN(scope, type, variable) do { \
                    this->emit_byte(scope); this->emit_byte(variable); \
                    this->expression(); \
                    this->emit_byte(type); \
                    goto ASSIGNMENT; \
                } while(false)

            #define SHORT(scope, type, variable) do { \
                    this->emit_byte(scope); this->emit_byte(variable); \
                    Number value("1"); this->emit_constant(value); \
                    this->emit_byte(type); \
                    goto ASSIGNMENT; \
                } while(false)

            uint8_t get = 0, set = 0;                                
            int variable = this->local(&this->previous);
            
            if (variable != GLOBAL_SCOPE - 1) { get = OP_LOCAL; set = OP_ASSIGNMENT_LOCAL; }
            else {                                             
                String name(this->previous.content); variable = this->constant(name);                   
                get = OP_VARIABLE; set = OP_ASSIGNMENT_VARIABLE;                           
            }   

            if(this->assignment && (this->check(ASSIGN) || 
               this->check(PLUS_ASSIGN) || this->check(MINUS_ASSIGN) || this->check(ASTERISK_ASSIGN) || this->check(SLASH_ASSIGN) || this->check(MOD_ASSIGN) ||
               this->check(INCREMENT) || this->check(DECREMENT))) {
                    if(this->match(ASSIGN)) { this->expression(); goto ASSIGNMENT; }

                    if(this->match(PLUS_ASSIGN)) ASSIGN(get, OP_ADD, variable);
                    if(this->match(MINUS_ASSIGN)) ASSIGN(get, OP_SUBTRACT, variable);
                    if(this->match(ASTERISK_ASSIGN)) ASSIGN(get, OP_MULTIPLY, variable);
                    if(this->match(SLASH_ASSIGN)) ASSIGN(get, OP_DIVIDE, variable);
                    if(this->match(MOD_ASSIGN)) ASSIGN(get, OP_MOD, variable);

                    if(this->match(INCREMENT)) SHORT(get, OP_ADD, variable);
                    if(this->match(DECREMENT)) SHORT(get, OP_SUBTRACT, variable);

                    ASSIGNMENT:
                    this->emit_byte(set); this->emit_byte(variable);
            }
            else { this->emit_byte(get); this->emit_byte(variable); }

            #undef ASSIGN
            #undef SHORT
        }

        int local(Token* variable) {
            for (int i = this->compiler->count - 1; i >= 0; i--) {   
                Compiler::Local* local = &this->compiler->locals[i];                  
                if(variable->content == local->local.content) return i;                                                                                              
            }

            return -1;                                          
        }  

        void unary() {
            Types operation = this->previous.type;                     
            precedence(PREC_UNARY);                                
                    
            switch(operation) {     
                case PLUS: emit_byte(OP_PLUS); break;                   
                case MINUS: emit_byte(OP_MINUS); break;
                case NOT: emit_byte(OP_NOT); break;                    
            }      
        }

        void binary() {
            Types operation = this->previous.type;
                            
            Rule* r = &rules[operation];                 
            precedence((Precedences)(r->precedence + 1));     
                            
            switch(operation) {                                  
                case PLUS: emit_byte(OP_ADD); break;     
                case MINUS: emit_byte(OP_SUBTRACT); break;
                case ASTERISK: emit_byte(OP_MULTIPLY); break;
                case SLASH: emit_byte(OP_DIVIDE); break;
                case MOD: emit_byte(OP_MOD); break;  

                case CONCATENATE: emit_byte(OP_CONCATENATE); break;

                case AND: emit_byte(OP_AND); break; case OR: emit_byte(OP_OR); break;

                case EQUALS: emit_byte(OP_EQUALS); break; case NOT_EQUALS: emit_byte(OP_NOT_EQUALS); break; 

                case GREATER: emit_byte(OP_GREATER); break; case LESS: emit_byte(OP_LESS); break;                        
                case GREATER_EQUALS: emit_byte(OP_GREATER_EQUALS); break; case LESS_EQUALS: emit_byte(OP_LESS_EQUALS); break; 
            }       
        }

        void grouping() {
            this->expression();                                              
            this->consume(CLOSE_PARENTHESES, EXPRESSION_ERROR);    
        }

        void call() {
            uint8_t arguments = 0;

            if(this->check(CLOSE_PARENTHESES) == false) {   
                ARGUMENT:                                                       
                this->expression();                                   
                arguments++;

                if(this->match(COMMA)) goto ARGUMENT;                                                     
            }

            this->consume(CLOSE_PARENTHESES, PARAMETER_ERROR); 
            this->emit_byte(OP_CALL); this->emit_byte(arguments);
        }

        void expression() { this->precedence(PREC_ASSIGNMENT); }

        void statement() {
            Types type = this->current.type;

            switch(type) {
                case PRINT: this->print(); break;
                case SET: this->declaration(); break;
                case DEFINE: this->definition(); break;
                case RETURN: this->returning(); break;
                case OPEN_BRACES: this->block(); break;
                case IF: this->conditional(); break;
                case WHILE: this->iterational(); break;
                case FOR: this->counter(); break;
                case EXIT: this->consume(EXIT); this->emit_byte(OP_EXIT); break;

                case END_OF_LINE: case END_OF_FILE: case EMPTY: this->match(EMPTY); this->emit_byte(OP_EMPTY); break;
                
                default: this->expression(); this->emit_byte(OP_POP); break;
            }

            if(this->match(SEMICOLON) == false) this->match(END_OF_LINE);
        }

        void print() {
            this->consume(PRINT); this->expression();
            this->emit_byte(OP_PRINT);
        }

        void declaration() {
            this->consume(SET);

            VARIABLE:
            this->consume(IDENTIFIER, IDENTIFIER_ERROR);

            if(this->compiler->depths == GLOBAL_SCOPE)  {
                String name(this->previous.content); 
                uint8_t variable = this->constant(name);

                if(this->match(COLON)) this->expression();                                                    
                else this->emit_byte(OP_VOID);

                this->emit_byte(OP_DECLARATION); this->emit_byte(variable);
            } 

            if(this->compiler->depths != GLOBAL_SCOPE) {
                Token* token = &this->previous;

                for(int i = this->compiler->count - 1; i >= 0; i--) {                 
                    Compiler::Local* local = &this->compiler->locals[i];                                
                    if(local->depth != GLOBAL_SCOPE - 1 && local->depth < this->compiler->depths) break;
                    if(token->content == local->local.content) this->problem(&this->current, SCOPE_ERROR);
                }   

                this->compiler->add(*token);

                if(this->match(COLON)) this->expression();                                                    
                else this->emit_byte(OP_VOID);
            }

            if(this->match(COMMA)) goto VARIABLE;                                          
        }

        void definition() {
            if(this->compiler->depths != GLOBAL_SCOPE) { this->problem(&this->current, DEFINE_ERROR); return; }

            this->consume(DEFINE);
            this->consume(IDENTIFIER, EXPECTED_FUNCTION_ERROR); String name(this->previous.content); 
            uint8_t function = this->constant(name);

            Compiler* enclosing = this->compiler; 

            this->compiler = new Compiler();
            this->compiler->function->name = this->previous.content;
            this->compiler->depths++;

            this->parameter();
            
            this->match(END_OF_LINE);

            this->statement();

            this->emit_byte(OP_VOID); this->emit_byte(OP_RETURN);

            Function constant = *(this->compiler->function);

            this->compiler = enclosing;
            
            this->emit_constant(constant);
            this->emit_byte(OP_DECLARATION); this->emit_byte(function); 
        }

        void parameter() {
            if(this->match(OPEN_PARENTHESES)) {
                if(this->match(CLOSE_PARENTHESES) == false) {
                    PARAMETER:

                    this->compiler->function->parameters++;                                     

                    this->consume(IDENTIFIER, EXPECTED_PARAMETER_ERROR);

                    Token* token = &this->previous;

                    for(int i = this->compiler->count - 1; i >= 0; i--) {                 
                        Compiler::Local* local = &this->compiler->locals[i];                                
                        if(local->depth != GLOBAL_SCOPE - 1 && local->depth < this->compiler->depths) break;
                        if(token->content == local->local.content) this->problem(&this->current, SCOPE_ERROR);
                    }   

                    this->compiler->add(*token);

                    if(this->match(COMMA)) goto PARAMETER;
                    this->consume(CLOSE_PARENTHESES, SYNTAX_ERROR);
                }
            }
            else this->consume(COLON, SYNTAX_ERROR); 
        }

        void returning() {
            this->consume(RETURN);

            if(this->check(SEMICOLON) || this->check(END_OF_LINE)) this->emit_byte(OP_VOID);
            else this->expression();

            this->emit_byte(OP_RETURN);
        }

        void block() {
            this->consume(OPEN_BRACES);

            this->compiler->depths++;
            while(!this->check(CLOSE_BRACES) && !this->check(END_OF_FILE) && this->error == false) this->statement();
            this->compiler->depths--;

            while (this->compiler->count > 0 && 
                    this->compiler->locals[this->compiler->count - 1].depth > this->compiler->depths) {                       
                this->emit_byte(OP_POP);                                    
                this->compiler->count--;                               
            }    

            this->consume(CLOSE_BRACES, BLOCK_ERROR);
        }

        void conditional() {
            this->consume(IF);
            this->expression(); 
            this->consume(COLON, SYNTAX_ERROR); this->match(END_OF_LINE);

            int truly = this->emit_jump(OP_CONDITION);
            this->emit_byte(OP_POP);
            this->statement(); 
            this->match(END_OF_LINE);

            int falsey = this->emit_jump(OP_JUMP);

            this->patch(truly); this->emit_byte(OP_POP);

            if(this->match(ELSE)) {
                this->consume(COLON, SYNTAX_ERROR); this->match(END_OF_LINE);
                this->statement();
            }

            this->patch(falsey);
        }

        void iterational() {
            int start = this->compiler->function->chunk->codes.size();

            this->consume(WHILE);
            this->expression();
            this->consume(COLON, SYNTAX_ERROR); this->match(END_OF_LINE);

            int loop = emit_jump(OP_CONDITION);                

            this->emit_byte(OP_POP);                                         
            this->statement();

            this->emit_loop(start);                                         

            this->patch(loop); this->emit_byte(OP_POP);
        }

        void counter() {
            this->compiler->depths++;

            this->consume(FOR); 
            this->consume(OPEN_PARENTHESES, SYNTAX_ERROR);

            switch(this->current.type) {
                case SEMICOLON: break;
                case SET: this->declaration(); break;
                default: this->expression(); this->emit_byte(OP_POP); break;
            }

            this->consume(SEMICOLON, SYNTAX_ERROR); 

            int start = this->compiler->function->chunk->codes.size(), jump = -1;    

            if (this->match(SEMICOLON) == false) {                                 
                this->expression();                                       
                this->consume(SEMICOLON, SYNTAX_ERROR);
          
                jump = this->emit_jump(OP_CONDITION);                       
                this->emit_byte(OP_POP);                              
            }  

            if (this->match(CLOSE_PARENTHESES) == false) {                              
                int body = this->emit_jump(OP_JUMP);

                int increment = this->compiler->function->chunk->codes.size();                 
                this->expression(); this->emit_byte(OP_POP);
                           
                this->consume(CLOSE_PARENTHESES, SYNTAX_ERROR);

                this->emit_loop(start);                                        
                start = increment;                                 
                this->patch(body);                                   
            } 
            
            this->match(END_OF_LINE);
            
            this->statement();

            this->emit_loop(start);

            if (jump != -1) {            
                this->patch(jump);           
                this->emit_byte(OP_POP);
            }  

            this->compiler->depths--;

            while(this->compiler->count > 0 && 
                   this->compiler->locals[this->compiler->count - 1].depth > this->compiler->depths) {                       
                this->emit_byte(OP_POP);                                    
                this->compiler->count--;                               
            }
        }
};

#endif