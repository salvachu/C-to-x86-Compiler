#ifndef EXP_H
#define EXP_H

#include <string>
#include <unordered_map>
#include <list>
#include <vector>
#include <stdexcept>
#include "visitor.h"
using namespace std;

struct TypeInfo {
    std::string tipo;
    int valor;
    bool isConst;
    std::string origen;  

    bool esTipoSinSigno() const {
        return tipo == "unsigned" || tipo == "unsigned long";
    }

    bool esNegativo() const {
        return isConst && valor < 0;
    }


    TypeInfo(std::string t = "int", int v = 0, bool c = false, std::string o = "")
        : tipo(t), valor(v), isConst(c), origen(o) {}
};



enum BinaryOp {
    PLUS_OP,        
    MINUS_OP,       
    MUL_OP,         
    DIV_OP,         
    LT_OP,          
    LE_OP,          
    EQ_OP,          
    GT_OP,
    GE_OP,
    NE_OP,
    ASSIGN_OP,      
    PLUS_ASSIGN_OP, 
    MINUS_ASSIGN_OP 
};

enum UnaryOp {
    NEG_OP,        
    POS_OP,        
    PRE_INC_OP,    
    PRE_DEC_OP,    
    POST_INC_OP,   
    POST_DEC_OP    
};



class Body;

class Exp {
public:
    virtual TypeInfo accept(Visitor* visitor) = 0;
    virtual ~Exp() {}
    static char binopToChar(BinaryOp op);
};

class UnaryExp : public Exp {
public:
    UnaryOp op;
    std::string id;  
    Exp* expr;       

    UnaryExp(const std::string& id, UnaryOp op)
        : op(op), id(id), expr(nullptr) {}

    UnaryExp(UnaryOp op, Exp* expr)
        : op(op), id(""), expr(expr) {}

    TypeInfo accept(Visitor* visitor) override;
};



class BinaryExp : public Exp {
public:
    Exp *left, *right;
    std::string type;
    BinaryOp op;
    BinaryExp(Exp* l, Exp* r, BinaryOp op) : left(l), right(r), op(op) {}
    TypeInfo accept(Visitor* visitor) override;
    ~BinaryExp() override {}
};

class NumberExp : public Exp {
public:
    int value;
    NumberExp(int v) : value(v) {}
    TypeInfo accept(Visitor* visitor) override;
    ~NumberExp() override {}
};

class IdentifierExp : public Exp {
public:
    std::string name;
    IdentifierExp(const std::string& n) : name(n) {}
    TypeInfo accept(Visitor* visitor) override;
    ~IdentifierExp() override {}
};

class FCallExp : public Exp {
public:
    std::string nombre;
    std::vector<Exp*> argumentos;
    FCallExp(std::string n, std::vector<Exp*> args) : nombre(n), argumentos(args) {}
    TypeInfo accept(Visitor* visitor) override;
    ~FCallExp() override {}
};


class Stm {
public:
    virtual int accept(Visitor* visitor) = 0;
    virtual ~Stm() {}
};

class WhileStatement : public Stm {
public:
    Exp* condition;
    Body* b;
    WhileStatement(Exp* condition, Body* b) : condition(condition), b(b) {}
    int accept(Visitor* visitor) override;
    ~WhileStatement() override {}
};

class PrintStatement : public Stm {
public:
    std::string formato;
    std::vector<Exp*> argumentos;
    std::vector<string> bools;
    PrintStatement(const std::string& f, const std::vector<Exp*>& args) : formato(f), argumentos(args) {}
    int accept(Visitor* visitor) override;
    ~PrintStatement() override {}



    void countFormatSpecifiers() {
        for (size_t i = 0; i + 1 < formato.size(); ++i) {
            if (formato[i] == '%' && formato[i + 1] != '%') {
                std::string spec;

                spec += formato[i + 1]; 
            
                if (i + 2 < formato.size() && formato[i + 1] == 'l' && (formato[i + 2] == 'd' || formato[i + 2] == 'u')) {
                    spec += formato[i + 2];
                    ++i;
                }

                bools.push_back(spec);
                ++i; 
            }
            else if (formato[i] == '%' && formato[i + 1] == '%') {
                ++i;
            }
        }
    }

};

class ReturnStatement : public Stm {
public:
    Exp* e;
    ReturnStatement(Exp* e) : e(e) {}
    int accept(Visitor* visitor) override;
    ~ReturnStatement() override {}
};


class ExpressionStatement : public Stm {
public:
    Exp* expr;
    ExpressionStatement(Exp* e) : expr(e) {}
    int accept(Visitor* v) override;
};

class IfStatement : public Stm {
public:
    Exp* condition;
    Body* then;
    Body* els;
    IfStatement(Exp* condition, Body* then, Body* els) : condition(condition), then(then), els(els) {}
    int accept(Visitor* visitor) override;
    ~IfStatement() override {}
};

class VarDec : public Stm {
public:
    std::string tipo;
    std::list<std::pair<std::string, Exp*>> vars;
    VarDec(std::string tipo, std::list<std::pair<std::string, Exp*>> vars) : tipo(tipo), vars(vars) {}
    int accept(Visitor* visitor);
    ~VarDec() {}
};

class Body {
public:
    list<Stm*> stms;
    Body(list<Stm*> stms) : stms(stms) {}
    int accept(Visitor* visitor);
    ~Body() {}
};

class ForStatement : public Stm {
public:
    VarDec* varInit;     
    Exp* init;          
    Exp* cond;
    Exp* update;
    Body* body;

    ForStatement(VarDec* vd, Exp* ini, Exp* c, Exp* u, Body* b)
        : varInit(vd), init(ini), cond(c), update(u), body(b) {}

    int accept(Visitor* visitor) override;
    ~ForStatement() override {
        delete varInit;
        delete init;
        delete cond;
        delete update;
        delete body;
    }
};


class FunDec {
public:
    string nombre;
    string tipo;
    vector<string> parametros;
    vector<string> tipos;
    Body* cuerpo;
    FunDec(string nombre, string tipo, vector<string> parametros, vector<string> tipos, Body* cuerpo)
        : nombre(nombre), tipo(tipo), parametros(parametros), tipos(tipos), cuerpo(cuerpo) {}

    int accept(Visitor* visitor);
    ~FunDec() {}
};

class Program {
public:
    list<string> includes;
    list<FunDec*> Fundecs;
    Body* mainBody;
    Program(list<string> inc, list<FunDec*> funcs, Body* mainBody)
        : includes(inc), Fundecs(funcs), mainBody(mainBody) {}

    

    int accept(Visitor* visitor);
    ~Program() {}
};

#endif // EXP_H
