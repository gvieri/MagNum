#ifndef TOKENIZER_H 
#define TOKENIZER_H

#include <utility> 
#include <map>

#define TOKEN_NUMBER 52

enum Types { 
    END_OF_FILE, END_OF_LINE, ERROR, EMPTY, EXIT,
    IDENTIFIER,
    NUMBER, STRING, VOID,
    BOOLEAN_TRUE, BOOLEAN_FALSE,
    PLUS, MINUS, ASTERISK, SLASH, MOD,
    CONCATENATE,
    AND, OR, NOT,
    EQUALS, NOT_EQUALS, 
    GREATER, LESS, GREATER_EQUALS, LESS_EQUALS,
    COMMA, COLON,
    OPEN_PARENTHESES, CLOSE_PARENTHESES,
    OPEN_BRACES, CLOSE_BRACES,
    ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, ASTERISK_ASSIGN, SLASH_ASSIGN, MOD_ASSIGN,
    INCREMENT, DECREMENT,
    SET, GET, PRINT,
    IF, ELSE, WHILE, FOR,
    DEFINE, INVOKE, RETURN,
    SEMICOLON 
};

const std::map<std::string, Types> keywords = {
    {"empty", EMPTY}, {"exit", EXIT},
    {"set", SET}, {"get", GET},
    {"print", PRINT},
    {"void", VOID},
    {"true", BOOLEAN_TRUE}, {"false", BOOLEAN_FALSE},
    {"and", AND}, {"or", OR}, {"not", NOT}, {"is", EQUALS},
    {"if", IF}, {"else", ELSE}, {"while", WHILE}, {"for", FOR},
    {"define", DEFINE}, {"return", RETURN}, {"invoke", INVOKE}
};

typedef struct {
    Types type;
    std::string content;
    int line;
} Token;

class Tokenizer {
    public:
        Tokenizer(std::string source) {
            this->source = source;
            this->current = this->source[this->pos];
        }

        Token next() {
            START:
            if(this->current == '\n') { 
                Token token = make(END_OF_LINE, "END-OF-LINE");
                while(this->current == '\n') { this->advance(); this->line++; } 
                return token; 
            }

            if(isspace(this->current)) { while(isspace(this->current) && this->current != '\n') this->advance(); goto START; } 

            if(this->current == '#' || this->peek("/*")) {
                if(this->current == '#') { this->advance(); while(this->current != '\n' && this->current != '\0') this->advance(); }

                if(this->peek("/*")) { 
                    this->advance(2); 

                    while(this->peek("*/") == false) {
                        if(this->current == '\n') this->line++;
                        this->advance(); 
                    }

                    this->advance(2); 
                }

                goto START;
            }

            if(this->pos >= this->source.length()) { this->line++; return make(END_OF_FILE, "EOF"); }

            if(this->current == '"' || this->current == '\'' || this->current == '`') {
                char quote = this->current; this->advance();
                this->start = this->pos;
                
                return this->string(quote);
            }

            this->start = this->pos;

            if(isalpha(this->current) || this->current == '_' || this->current == '$') return this->word();
            if(isdigit(this->current) || this->current == '.') return this->number();

            if(this->peek("==")) { this->advance(2); return make(EQUALS); }
            if(this->peek("!=")) { this->advance(2); return make(NOT_EQUALS); }
            if(this->peek(">=")) { this->advance(2); return make(GREATER_EQUALS); }
            if(this->peek("<=")) { this->advance(2); return make(LESS_EQUALS); }

            if(this->peek("+=")) { this->advance(2); return make(PLUS_ASSIGN); }
            if(this->peek("-=")) { this->advance(2); return make(MINUS_ASSIGN); }
            if(this->peek("*=")) { this->advance(2); return make(ASTERISK_ASSIGN); }
            if(this->peek("/=")) { this->advance(2); return make(SLASH_ASSIGN); }
            if(this->peek("%=")) { this->advance(2); return make(MOD_ASSIGN); }

            if(this->peek("++")) { this->advance(2); return make(INCREMENT); }
            if(this->peek("--")) { this->advance(2); return make(DECREMENT); }

            switch(this->current) {
                case ':': this->advance(); return make(COLON);
                case '=': this->advance(); return make(ASSIGN);

                case '+': this->advance(); return make(PLUS);
                case '-': this->advance(); return make(MINUS);
                case '*': this->advance(); return make(ASTERISK);
                case '/': this->advance(); return make(SLASH);
                case '%': this->advance(); return make(MOD);

                case '@': this->advance(); return make(CONCATENATE);

                case '&': this->advance(); return make(AND);
                case '|': this->advance(); return make(OR);
                case '!': this->advance(); return make(NOT);

                case '>': this->advance(); return make(GREATER);
                case '<': this->advance(); return make(LESS);

                case ',': this->advance(); return make(COMMA);

                case '(': this->advance(); return make(OPEN_PARENTHESES);
                case ')': this->advance(); return make(CLOSE_PARENTHESES);
                case '{': this->advance(); return make(OPEN_BRACES);
                case '}': this->advance(); return make(CLOSE_BRACES);

                case ';': this->advance(); return make(SEMICOLON);
            }

            return make(ERROR, "Unexpected or Wrong character");
        }

    private:
        int line = 1;
        std::string source; char current;
        int start = 0, pos = 0;

        Token make(Types type, std::string message = "") {                
            Token token;                                          
            token.type = type;                                    
            token.content = message.empty() ? this->source.substr(this->start, this->pos - this->start) : message;
            token.line = this->line;

            return token;                                         
        } 

        bool peek(std::string token) {
            for(int i = 0, peek = this->pos; i < token.length(); i++, peek++) 
                if(token[i] != this->source[peek]) 
                    return false;         

            return true;
        }

        void advance(int advance = 1) {
            for(int i = 0; i < advance; i++) {
                this->pos++;

                if (this->pos == this->source.length()) { this->current = '\0'; break; }
                else this->current = this->source[this->pos];     
            }
        }

        Token word() {
            std::string result;

            while(isalpha(this->current) || isdigit(this->current) || this->current == '_' || this->current == '$') {
                result += this->current;
                this->advance();
            }

            auto keyword = keywords.find(result);

            if(keyword != keywords.end()) return make(keyword->second);
            else return make(IDENTIFIER);
        }

        Token number() {
            while(isdigit(this->current)) this->advance();

            if(this->current == '.') {
                this->advance();
                while(isdigit(this->current)) this->advance();
            }

            return make(NUMBER);
        }

        Token string(char quote) {
            while(this->current != quote && this->current != '\n') 
                this->advance();

            if(this->current == '\n') 
                return make(ERROR, "Unterminated String, expected closing quote");

            Token token = make(STRING); this->advance();
            return token;
        }
};

#endif