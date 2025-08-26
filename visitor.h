#ifndef VISITOR_H
#define VISITOR_H

#include <unordered_map>
#include <string>
#include <unordered_set>

#include "env.h"


struct TypeInfo;
class UnaryExp;
class BinaryExp;
class NumberExp;
class IdentifierExp;
class FCallExp;
class PrintStatement;
class IfStatement;
class WhileStatement;
class ForStatement;
class VarDec;
class Body;
class FunDec;
class ReturnStatement;
class Program;
class ExpressionStatement;

class Visitor {
public:
    virtual ~Visitor() {}

    std::unordered_map<std::string, std::string> mapaFormatoATipo = {
        {"d", "int"},
        {"u", "unsigned"},
        {"ld", "long"},
        {"lu", "unsignedl"}
    };


    std::string aceptados = "%d %u %ld %lu";


    bool esTruncamientoPeligroso(const TypeInfo& from, const std::string& to);
    virtual TypeInfo visit(UnaryExp* e) = 0;
    virtual TypeInfo visit(BinaryExp* e) = 0;
    virtual TypeInfo visit(NumberExp* e) = 0;
    virtual TypeInfo visit(IdentifierExp* e) = 0;
    virtual TypeInfo visit(FCallExp* e) = 0;
    virtual void visit(PrintStatement* s) = 0;
    virtual void visit(IfStatement* s) = 0;
    virtual void visit(WhileStatement* s) = 0;
    virtual void visit(ForStatement* s) = 0;
    virtual void visit(ReturnStatement* s) = 0;
    virtual void visit(VarDec* v) = 0;
    virtual void visit(Body* b) = 0;
    virtual void visit(FunDec* f) = 0;
    virtual void visit(Program* p) = 0;
    virtual void visit(ExpressionStatement* s) = 0;
};

class TypeCheckerVisitor : public Visitor {
private:

    Environment<TypeInfo> env;  
    std::unordered_map<std::string, std::string> functionReturnTypes;
    std::string currentFunctionType;
    std::unordered_map<std::string, std::vector<std::string>> functionParamTypes;
    std::unordered_map<std::string, TypeInfo> functionReturnValues;
    bool hasStdio = false;
    int errorCount = 0;
    int warningCount = 0;
    bool hasReturn = false;
    std::string currentFunctionName;

public:
    int getErrorCount() const { return errorCount; }
    int getWarningCount() const { return warningCount; }
    bool hasStdioIncluded() const;
    void warning(const std::string& msg, bool incluirFuncion);
    void error(const std::string& msg, bool incluirFuncion);
    void reset();
    void validatePrintf(PrintStatement* s);

    TypeInfo visit(UnaryExp* e) override;
    TypeInfo visit(BinaryExp* e) override;
    TypeInfo visit(NumberExp* e) override;
    TypeInfo visit(IdentifierExp* e) override;
    TypeInfo visit(FCallExp* e) override;

    void visit(PrintStatement* s) override;
    void visit(IfStatement* s) override;
    void visit(WhileStatement* s) override;
    void visit(ForStatement* s) override;
    void visit(ReturnStatement* s) override;
    void visit(VarDec* v) override;
    void visit(Body* b) override;
    void visit(FunDec* f) override;
    void visit(Program* p) override;
    void visit(ExpressionStatement* s)override;
};

class CodeGenVisitor : public Visitor {
private:
    std::ostream& out;
    std::unordered_map<std::string, int> memoria;        
    std::unordered_map<std::string, std::string> tiposVariables; 
    std::unordered_map<std::string, std::string> functionReturnTypes;
    std::unordered_set<std::string> formatosUsados;     
    bool firstPass;                
    int offset;
    int labelcont;                 
    bool entornoFuncion;          
    std::string nombreFuncion;        
    void emit(const std::string& instruction);
    void emit_label(const std::string& label);
    std::string new_label(const std::string& prefix = "L");
    void emit_truncation(const std::string& from_type, const std::string& to_type);
    void emit_unsigned_conversion(long value, const std::string& target_type);
    std::string get_printf_format(const std::string& tipo);
    std::string get_printf_label(const std::string& tipo);
    void register_printf_format(const std::string& tipo);
    void generate_used_formats();
    void emit_convert(const std::string& reg, const std::string& from_type, const std::string& to_type);
    void emit_op(const std::string& op, const std::string& a, const std::string& b, const std::string& tipo);

public:
    CodeGenVisitor(std::ostream& output) : out(output), firstPass(false), offset(-8), labelcont(0), entornoFuncion(false) {}
    
    void generar(Program* program);
    void first_pass(Program* program);  
    void second_pass(Program* program); 
    
    TypeInfo visit(UnaryExp* e) override;
    TypeInfo visit(BinaryExp* e) override;
    TypeInfo visit(NumberExp* e) override;
    TypeInfo visit(IdentifierExp* e) override;
    TypeInfo visit(FCallExp* e) override;

    void visit(PrintStatement* s) override;
    void visit(IfStatement* s) override;
    void visit(WhileStatement* s) override;
    void visit(ForStatement* s) override;
    void visit(ReturnStatement* s) override;
    void visit(VarDec* v) override;
    void visit(Body* b) override;
    void visit(FunDec* f) override;
    void visit(Program* p) override;
    void visit(ExpressionStatement* s) override;
};

#endif // VISITOR_H
