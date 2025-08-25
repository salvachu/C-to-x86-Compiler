#include <iostream>
#include <string>
#include <unordered_map>

class Token {
public:
    enum Type {
        INCLUDE, STDIO_H, LIBRARY, VOID, BOOL, UNSIGNED, INT, LONG, IF, ELSE, WHILE, FOR, RETURN, PRINTF, MAIN, TRUE, FALSE, PLUS, MINUS, MULT, DIV, 
        INCRE, DECRE, ASSING, PLUSASSING, MINUSASSING, LT, LR, EQ, GT, GE, NE, PD, PI, CD, CI, LLD, LLI, PC, COMA, ID, NUM, STRING, ERR, END, EPSILON
    };

    Type type;
    std::string text;
    Token(Type type) : type(type) { text = ""; }
    Token(Type type, char c) : type(type) { text = std::string(1, c); }
    Token(Type type, const std::string& source, int first, int last) : type(type) {
        text = source.substr(first, last - first + 1);
    }
    Token(Type type, const std::string& value) : type(type), text(value) {}

    friend std::ostream& operator<<(std::ostream& outs, const Token& tok);
    friend std::ostream& operator<<(std::ostream& outs, const Token* tok);
};

std::ostream& operator<<(std::ostream& outs, const Token& tok) {
    switch (tok.type) {
        case Token::INCLUDE: outs << "TOKEN(INCLUDE)"; break;
        case Token::STDIO_H: outs << "TOKEN(STDIO_H)"; break;
        case Token::LIBRARY: outs << "TOKEN(LIBRARY)"; break;
        case Token::VOID: outs << "TOKEN(VOID)"; break;
        case Token::BOOL: outs << "TOKEN(BOOL)"; break;
        case Token::UNSIGNED: outs << "TOKEN(UNSIGNED)"; break;
        case Token::INT: outs << "TOKEN(INT)"; break;
        case Token::LONG: outs << "TOKEN(LONG)"; break;
        case Token::IF: outs << "TOKEN(IF)"; break;
        case Token::ELSE: outs << "TOKEN(ELSE)"; break;
        case Token::WHILE: outs << "TOKEN(WHILE)"; break;
        case Token::FOR: outs << "TOKEN(FOR)"; break;
        case Token::RETURN: outs << "TOKEN(RETURN)"; break;
        case Token::PRINTF: outs << "TOKEN(PRINTF)"; break;
        case Token::MAIN: outs << "TOKEN(MAIN)"; break;
        case Token::TRUE: outs << "TOKEN(TRUE)"; break;
        case Token::FALSE: outs << "TOKEN(FALSE)"; break;
        
        case Token::PLUS: outs << "TOKEN(PLUS)"; break;
        case Token::MINUS: outs << "TOKEN(MINUS)"; break;
        case Token::MULT: outs << "TOKEN(MULT)"; break;
        case Token::DIV: outs << "TOKEN(DIV)"; break;
        case Token::INCRE: outs << "TOKEN(INCRE)"; break;
        case Token::DECRE: outs << "TOKEN(DECRE)"; break;
        case Token::ASSING: outs << "TOKEN(ASSING)"; break;
        case Token::PLUSASSING: outs << "TOKEN(PLUSASSING)"; break;
        case Token::MINUSASSING: outs << "TOKEN(MINUSASSING)"; break;
        
        case Token::LT: outs << "TOKEN(LT)"; break;
        case Token::LR: outs << "TOKEN(LR)"; break;     
        case Token::EQ: outs << "TOKEN(EQ)"; break;
        case Token::GT: outs << "TOKEN(GT)"; break;
        case Token::GE: outs << "TOKEN(GE)"; break;
        case Token::NE: outs << "TOKEN(NE)"; break;
        
        case Token::PD: outs << "TOKEN(PD)"; break;
        case Token::PI: outs << "TOKEN(PI)"; break;     
        case Token::CD: outs << "TOKEN(CD)"; break;     
        case Token::CI: outs << "TOKEN(CI)"; break;     
        case Token::LLD: outs << "TOKEN(LLD)"; break;   
        case Token::LLI: outs << "TOKEN(LLI)"; break;   
        case Token::PC: outs << "TOKEN(PC)"; break;     
        case Token::COMA: outs << "TOKEN(COMA)"; break;
        
        case Token::ID: outs << "TOKEN(ID)"; break;
        case Token::NUM: outs << "TOKEN(NUM)"; break;
        case Token::STRING: outs << "TOKEN(STRING)"; break;
        
        case Token::ERR: outs << "TOKEN(ERR)"; break;
        case Token::END: outs << "TOKEN(END)"; break;
        case Token::EPSILON: outs << "TOKEN(EPSILON)"; break;
        
        default: outs << "TOKEN(UNKNOWN)"; break;
    }
    return outs;
}


int main(){
    return 0;
}