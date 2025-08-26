#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "exp.h"
#include <iostream>
#include <stdexcept>
#include "no_ambigua.h"

using namespace std;

class Parser {
private:
    Scanner* scanner;
    Token *current, *previous;

    bool match(Token::Type ttype) {
        if (check(ttype)) {
            std::cout << "[match] Consumido token: " << current->text << " (tipo: " << ttype << ")" << std::endl;
            advance();
            return true;
        }
        return false;
    }

    bool check(Token::Type ttype) {
        if (isAtEnd()) return false;
        return current->type == ttype;
    }

    bool advance() {
        if (!isAtEnd()) {
            Token* temp = current;
            if (previous) delete previous;
            current = scanner->nextToken();
            previous = temp;
            if (check(Token::ERR)) {
                std::cout << "Error de análisis, carácter no reconocido: " << current->text << std::endl;
                exit(1);
            }
            return true;
        }
        return false;
    }

    bool isAtEnd() {
        return current->type == Token::END;
    }

public:

    Parser(Scanner* sc): scanner(sc), previous(nullptr) {
        current = scanner->nextToken();
        if (current->type == Token::ERR) {
            std::cout << "Error en el primer token: " << current->text << std::endl;
            exit(1);
        }
    }

    Program* parseProgram() {
        list<string> includes;
        do {
            includes.push_back(parseInclude());
        } while (check(Token::HASH));
        list<FunDec*> funciones;
        while (check(Token::LONG) || check(Token::VOID) || (check(Token::UNSIGNEDL)) || (check(Token::INT)) || check(Token::UNSIGNED) ) {
            funciones.push_back(parseFunction());
        }

        Body* mainBody = parseMain();

        if (main_count != 1) {
            std::cerr << "Error: Se encontraron " << main_count << " funciones main(). Debe haber exactamente una." << std::endl;
            exit(1);
        }
        return new Program(includes, funciones, mainBody);
    }


    std::string parseInclude() {
        if (!match(Token::HASH)) {
            std::cout << "Error: se esperaba '#' al inicio del include.\n";
            exit(1);
        }

        if (!match(Token::INCLUDE)) {
            std::cout << "Error: se esperaba 'include'.\n";
            exit(1);
        }
        if (!match(Token::LT)) {
            std::cout << "Error: se esperaba '<' en la declaración de include." << std::endl;
            exit(1);
        }

        if (!match(Token::LIBRARY)) {
            std::cout << "Error: se esperaba una librería (ej. stdio.h), pero se encontró: " << current->text << std::endl;
            exit(1);
        }

        std::string lib = previous->text;

        if (!match(Token::GT)) {
            std::cout << "Error: se esperaba '>' al final del include." << std::endl;
            exit(1);
        }

        return lib;
    }

    FunDec* parseFunction() {
        std::string tipo;
        if (match(Token::INT)) tipo = "int";
        else if (match(Token::LONG)) tipo = "long";
        else if (match(Token::UNSIGNED)) tipo = "unsigned";
        else if (match(Token::UNSIGNEDL)) tipo = "unsigned long";
        else if (match(Token::VOID)) tipo = "void";
        else {
            std::cout << "Error: se esperaba tipo de retorno válido (int, long, unsigned long, void)." << std::endl;
            exit(1);
        }

        if (!match(Token::ID)) {
            std::cout << "Error: se esperaba identificador para la función." << std::endl;
            exit(1);
        }
        std::string nombre = previous->text;

        if (!match(Token::PI)) {
            std::cout << "Error: se esperaba '(' tras el nombre de la función." << std::endl;
            exit(1);
        }

        std::vector<std::string> params;
        std::vector<std::string> tipos;

        if (!check(Token::PD)) {
            do {
                std::string tipoParam;

                if (match(Token::INT)) tipoParam = "int";
                else if (match(Token::LONG)) tipoParam = "long";
                else if (match(Token::UNSIGNED)) tipoParam = "unsigned";
                else if (match(Token::UNSIGNEDL)) tipoParam = "unsigned long";
                else {
                    std::cout << "Error: se esperaba tipo de parámetro (int, long, unsigned long)." << std::endl;
                    exit(1);
                }
                tipos.push_back(tipoParam);

                if (!match(Token::ID)) {
                    std::cout << "Error: se esperaba identificador de parámetro." << std::endl;
                    exit(1);
                }
                params.push_back(previous->text);
            } while (match(Token::COMA));
        }

        if (!match(Token::PD)) {
            std::cout << "Error: se esperaba ')' cerrando parámetros." << std::endl;
            exit(1);
        }

        if (!match(Token::CD)) {
            std::cout << "Error: se esperaba '{' abriendo el cuerpo de la función." << std::endl;
            exit(1);
        }
        cout << *(current) <<endl;

        Body* cuerpo = new Body(parseStatementList());

        if (!match(Token::CI)) {
            std::cout << "Error: se esperaba '}' cerrando el cuerpo de la función." << std::endl;
            exit(1);
        }

        return new FunDec(nombre, tipo, params, tipos, cuerpo);
    }

    Body* parseMain() {
        if (!match(Token::MAIN)) {
            std::cout << "Error: se esperaba 'main'. Encontrado: " << current->text << std::endl;
            exit(1);
        }

        if (!match(Token::PI) || !match(Token::PD)) {
            std::cout << "Error: se esperaba '()' tras 'main'. Encontrado: " << current->text << std::endl;
            exit(1);
        }

        if (!match(Token::CD)) {
            std::cout << "Error: se esperaba '{' al inicio del cuerpo de main. Encontrado: " << current->text << std::endl;
            exit(1);
        }

        list<Stm*> stms = parseStatementList();

        if (!match(Token::CI)) {
            std::cout << "Error: se esperaba '}' al final de main. Encontrado: " << current->text << std::endl;
            exit(1);
        }

        return new Body(stms);
    }


    list<VarDec*> parseVarDecList() {
        list<VarDec*> vardecs;

        while (check(Token::LONG) || check(Token::UNSIGNEDL) || check(Token::UNSIGNED) || check(Token::INT)) {
            std::cout << "[parseVarDecList] Token actual: " << current->text << std::endl;
            vardecs.push_back(parseVarDec());
        }

        return vardecs;
    }

    VarDec* parseVarDec(bool soloUna = false) {
        std::string tipo;

        if (match(Token::INT)) tipo = "int";
        else if (match(Token::LONG)) tipo = "long";
        else if (match(Token::UNSIGNED)) tipo = "unsigned";
        else if (match(Token::UNSIGNEDL)) tipo = "unsigned long";
        else {
            std::cout << "Error: se esperaba tipo válido." << std::endl;
            exit(1);
        }

        std::list<std::pair<std::string, Exp*>> vars;

        if (!match(Token::ID)) {
            std::cout << "Error: se esperaba nombre de variable." << std::endl;
            exit(1);
        }

        std::string nombre = previous->text;
        Exp* valor = nullptr;

        if (match(Token::ASSING)) {
            std::cout << "[DEBUG] Entrando a parseCExp() en declaración de variable" << std::endl;
            valor = parseCExp();
            std::cout << "[DEBUG] Terminé parseCExp() en declaración de variable" << std::endl;
        }
        vars.push_back({nombre, valor});

        if (!soloUna) {
            while (match(Token::COMA)) {
                if (!match(Token::ID)) {
                    std::cout << "Error: se esperaba nombre de variable tras ','." << std::endl;
                    exit(1);
                }

                std::string nombreExtra = previous->text;
                Exp* valExtra = nullptr;
                if (match(Token::ASSING)) {
                    valExtra = parseCExp();
                }

                vars.push_back({nombreExtra, valExtra});
            }
        }
        std::cout << "[DEBUG] Token actual: '" << current->text << "', tipo: " << current->type << std::endl;
        if (!match(Token::PC)) {
            std::cout << "Error: se esperaba ';' al final de la declaración." << std::endl;
            exit(1);
        }

        return new VarDec(tipo, vars);
    }


list<Stm*> parseStatementList() {
    list<Stm*> stms;
    cout << "[parseStatementList] Iniciando..." << endl;

    while (!check(Token::CI) && !isAtEnd()) {
        if (check(Token::INT) || check(Token::LONG) || check(Token::UNSIGNEDL) || check(Token::UNSIGNED)) {
            while (check(Token::INT) || check(Token::LONG) || check(Token::UNSIGNEDL) || check(Token::UNSIGNED)) {
                std::cout << "[parseStatementList] Token actual (declaración): " << current->text << std::endl;
                stms.push_back(parseVarDec());
            }
        } else {
            stms.push_back(parseStatement());
        }
    }

    return stms;
}



    Stm* parseStatement() {

        if (check(Token::ID)) {
            Exp* expr = parseCExp();
            if (!match(Token::PC)) {
                std::cout << "Se esperaba ';' tras la expresión." << std::endl;
                exit(1);
            }
            return new ExpressionStatement(expr);
        }

        if (match(Token::PRINTF)) {


            if (!match(Token::PI)) {
                std::cout << "Se esperaba '(' tras printf." << std::endl;
                exit(1);
            }

            std::string formato = "";
            std::vector<Exp*> args;

            if (check(Token::STRING)) {
                formato = current->text;
                match(Token::STRING); 
                if (check(Token::COMA)) {
                    match(Token::COMA); 
                }
            }

            if (check(Token::ID)   || check(Token::NUM)    || check(Token::PI)  ||
                check(Token::PLUS) || check(Token::MINUS)  ||
                check(Token::INCRE)|| check(Token::DECRE)) 
            {
                args.push_back(parseCExp());
                while (match(Token::COMA)) {
                    args.push_back(parseCExp());
                }
            } else if (formato.empty()) {
                std::cout << "Se esperaba al menos un argumento o un string en printf." << std::endl;
                exit(1);
            }


            if (!match(Token::PD)) {
                std::cout << "Se esperaba ')' al final de printf." << std::endl;
                exit(1);
            }

            if (!match(Token::PC)) {
                std::cout << "Se esperaba ';' tras printf." << std::endl;
                exit(1);
            }

            return new PrintStatement(formato, args);
        }

        if (match(Token::IF)) {
            if (!match(Token::PI)) {
                std::cout << "Se esperaba '(' tras 'if'." << std::endl;
                exit(1);
            }

            Exp* condition = parseCExp();

            if (!match(Token::PD)) {
                std::cout << "Se esperaba ')' cerrando condición del 'if'." << std::endl;
                exit(1);
            }

            if (!match(Token::CD)) {
                std::cout << "Se esperaba '{' para abrir el bloque del 'if'." << std::endl;
                exit(1);
            }

            list<Stm*> stmsThen = parseStatementList();

            if (!match(Token::CI)) {
                std::cout << "Se esperaba '}' cerrando bloque del 'if'." << std::endl;
                exit(1);
            }

            Body* thenBody = new Body(stmsThen);
            Body* elseBody = nullptr;

            if (match(Token::ELSE)) {
                if (!match(Token::CD)) {
                    std::cout << "Se esperaba '{' para abrir el bloque del 'else'." << std::endl;
                    exit(1);
                }

                list<Stm*> stmsElse = parseStatementList();

                if (!match(Token::CI)) {
                    std::cout << "Se esperaba '}' cerrando bloque del 'else'." << std::endl;
                    exit(1);
                }

                elseBody = new Body(stmsElse);
            }

            return new IfStatement(condition, thenBody, elseBody);
        }

        if (match(Token::WHILE)) {
            if (!match(Token::PI)) {
                std::cout << "Se esperaba '(' tras 'while'." << std::endl;
                exit(1);
            }

            Exp* condition = parseCExp();

            if (!match(Token::PD)) {
                std::cout << "Se esperaba ')' cerrando condición del 'while'." << std::endl;
                exit(1);
            }

            if (!match(Token::CD)) {
                std::cout << "Se esperaba '{' abriendo el cuerpo del 'while'." << std::endl;
                exit(1);
            }

            list<Stm*> stms = parseStatementList();

            if (!match(Token::CI)) {
                std::cout << "Se esperaba '}' cerrando el cuerpo del 'while'." << std::endl;
                exit(1);
            }

            Body* cuerpo = new Body(stms);
            return new WhileStatement(condition, cuerpo);
        }


        if (match(Token::FOR)) {
            if (!match(Token::PI)) {
                std::cout << "Se esperaba '(' tras 'for'." << std::endl;
                exit(1);
            }

            VarDec* varInit = nullptr;
            Exp* expInit = nullptr;

            if (check(Token::INT) || check(Token::LONG) || check(Token::UNSIGNEDL) || check(Token::UNSIGNED)) {
                varInit = parseVarDec(true); 
            } else {
                expInit = parseCExp();
                if (!match(Token::PC)) {
                    std::cout << "Se esperaba ';' después de la inicialización en 'for'." << std::endl;
                    exit(1);
                }
            }

            Exp* cond = parseCExp();
            if (!match(Token::PC)) {
                std::cout << "Se esperaba ';' después de la condición del for." << std::endl;
                exit(1);
            }

            Exp* update = parseCExp();
            if (!match(Token::PD)) {
                std::cout << "Se esperaba ')' cerrando la cabecera del for." << std::endl;
                exit(1);
            }

            if (!match(Token::CD)) {
                std::cout << "Se esperaba '{' abriendo el cuerpo del for." << std::endl;
                exit(1);
            }

            std::list<Stm*> stms = parseStatementList();

            if (!match(Token::CI)) {
                std::cout << "Se esperaba '}' cerrando el cuerpo del for." << std::endl;
                exit(1);
            }

            Body* cuerpo = new Body(stms);
            return new ForStatement(varInit, expInit, cond, update, cuerpo);
        }

        if (match(Token::RETURN)) {
            Exp* e = parseCExp();
            
            if (!match(Token::PC)) {
                std::cout << "Se esperaba ';' después de return." << std::endl;
                exit(1);
            }

            return new ReturnStatement(e);
        }


        std::cout << "Error: statement desconocido." << std::endl;
        exit(1);
        return nullptr;  

    }


    Exp* parseCExp() {
        return parseAssignExpr();
    }

    Exp* parseAssignExpr() {
        Exp* left = parseRelExpr();

        if (auto idExp = dynamic_cast<IdentifierExp*>(left)) {
            if (match(Token::ASSING)) {
                Exp* right = parseAssignExpr();  // recursivo
                return new BinaryExp(left, right, ASSIGN_OP);
            }
            if (match(Token::PLUSASSING)) {
                Exp* right = parseAssignExpr();  // recursivo
                return new BinaryExp(left, right, PLUS_ASSIGN_OP);
            }
            if (match(Token::MINUSASSING)) {
                Exp* right = parseAssignExpr();  // recursivo
                return new BinaryExp(left, right, MINUS_ASSIGN_OP);
            }
        }

        return left;
    }

    Exp* parseRelExpr() {
        Exp* left = parseAddExpr();

        while (check(Token::LT) || check(Token::LR) || check(Token::GT) || 
            check(Token::GE) || check(Token::EQ) || check(Token::NE)) {
            
            Token::Type opType = current->type;
            advance();
            BinaryOp op;
            switch (opType) {
                case Token::LT: op = LT_OP; cout << "mega kaka" << endl; break;
                case Token::LR: op = LE_OP; break;
                case Token::GT: op = GT_OP; break;
                case Token::GE: op = GE_OP; break;
                case Token::EQ: op = EQ_OP; break;
                case Token::NE: op = NE_OP; break;
                default:
                    std::cout << "Error: operador relacional no esperado.\n";
                    exit(1);
            }

            Exp* right = parseAddExpr();
            left = new BinaryExp(left, right, op);
        }

        return left;
    }



    Exp* parseAddExpr() {
        Exp* left = parseMulExpr();

        while (match(Token::PLUS) || match(Token::MINUS)) {
            BinaryOp op = previous->type == Token::PLUS ? PLUS_OP : MINUS_OP;
            Exp* right = parseMulExpr();
            left = new BinaryExp(left, right, op);
        }

        return left;
    }


    Exp* parseMulExpr() {
        Exp* left = parseUnaryExpr();

        while (match(Token::MULT) || match(Token::DIV)) {
            BinaryOp op = previous->type == Token::MULT ? MUL_OP : DIV_OP;
            Exp* right = parseUnaryExpr();
            left = new BinaryExp(left, right, op);
        }

        return left;
    }



    Exp* parseUnaryExpr() {
        if (match(Token::INCRE)) {
            if (!match(Token::ID)) {
                std::cout << "Error: se esperaba identificador después de ++\n";
                exit(1);
            }
            return new UnaryExp(previous->text, PRE_INC_OP);
        }

        if (match(Token::DECRE)) {
            if (!match(Token::ID)) {
                std::cout << "Error: se esperaba identificador después de --\n";
                exit(1);
            }
            return new UnaryExp(previous->text, PRE_DEC_OP);
        }

        return parsePrimary(); 
    }


    Exp* parsePrimary() {
        if (match(Token::PLUS)) {
            Exp* sub = parsePrimary();
            return new UnaryExp(POS_OP, sub);
        }
        if (match(Token::MINUS)) {
            Exp* sub = parsePrimary();
            return new UnaryExp(NEG_OP, sub);
        }
        if (match(Token::NUM)) {
            return new NumberExp(std::stoi(previous->text));
        }
        if (match(Token::ID)) {
            std::string id = previous->text;

            if (match(Token::PI)) {
                std::vector<Exp*> args;
                if (!check(Token::PD)) {
                    do {
                        args.push_back(parseCExp());
                    } while (match(Token::COMA));
                }
                if (!match(Token::PD)) {
                    std::cout << "Error: se esperaba ')' cerrando llamada a función.\n";
                    exit(1);
                }
                return new FCallExp(id, args);
            }

            if (match(Token::INCRE)) {
                return new UnaryExp(id, POST_INC_OP);
            }
            if (match(Token::DECRE)) {
                return new UnaryExp(id, POST_DEC_OP);
            }

            return new IdentifierExp(id);
        }

        if (match(Token::PI)) {
            Exp* e = parseCExp();
            if (!match(Token::PD)) {
                std::cout << "Error: se esperaba ')' cerrando expresión.\n";
                exit(1);
            }
            return e;
        }

        std::cout << "Error: expresión primaria inválida.\n";
        exit(1);
    }



};

#endif // PARSER_H
