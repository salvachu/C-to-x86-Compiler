#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
#include <string>
#include <cctype>

class Token {
public:
    enum Type {HASH, INCLUDE, LIBRARY, VOID, UNSIGNED, UNSIGNEDL, INT, LONG, IF, ELSE, WHILE, FOR, RETURN, PRINTF, MAIN, PLUS, MINUS, MULT, DIV,
               INCRE, DECRE, ASSING, PLUSASSING, MINUSASSING, LT, LR, EQ, GT, GE, NE, PD, PI, CD, CI, PC, COMA, ID, NUM, STRING, ERR, END };

    Type type;
    std::string text;

    Token(Type type) : type(type), text("") {}
    Token(Type type, char c) : type(type), text(std::string(1, c)) {}
    Token(Type type, const std::string& source, int first, int last)
        : type(type), text(source.substr(first, last - first + 1)) {}
    Token(Type type, const std::string& value)
        : type(type), text(value) {}

    static std::string tokenTypeToString(Type type) {
        switch (type) {
            case HASH: return "HASH";case INCLUDE: return "INCLUDE";case LIBRARY: return "LIBRARY";
            case VOID: return "VOID";case UNSIGNED: return "UNSIGNED"; case UNSIGNEDL: return "UNSIGNEDL"; case INT: return "INT"; case LONG: return "LONG";
            case IF: return "IF"; case ELSE: return "ELSE";
            case WHILE: return "WHILE"; case FOR: return "FOR";
            case RETURN: return "RETURN"; case PRINTF: return "PRINTF"; 
            case MAIN: return "MAIN";
            case PLUS: return "PLUS"; case MINUS: return "MINUS"; case MULT: return "MULT"; case DIV: return "DIV";
            case INCRE: return "INCRE"; case DECRE: return "DECRE";
            case ASSING: return "ASSING"; case PLUSASSING: return "PLUSASSING"; case MINUSASSING: return "MINUSASSING";
            case LT: return "LT"; case LR: return "LR"; case EQ: return "EQ";
            case GT: return "GT"; case GE: return "GE"; case NE: return "NE";
            case PD: return "PD"; case PI: return "PI"; case CD: return "CD"; case CI: return "CI"; 
            case PC: return "PC"; case COMA: return "COMA";
            case ID: return "ID"; case NUM: return "NUM"; case STRING: return "STRING";
            case ERR: return "ERR"; case END: return "END";
            default: return "UNKNOWN";
        }
    }

    friend std::ostream& operator<<(std::ostream& outs, const Token& tok) {
        outs << "TOKEN(" << tokenTypeToString(tok.type) << ")";
        if (!tok.text.empty()) outs << " -> \"" << tok.text << "\"";
        return outs;
    }

    friend std::ostream& operator<<(std::ostream& outs, const Token* tok) {
        return tok ? outs << *tok : outs << "TOKEN(NULL)";
    }
};

#endif // TOKEN_H
