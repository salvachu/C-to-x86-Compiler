#include <iostream>
#include <fstream>
#include <sstream>
#include "no_ambigua.h"
#include "parser.h"
#include "visitor.h"





int main() {
    std::ifstream file("input.txt");
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir input.txt" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Scanner scanner(source.c_str());
    std::vector<Token*> tokens = scanner.scanAllTokens();

    std::cout << "=== Tokens originales ===\n";
    for (Token* t : tokens) {
        std::cout << *t << "\n";
    }

    no_ambigua(tokens);

    std::cout << "\n=== Tokens después de no_ambigua ===\n";
    Scanner nuevo_scanner(tokens);
    for (Token* t : tokens) {
        std::cout << *t << "\n";
    }

    Parser parser(&nuevo_scanner);

    Program* prog = parser.parseProgram();
    std::cout << "\n¡Parseo exitoso!\n";

    TypeCheckerVisitor checker;
    prog->accept(&checker);

    if (checker.getErrorCount() > 0 || main_count > 1) {
        if (main_count > 1) {
            checker.error(std::to_string(main_count) + " funciones 'main' encontradas. Solo se permite una.",0);
        }
        std::cout << "El chequeo de tipos falló." << std::endl;
        exit(0); 
    } else {
        std::cout << "Chequeo de tipos exitoso." << std::endl;
    }

    std::cout << "\n=== Generando código ===\n";
    std::ofstream output("output.s");
    CodeGenVisitor codegen(output);
    codegen.generar(prog);  
    output.close();
    std::cout << "Código generado en output.s\n";

    return 0;
}
