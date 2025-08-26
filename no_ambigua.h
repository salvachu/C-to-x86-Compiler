#ifndef NO_AMBIGUA_H
#define NO_AMBIGUA_H

#include "token.h"
#include <vector>
#include <string>

int main_count = 0;

void no_ambigua(std::vector<Token*>& tokens) {
    std::vector<Token*> result;

    for (size_t i = 0; i < tokens.size(); ++i) {
        Token* curr = tokens[i];

        if (curr->type == Token::INT &&
            i + 1 < tokens.size() &&
            tokens[i + 1]->type == Token::MAIN) {
            Token* mainToken = tokens[i + 1];
            result.push_back(mainToken);
            ++i; 
            main_count++;
            continue;
        }

        if (curr->type == Token::UNSIGNED) {
            if (i + 2 < tokens.size() &&
                tokens[i+1]->type == Token::LONG &&
                tokens[i+2]->type == Token::INT) {
                Token* t = new Token(Token::UNSIGNEDL, "unsigned long");
                result.push_back(t);
                i += 2;
                continue;
            }
            if (i + 1 < tokens.size() &&
                tokens[i+1]->type == Token::LONG) {
                Token* t = new Token(Token::UNSIGNEDL, "unsigned long");
                result.push_back(t);
                i += 1;
                continue;
            }
            Token* t = new Token(Token::UNSIGNED, "unsigned");
            result.push_back(t);
            if (i + 1 < tokens.size() && tokens[i+1]->type == Token::INT)
                ++i;
            continue;
        }

        if (curr->type == Token::LONG) {
            if (i + 1 < tokens.size() && tokens[i+1]->type == Token::INT)
                ++i;
            Token* t = new Token(Token::LONG, "long");
            result.push_back(t);
            continue;
        }

        if (curr->type == Token::INT) {
            Token* t = new Token(Token::INT, "int");
            result.push_back(t);
            continue;
        }

        result.push_back(curr);
    }

    tokens = result;
}

#endif // NO_AMBIGUA_H
