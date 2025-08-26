#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include <string>
#include <cctype>
#include<vector>
#include <iostream>

class Scanner {
private:

    std::vector<Token*> token_stream;
    int token_index = 0;
    bool use_token_stream = false;

    std::string input;
    int first, current;

    bool is_white_space(char c) {
        return c == ' ' || c == '\n' || c == '\t' || c == '\r';
    }


public:
    Scanner(const char* in_s) : input(in_s), first(0), current(0) {}

    Scanner(const std::vector<Token*>& tokens)
    : token_stream(tokens), token_index(0), use_token_stream(true), first(0), current(0) {}

    Token* nextToken() {

        if (use_token_stream) {
        if (token_index < token_stream.size())
            return token_stream[token_index++];
        else
            return new Token(Token::END);
        }

        while (current < input.length() && is_white_space(input[current]))
            current++;

        if (current >= input.length()) return new Token(Token::END);

        char c = input[current];
        first = current;

        if (c == '"') {
            current++;
            int start = current;
            while (current < input.length() && input[current] != '"') {
                if (input[current] == '\\' && current + 1 < input.length()) current += 2;
                else current++;
            }
            std::string content = input.substr(start, current - start);
            if (current < input.length() && input[current] == '"') current++;
            return new Token(Token::STRING, content);
        }

        if (isdigit(c)) {
            current++;
            while (current < input.length() && isdigit(input[current])) current++;
            return new Token(Token::NUM, input, first, current - 1);
        }

        if (isalpha(c) || c == '_') {
            current++;
            while (current < input.length() && (isalnum(input[current]) || input[current] == '_' || input[current] == '.')) current++;
            std::string word = input.substr(first, current - first);

            if (word == "void") return new Token(Token::VOID, word);
            if (word == "int") return new Token(Token::INT, word);
            if (word == "long") return new Token(Token::LONG, word);
            if (word == "unsigned") return new Token(Token::UNSIGNED, word);
            if (word == "unsignedl") return new Token(Token::UNSIGNEDL, word);
            if (word == "main") return new Token(Token::MAIN, word);
            if (word == "return") return new Token(Token::RETURN, word);
            if (word == "printf") return new Token(Token::PRINTF, word);
            if (word == "if") return new Token(Token::IF, word);
            if (word == "else") return new Token(Token::ELSE, word);
            if (word == "for") return new Token(Token::FOR, word);
            if (word == "while") return new Token(Token::WHILE, word);

            if (word == "include") return new Token(Token::INCLUDE, word);
            if (word == "stdio.h") return new Token(Token::LIBRARY, word);

            return new Token(Token::ID, word);
        }

        switch (c) {
            case '+':
                if (current + 1 < input.length() && input[current + 1] == '+') {
                    current += 2; return new Token(Token::INCRE, "++");
                }
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    current += 2; return new Token(Token::PLUSASSING, "+=");
                }
                current++; return new Token(Token::PLUS, "+");

            case '-':
                if (current + 1 < input.length() && input[current + 1] == '-') {
                    current += 2; return new Token(Token::DECRE, "--");
                }
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    current += 2; return new Token(Token::MINUSASSING, "-=");
                }
                current++; return new Token(Token::MINUS, "-");

            case '*': current++; return new Token(Token::MULT, "*");
            case '/': current++; return new Token(Token::DIV, "/");

            case '=':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    current += 2; return new Token(Token::EQ, "==");
                }
                current++; return new Token(Token::ASSING, "=");

            case '<':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    current += 2; return new Token(Token::LR, "<=");
                }
                current++; return new Token(Token::LT, "<");

            case '>':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    current += 2; return new Token(Token::GE, ">=");
                }
                current++; return new Token(Token::GT, ">");

            case '!':
                if (current + 1 < input.length() && input[current + 1] == '=') {
                    current += 2; return new Token(Token::NE, "!=");
                }
                current++; return new Token(Token::ERR, "!");
            
            case '(': current++; return new Token(Token::PI, "(");  
            case ')': current++; return new Token(Token::PD, ")");  
            case '{': current++; return new Token(Token::CD, "{");
            case '}': current++; return new Token(Token::CI, "}");
            case ';': current++; return new Token(Token::PC, ";");
            case ',': current++; return new Token(Token::COMA, ",");
            case '#': current++; return new Token(Token::HASH, "#");

            default:
                current++;
                return new Token(Token::ERR, std::string(1, c));
        }
    }
    std::vector<Token*> scanAllTokens() {
        std::vector<Token*> tokens;
        Token* t;
        while ((t = nextToken())->type != Token::END) {
            tokens.push_back(t);
        }
        return tokens;
    }


};

void test_scanner(Scanner* scanner) {
    Token* current;
    std::cout << "Escaneando:\n";
    while ((current = scanner->nextToken())->type != Token::END) {
        std::cout << *current << "\n";
    }
    std::cout << "TOKEN(END)\n";
}




#endif // SCANNER_H
