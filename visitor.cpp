#include "exp.h"
#include <stdexcept>
#include <iostream>
#include <string>
#include <algorithm>
#include <climits> 
#include "visitor.h"

TypeInfo UnaryExp::accept(Visitor* visitor) { return visitor->visit(this); }
TypeInfo BinaryExp::accept(Visitor* visitor) { return visitor->visit(this); }
TypeInfo NumberExp::accept(Visitor* visitor) { return visitor->visit(this); }
TypeInfo IdentifierExp::accept(Visitor* visitor) { return visitor->visit(this); }
TypeInfo FCallExp::accept(Visitor* visitor) { return visitor->visit(this); }
int ExpressionStatement::accept(Visitor* visitor) { visitor->visit(this); return 0; }
int PrintStatement::accept(Visitor* visitor) { visitor->visit(this); return 0; }
int IfStatement::accept(Visitor* visitor) { visitor->visit(this); return 0; }
int WhileStatement::accept(Visitor* visitor) { visitor->visit(this); return 0; }
int ForStatement::accept(Visitor* visitor) { visitor->visit(this); return 0; }
int ReturnStatement::accept(Visitor* visitor) { visitor->visit(this); return 0; }
int VarDec::accept(Visitor* visitor) { visitor->visit(this); return 0; }
int Body::accept(Visitor* visitor) { visitor->visit(this); return 0; }
int FunDec::accept(Visitor* visitor) { visitor->visit(this); return 0; }
int Program::accept(Visitor* visitor) { visitor->visit(this); return 0; }

bool esTruncamientoPorTamanio(const std::string& from, const std::string& to) {
    if ((from == "long" && to == "int") ||
        (from == "unsigned long" && (to == "int" || to == "unsigned")) ||
        (from == "long" && to == "unsigned"))
        return true;
    return false;
}

std::string promote(std::string a, std::string b) {
    if (a == b) return a;
    if ((a == "int" && b == "long") || (a == "long" && b == "int")) return "long";
    if ((a == "int" && b == "unsigned") || (a == "unsigned" && b == "int")) return "unsigned";
    if ((a == "long" && b == "unsigned") || (a == "unsigned" && b == "long") ||
        (a == "int" && b == "unsigned long") || (a == "long" && b == "unsigned long") ||
        (a == "unsigned long" && b == "int") || (a == "unsigned long" && b == "long")) return "unsigned long";
    if ((a == "unsigned" && b == "unsigned long") || (a == "unsigned long" && b == "unsigned")) return "unsignedl";
    return "int";
}

int que_es(const std::string& s) {
    if (s.find("(%rbp)") != std::string::npos) return 2;
    if (!s.empty() && std::all_of(s.begin(), s.end(), [](char c){ return std::isdigit(c) || c == '-'; })) return 1;
    return 0;
}

std::string getDominantType(const std::string& a, const std::string& b) {
    if (a == "unsigned long" || b == "unsigned long") return "unsigned long";
    if (a == "long" || b == "long") return "long";
    if (a == "unsigned" || b == "unsigned") return "unsigned";
    return "int";
}

bool Visitor::esTruncamientoPeligroso(const TypeInfo& from, const std::string& to) { return 1; }

void TypeCheckerVisitor::warning(const std::string& msg, bool incluirFuncion) {
    if (incluirFuncion)
        std::cout << "Advertencia: En función '" << currentFunctionName << "': " << msg << std::endl;
    else
        std::cout << "Advertencia: " << msg << std::endl;
    warningCount++;
}

void TypeCheckerVisitor::error(const std::string& msg, bool incluirFuncion) {
    if (incluirFuncion)
        std::cout << "Error: En función '" << currentFunctionName << "': " << msg << std::endl;
    else
        std::cout << "Error: " << msg << std::endl;
    errorCount++;
}

bool TypeCheckerVisitor::hasStdioIncluded() const { return hasStdio; }

void TypeCheckerVisitor::reset() {
    env.clear();
    hasStdio = false;
    errorCount = 0;
    warningCount = 0;
    functionReturnTypes.clear();
    currentFunctionType = "";
}

void TypeCheckerVisitor::visit(PrintStatement* s) {
    s->countFormatSpecifiers();
    for (const auto& i : s->bools) {
        if (!mapaFormatoATipo.count(i)) {
            error("En printf no se acepta '%" + i + "'. Solo se aceptan: " + aceptados, 1);
            return;
        }
    }
    if(s->bools.size() != s->argumentos.size()){
        error("En printf se esperaban " + std::to_string (s->bools.size()) + " argumentos pero se encontraron " + std::to_string(s->argumentos.size()), 1);
        return;
    }
    for (size_t i = 0; i < s->bools.size(); ++i) {
        std::string esperado = mapaFormatoATipo[s->bools[i]]; 
        TypeInfo real = s->argumentos[i]->accept(this); 
        if (real.tipo != esperado) {
            warning("En printf, el argumento " + std::to_string(i+1) +
                    " esperaba tipo '" + esperado + "', pero se recibió '" + real.tipo + "'", true);
        }
    }
}

TypeInfo TypeCheckerVisitor::visit(UnaryExp* e) {
    if (e->op == POST_INC_OP || e->op == POST_DEC_OP) {
        if (!env.check(e->id)) {
            warning("Variable '" + e->id + "' no declarada", true);
            return TypeInfo("int", 0, false);
        }
        TypeInfo info = env.lookup(e->id);
        long valorAnterior = info.valor;
        bool eraConst = info.isConst;
        if (e->op == POST_INC_OP)
            info.valor += 1;
        else
            info.valor -= 1;
        info.isConst = eraConst;
        env.update(e->id, info);
        return TypeInfo(info.tipo, valorAnterior, eraConst);
    }
    if (e->op == PRE_INC_OP || e->op == PRE_DEC_OP) {
        if (!env.check(e->id)) {
            warning("Variable '" + e->id + "' no declarada", true);
            return TypeInfo("int", 0, false);
        }
        TypeInfo info = env.lookup(e->id);
        bool eraConst = info.isConst;
        if (e->op == PRE_INC_OP)
            info.valor += 1;
        else
            info.valor -= 1;
        info.isConst = eraConst;
        env.update(e->id, info);
        return TypeInfo(info.tipo, info.valor, eraConst);
    }
    if (e->expr) {
        TypeInfo t = e->expr->accept(this);
        long valor = (e->op == NEG_OP) ? -t.valor : +t.valor;
        return TypeInfo(t.tipo, valor, t.isConst, t.origen);
    }
    return TypeInfo("int", 0, false);
}

TypeInfo TypeCheckerVisitor::visit(BinaryExp* e) {
    if (e->op == ASSIGN_OP || e->op == PLUS_ASSIGN_OP || e->op == MINUS_ASSIGN_OP) {
        auto idExp = dynamic_cast<IdentifierExp*>(e->left);
        if (!idExp) {
            error("La izquierda de una asignación debe ser una variable (identificador)", true);
            return TypeInfo("int", 0, false);
        }
        std::string nombre = idExp->name;
        if (!env.check(nombre)) {
            warning("Variable '" + nombre + "' no declarada", true);
            return TypeInfo("int", 0, false);
        }
        TypeInfo left = env.lookup(nombre);
        TypeInfo right = e->right->accept(this);
        if (right.tipo == "??") {
            return TypeInfo(left.tipo, 0, false);
        }
        if ((left.tipo == "unsigned" || left.tipo == "unsigned long") && right.isConst && right.valor < 0) {
            warning("Asignación de valor negativo a variable de tipo '" + left.tipo + "'", true);
        }
        long nuevoValor = 0;
        bool isConst = false;
        std::string origen = right.origen;
        switch (e->op) {
            case ASSIGN_OP:
                if (esTruncamientoPorTamanio(right.tipo, left.tipo)) {
                    warning("Asignación con tipos distintos: '" + right.tipo + "' a '" + left.tipo + "'; posible truncamiento", true);
                }
                nuevoValor = right.valor;
                isConst = right.isConst;
                break;
            case PLUS_ASSIGN_OP:
                {
                    std::string tipoResultado = promote(left.tipo, right.tipo);
                    if (esTruncamientoPorTamanio(tipoResultado, left.tipo)) {
                        warning("Asignación con tipos distintos: '" + tipoResultado + "' a '" + left.tipo + "'; posible truncamiento", true);
                    }
                }
                nuevoValor = left.valor + right.valor;
                isConst = left.isConst && right.isConst;
                if (!left.isConst) origen = left.origen;
                break;
            case MINUS_ASSIGN_OP:
                {
                    std::string tipoResultado = promote(left.tipo, right.tipo);
                    if (esTruncamientoPorTamanio(tipoResultado, left.tipo)) {
                        warning("Asignación con tipos distintos: '" + tipoResultado + "' a '" + left.tipo + "'; posible truncamiento", true);
                    }
                }
                nuevoValor = left.valor - right.valor;
                isConst = left.isConst && right.isConst;
                if (!left.isConst) origen = left.origen;
                break;
            default: break;
        }
        env.update(nombre, TypeInfo(left.tipo, nuevoValor, isConst, isConst ? "" : origen));
        return TypeInfo(left.tipo, nuevoValor, isConst, isConst ? "" : origen);
    }
    TypeInfo t1 = e->left->accept(this);
    TypeInfo t2 = e->right->accept(this);
    std::string resultType = promote(t1.tipo, t2.tipo);
    if (t1.isConst && t2.isConst) {
        long result = 0;
        switch (e->op) {
            case PLUS_OP:  result = t1.valor + t2.valor; break;
            case MINUS_OP: result = t1.valor - t2.valor; break;
            case MUL_OP:   result = t1.valor * t2.valor; break;
            case DIV_OP:   result = t2.valor != 0 ? t1.valor / t2.valor : 0; break;
            case LT_OP:    result = t1.valor <  t2.valor; break;
            case LE_OP:    result = t1.valor <= t2.valor; break;
            case EQ_OP:    result = t1.valor == t2.valor; break;
            case GT_OP:    result = t1.valor >  t2.valor; break;
            case GE_OP:    result = t1.valor >= t2.valor; break;
            case NE_OP:    result = t1.valor != t2.valor; break;
            default: break;
        }
        return TypeInfo(resultType, result, true);
    }
    std::string origen = !t1.isConst ? t1.origen : (!t2.isConst ? t2.origen : "");
    return TypeInfo(resultType, 0, false, origen);
}

TypeInfo TypeCheckerVisitor::visit(IdentifierExp* e) {
    if (!env.check(e->name))
        error("Variable '" + e->name + "' no declarada", 1);
    TypeInfo t = env.lookup(e->name);
    t.origen = e->name;
    return t;
}

TypeInfo TypeCheckerVisitor::visit(FCallExp* e) {
    std::vector<TypeInfo> tiposArgs;
    for (Exp* arg : e->argumentos) {
        if (arg)
            tiposArgs.push_back(arg->accept(this));
    }
    if (!functionReturnTypes.count(e->nombre)) {
        error("Función '" + e->nombre + "' no existe", 0);
        return TypeInfo("int", 0, false);
    }
    if (functionParamTypes.count(e->nombre)) {
        const auto& tiposEsperados = functionParamTypes[e->nombre];
        if (tiposEsperados.size() != tiposArgs.size()) {
            error("En función '" + currentFunctionName + "': la función '" + e->nombre +
                  "' espera " + std::to_string(tiposEsperados.size()) + " argumento(s), pero recibió " +
                  std::to_string(tiposArgs.size()), 0);
        } else {
            for (size_t i = 0; i < tiposArgs.size(); ++i) {
                if (tiposArgs[i].tipo != tiposEsperados[i]) {
                    warning("En función '" + currentFunctionName + "': el argumento " +
                            std::to_string(i + 1) + " de '" + e->nombre +
                            "' esperaba '" + tiposEsperados[i] +
                            "', pero recibió '" + tiposArgs[i].tipo + "'", 0);
                }
            }
        }
    }
    std::string tipoRet = functionReturnTypes[e->nombre];
    if (functionReturnValues.count(e->nombre)) {
        TypeInfo info = functionReturnValues[e->nombre];
        return TypeInfo(tipoRet, info.valor, info.isConst);
    }
    return TypeInfo(tipoRet, 0, true);
}

void TypeCheckerVisitor::visit(VarDec* v) {
    for (const auto& p : v->vars) {
        const std::string& id = p.first;
        Exp* init = p.second;
        if (env.checkCurrentLevel(id))
            warning("En función '" + currentFunctionName + "': variable redeclarada: " + id, 0);
        if (functionReturnTypes.count(id))
            error("En función '" + currentFunctionName + "': el nombre '" + id + "' ya fue usado como nombre de función", 0);
        TypeInfo tipoFinal = TypeInfo(v->tipo);
        if (init) {
            TypeInfo tipoInit = init->accept(this);
            if (tipoInit.tipo == "void") {
                error("En función '" + currentFunctionName + "': no se puede inicializar variable '" + id + "' con valor de tipo void.", 0);
            }
            if (!tipoInit.isConst) {
                error("En función '" + currentFunctionName + "': variable '" + id + "' usa la variable '" + tipoInit.origen + "' que no es constante", 0);
            }
            if ((v->tipo == "unsigned" || v->tipo == "unsigned long") && tipoInit.valor < 0) {
                warning("En función '" + currentFunctionName + "': asignación de valor negativo a variable '" + id + "' de tipo '" + v->tipo + "'", 0);
            }
            if (esTruncamientoPorTamanio(tipoInit.tipo, v->tipo)) {
                warning("En función '" + currentFunctionName + "': inicialización de variable '" + id +
                        "' de tipo '" + v->tipo + "' con valor de tipo '" + tipoInit.tipo +
                        (tipoInit.isConst ? (" (" + std::to_string(tipoInit.valor) + ")") : "") +
                        "'; posible truncamiento", 0);
            }
            tipoFinal = TypeInfo(v->tipo, tipoInit.valor, tipoInit.isConst, tipoInit.origen);
        }
        env.add_var(id, tipoFinal);
    }
}

void TypeCheckerVisitor::visit(ReturnStatement* s) {
    hasReturn = true;
    if (!s->e) {
        if (currentFunctionType != "void") {
            error("Espera un valor de retorno (" + currentFunctionType + "), pero no se retornó nada.", 1);
        }
        return;
    }
    TypeInfo tipoRet = s->e->accept(this);
    if (currentFunctionType == "void") {
        error("En función '" + currentFunctionName + "': no se puede retornar un valor en una función void.", 0);
        return;
    }
    if (tipoRet.isConst && esTruncamientoPorTamanio(tipoRet.tipo, currentFunctionType)) {
        warning("En función '" + currentFunctionName + "': valor retornado constante de tipo '" + tipoRet.tipo +
                "' asignado a tipo '" + currentFunctionType +
                "'; posible truncamiento", 0);
    }
    if ((currentFunctionType == "unsigned" || currentFunctionType == "unsigned long") && tipoRet.isConst && tipoRet.valor < 0) {
        warning("En función '" + currentFunctionName + "': se retorna un valor negativo hacia un '" + currentFunctionType + "' posible casteo", 0);
    }
    bool tiposnoCompatibles =(currentFunctionType == "int" && (tipoRet.tipo == "long" || tipoRet.tipo == "unsigned long" )) ||
                            (currentFunctionType == "unsigned" && (tipoRet.tipo == "long" || tipoRet.tipo == "unsigned long" ));
    if (tiposnoCompatibles) {
        warning("En función '" + currentFunctionName + "': tipo retornado '" + tipoRet.tipo +
                "' no coincide con el tipo esperado '" + currentFunctionType + "'", 0);
    }
    if (tipoRet.isConst) {
        functionReturnValues[currentFunctionName] = tipoRet;
    }
}

void TypeCheckerVisitor::visit(FunDec* f) {
    if (functionReturnTypes.count(f->nombre)) {
        error("Redefinición de la función '" + f->nombre + "'",0);
        return;
    }
    if (env.check(f->nombre)) {
        error("El nombre de función '" + f->nombre + "' ya fue usado como variable",0);
        return;
    }
    functionReturnTypes[f->nombre] = f->tipo;
    functionParamTypes[f->nombre] = f->tipos;
    currentFunctionType = f->tipo;
    currentFunctionName = f->nombre;
    hasReturn = false;
    if (f->parametros.size() != f->tipos.size()) {
        error("La función '" + f->nombre + "' tiene distinta cantidad de parámetros y tipos.",0);
        return;
    }
    env.add_level();
    for (size_t i = 0; i < f->parametros.size(); ++i)
        env.add_var(f->parametros[i], TypeInfo(f->tipos[i], 0, true));
    if (f->cuerpo)
        f->cuerpo->accept(this);
    if (currentFunctionType != "void" && f->nombre != "main" && !hasReturn)
        error("La función '" + f->nombre + "' no retorna ningún valor de tipo '" + currentFunctionType + "'",0);
    env.remove_level();
}

void TypeCheckerVisitor::visit(Program* p) {
    hasStdio = false;
    for (const std::string& inc : p->includes) {
        if (inc == "stdio.h") hasStdio = true;
    }
    if (!hasStdio)
        warning("Falta #include<stdio.h>",0);
    for (FunDec* f : p->Fundecs)
        f->accept(this);
    if (p->mainBody) {
        currentFunctionType = "int";
        currentFunctionName = "main";
        hasReturn = false;
        env.add_level();
        p->mainBody->accept(this);
        env.remove_level();
        if (!hasReturn)
            warning("La función 'main' no tiene un return; se asume return 0 por defecto", 0);
    }
}

void TypeCheckerVisitor::visit(ExpressionStatement* s) {
    if (s->expr) {
        s->expr->accept(this);
    }
}

TypeInfo TypeCheckerVisitor::visit(NumberExp* e) { return TypeInfo("int", e->value, true); }

void TypeCheckerVisitor::visit(Body* b) {
    env.add_level();
    for (Stm* stm : b->stms)
        stm->accept(this);
    env.remove_level();
}

void TypeCheckerVisitor::visit(IfStatement* s) {
    TypeInfo cond = s->condition->accept(this);
    if (cond.isConst) {
        if (cond.valor) {
            env.add_level();
            if (s->then) s->then->accept(this);
            env.remove_level();
        } else {
            env.add_level();
            if (s->els) s->els->accept(this);
            env.remove_level();
        }
    } else {
        env.add_level();
        if (s->then) s->then->accept(this);
        env.remove_level();
        env.add_level();
        if (s->els) s->els->accept(this);
        env.remove_level();
    }
}

void TypeCheckerVisitor::visit(WhileStatement* s) {
    TypeInfo cond = s->condition->accept(this);
    if (cond.isConst && cond.valor == 0) {
        warning("En función '" + currentFunctionName + "': bucle 'while' con condición siempre falsa; cuerpo nunca se ejecutará", 0);
    } else {
        if (s->b) s->b->accept(this);
    }
}

void TypeCheckerVisitor::visit(ForStatement* s) {
    env.add_level();
    if (s->varInit)
        s->varInit->accept(this);
    if (s->init)
        s->init->accept(this);
    TypeInfo cond;
    if (s->cond)
        cond = s->cond->accept(this);
    if (s->update)
        s->update->accept(this);
    if (cond.isConst && cond.valor == 0) {
        warning("En función '" + currentFunctionName + "': bucle 'for' con condición siempre falsa; cuerpo nunca se ejecutará", 0);
    } else {
        if (s->body)
            s->body->accept(this);
    }
    env.remove_level();
}

void CodeGenVisitor::emit(const std::string& instruction) {
    if (!firstPass) {
        out << "    " << instruction << "\n";
    }
}

void CodeGenVisitor::emit_op(const std::string& op, const std::string& a, const std::string& b, const std::string& tipo) {
    if (firstPass) return;
    std::string sufijo = (tipo == "int" || tipo == "unsigned") ? "l" : "q";
    bool unary = (op == "neg" || op == "push" || op == "pop" || op == "not");
    auto format_reg = [&](const std::string& r) -> std::string {
        std::string result = r;
        if (result[0] != '%') result = "%" + result;
        if (tipo == "int" || tipo == "unsigned")
            result.replace(1, 1, "e");
        else
            result.replace(1, 1, "r");
        return result;
    };
    int num = que_es(a);
    int num2 = que_es(b);
    if (unary) {
        if (num == 0) {
            emit(op + sufijo + " " + format_reg(a));
        } else {
            emit(op + sufijo + " " + a);
        }
        return;
    }
    std::string src, dst;
    if (num == 1)       src = "$" + a;
    else if (num == 0)  src = format_reg(a);
    else                src = a;
    if (num2 == 0)      dst = format_reg(b);
    else                dst = b;
    emit(op + sufijo + " " + src + ", " + dst);
}

static int countVarDecsInBody(Body* b) {
    int cnt = 0;
    for (Stm* s : b->stms) {
        if (auto vd = dynamic_cast<VarDec*>(s)) {
            cnt += vd->vars.size();
        }
        else if (auto iff = dynamic_cast<IfStatement*>(s)) {
            cnt += countVarDecsInBody(iff->then);
            if (iff->els) cnt += countVarDecsInBody(iff->els);
        }
        else if (auto wh = dynamic_cast<WhileStatement*>(s)) {
            cnt += countVarDecsInBody(wh->b);
        }
        else if (auto fr = dynamic_cast<ForStatement*>(s)) {
            if (fr->varInit) 
                cnt += fr->varInit->vars.size();
            cnt += countVarDecsInBody(fr->body);
        }
    }
    return cnt;
}

void CodeGenVisitor::emit_label(const std::string& label) {
    if (!firstPass) {
        out << label << ":\n";
    }
}

std::string CodeGenVisitor::new_label(const std::string& prefix) {
    return prefix + std::to_string(labelcont++);
}

void CodeGenVisitor::emit_truncation(const std::string& from_type, const std::string& to_type) {
    if (firstPass) return;
    if ((from_type == "long" && to_type == "int") ||
        (from_type == "unsigned long" && to_type == "unsigned")) {
        out << "    # Truncating from " << from_type << " to " << to_type << "\n";
        out << "    movl %eax, %eax\n";
    }
    else if ((from_type == "unsigned long" && to_type == "int") ||
             (from_type == "long" && to_type == "unsigned")) {
        out << "    # Truncating from " << from_type << " to " << to_type << "\n";
        out << "    movl %eax, %eax\n";
    }
}

void CodeGenVisitor::emit_unsigned_conversion(long value, const std::string& target_type) {
    if (firstPass) return;
    if (value < 0 && (target_type == "unsigned" || target_type == "unsigned long")) {
        out << "    # Converting negative value to unsigned\n";
        if (target_type == "unsigned") {
            out << "    movl $" << (unsigned int)value << ", %eax\n";
        } else {
            out << "    movq $" << (unsigned long)value << ", %rax\n";
        }
    }
}

std::string CodeGenVisitor::get_printf_format(const std::string& tipo) {
    if (tipo == "int") return "%d";
    else if (tipo == "long") return "%ld";
    else if (tipo == "unsigned") return "%u";
    else if (tipo == "unsigned long") return "%lu";
    else return "%d";
}

std::string CodeGenVisitor::get_printf_label(const std::string& tipo) {
    if (tipo == "int") return "print_fmt_int";
    else if (tipo == "long") return "print_fmt_long";
    else if (tipo == "unsigned") return "print_fmt_uint";
    else if (tipo == "unsigned long") return "print_fmt_ulong";
    else return "print_fmt_int";
}

void CodeGenVisitor::register_printf_format(const std::string& tipo) {
    formatosUsados.insert(tipo);
    std::cout << "[DEBUG] Registrado formato: " << tipo << std::endl;
}

void CodeGenVisitor::generate_used_formats() {
    if (!formatosUsados.empty()) {
        out << ".data\n";
        for (const std::string& spec : formatosUsados) {
            std::string label = "print_fmt_" + spec;
            out << label << ": .string \"%" << spec << " \\n\"\n";
        }
        out << "\n";
    }
}

void CodeGenVisitor::visit(PrintStatement* s) {
    s->countFormatSpecifiers();
    for (size_t i = 0; i < s->argumentos.size(); ++i) {
        TypeInfo arg = s->argumentos[i]->accept(this);
        std::string spec = s->bools[i];
        formatosUsados.insert(spec);
        if (spec == "d" || spec == "u") {
            emit("movl %eax, %esi");
        } else {
            emit("movq %rax, %rsi");
        }
        std::string label = "print_fmt_" + spec;
        emit("leaq " + label + "(%rip), %rdi");
        emit("movl $0, %eax");
        emit("call printf@PLT");
    }
}

TypeInfo CodeGenVisitor::visit(NumberExp* e) {
    if (e->value >= INT_MIN && e->value <= INT_MAX) {
        emit_op("mov",to_string(e->value),"rax","long");
        return TypeInfo("int", e->value, true);
    } else {
        emit_op("mov",to_string(e->value),"rax","long");
        return TypeInfo("long", e->value, true);
    }
}

TypeInfo CodeGenVisitor::visit(IdentifierExp* e) {
    if (!memoria.count(e->name)) {
        std::cerr << "Variable no encontrada: " << e->name << std::endl;
    }
    int offsetVar = memoria[e->name];
    emit_op("mov",std::to_string(offsetVar) + "(%rbp)","rax",tiposVariables[e->name]);
    return TypeInfo(tiposVariables[e->name], 0, true, e->name);
}

TypeInfo CodeGenVisitor::visit(UnaryExp* e) {
    string tipe = tiposVariables[e->id];
    if (e->op == PRE_INC_OP || e->op == PRE_DEC_OP ||
        e->op == POST_INC_OP|| e->op == POST_DEC_OP) {
        string off = to_string(memoria[e->id]) + "(%rbp)";
        if (e->op == POST_INC_OP || e->op == POST_DEC_OP) {
            emit_op("mov",off,"rax",tipe);
            if (e->op == POST_INC_OP)
                emit_op("inc",off,"rax",tipe);
            else
                emit_op("dec",off,"rax",tipe);
        } 
        else {
            if (e->op == PRE_INC_OP)
                emit_op("inc",off,"rax",tipe);
            else
                emit_op("dec",off,"rax",tipe);
            emit_op("mov",off,"rax",tipe);
        }
        return TypeInfo(tiposVariables[e->id], 0, false, e->id);
    }
    if (e->expr) {
        TypeInfo operand = e->expr->accept(this);
        if (e->op == NEG_OP) {
            emit_op("neg","rax","",tipe);
        }
        return TypeInfo(operand.tipo, -operand.valor, operand.isConst,operand.origen);
    }
    return TypeInfo("int", 0, false);
}

TypeInfo CodeGenVisitor::visit(BinaryExp* e) {
    if (e->op == ASSIGN_OP || e->op == PLUS_ASSIGN_OP || e->op == MINUS_ASSIGN_OP) {
        auto idExp = dynamic_cast<IdentifierExp*>(e->left);
        if (!idExp) {
            if (!firstPass) {
                out << "    # ERROR: Left side of assignment must be identifier\n";
            }
            return TypeInfo("int", 0, false);
        }
        if (firstPass) {
            TypeInfo right = e->right->accept(this);
            return right;
        }
        TypeInfo right = e->right->accept(this);
        std::string tipoDestino = "int";
        if (tiposVariables.count(idExp->name)) {
            tipoDestino = tiposVariables[idExp->name];
        }
        if (right.esNegativo() && TypeInfo(tipoDestino).esTipoSinSigno()) {
            emit_unsigned_conversion(right.valor, tipoDestino);
        }
        emit_truncation(right.tipo, tipoDestino);
        string mem = std::to_string(memoria[idExp->name]) + "(%rbp)";
        string tip = tiposVariables[idExp->name];
        if (memoria.count(idExp->name)) {
            switch (e->op) {
                case ASSIGN_OP:
                    emit_op("mov","rax",mem,tip);
                    break;
                case PLUS_ASSIGN_OP:
                    emit_op("add","rax",mem,tip);
                    break;
                case MINUS_ASSIGN_OP:
                    emit_op("sub","rax",mem,tip);
                    break;
            }
        } else {
            out << "    # ERROR: Variable '" << idExp->name << "' not found for assignment\n";
        }
        return right;
    }
    TypeInfo left = e->left->accept(this);
    emit("pushq %rax");
    TypeInfo right = e->right->accept(this);
    string tipo = getDominantType(right.tipo,left.tipo);
    emit_op("mov", "rax", "rcx", tipo);
    emit_convert("rcx", right.tipo, tipo);
    emit("popq %rax");
    emit_convert("rax", left.tipo, tipo);
    switch (e->op) {
        case PLUS_OP:   emit_op("add", "rcx", "rax", tipo); break;
        case MINUS_OP: emit_op("sub", "rcx", "rax", tipo);break;
        case MUL_OP:   emit_op("imul", "rcx", "rax", tipo); break;
        case DIV_OP:   emit("cqto"); emit("idivq %rcx"); break;
        case LT_OP:    emit("cmpq %rcx, %rax"); emit("movl $0, %eax"); emit("setl %al"); emit("movzbq %al, %rax"); break;
        case LE_OP:    emit("cmpq %rcx, %rax"); emit("movl $0, %eax"); emit("setle %al"); emit("movzbq %al, %rax"); break;
        case EQ_OP:    emit("cmpq %rcx, %rax"); emit("movl $0, %eax"); emit("sete %al"); emit("movzbq %al, %rax"); break;
        case GT_OP:    emit("cmpq %rcx, %rax"); emit("setg %al"); emit("movzbq %al, %rax"); break;
        case GE_OP:    emit("cmpq %rcx, %rax"); emit("movl $0, %eax"); emit("setge %al"); emit("movzbq %al, %rax"); break;
        case NE_OP:    emit("cmpq %rcx, %rax"); emit("movl $0, %eax"); emit("setne %al"); emit("movzbq %al, %rax"); break;
    }
    if(left.esTipoSinSigno() || right.esTipoSinSigno() ){
        string k = "";
        if(left.tipo == "long unsigned" || right.tipo == "long unsigned"){k = left.tipo;}
        else{k = "unsigned";}
        return TypeInfo(k, right.valor + left.valor, true);
    }
    return TypeInfo(tipo, 0, false);
}

TypeInfo CodeGenVisitor::visit(FCallExp* e) {
    std::vector<std::string> argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    if (e->nombre == "printf") {
        return TypeInfo("int", 0, false);
    }
    for (int i = 0; i < e->argumentos.size(); i++) {
        e->argumentos[i]->accept(this);
        emit("movq %rax, " + argRegs[i]);
    }
    emit("call " + e->nombre);
    if (functionReturnTypes.count(e->nombre)) {
        return functionReturnTypes[e->nombre];
    }
    return TypeInfo("int", 0, false);
}

void CodeGenVisitor::visit(ExpressionStatement* s) {
    if (s->expr) {
        TypeInfo result = s->expr->accept(this);
    }
}

void CodeGenVisitor::visit(ReturnStatement* s) {
    if (s->e) {
        TypeInfo ret = s->e->accept(this);
        emit("movq %rax, %r15");
    }
    emit("jmp .end_" + nombreFuncion);
}

void CodeGenVisitor::visit(IfStatement* s) {
    int label = labelcont++;
    std::string elseLabel = "else_" + std::to_string(label);
    std::string endLabel = "endif_" + std::to_string(label);
    s->condition->accept(this);
    emit("cmpq $0, %rax");
    if (s->els) {
        emit("je " + elseLabel);
    } else {
        emit("je " + endLabel);
    }
    if (s->then) s->then->accept(this);
    if (s->els) emit("jmp " + endLabel);
    if (s->els) {
        emit_label(elseLabel);
        s->els->accept(this);
    }
    emit_label(endLabel);
}

void CodeGenVisitor::visit(WhileStatement* s) {
    int label = labelcont++;
    std::string whileLabel = "while_" + std::to_string(label);
    std::string endwhileLabel = "endwhile_" + std::to_string(label);
    emit_label(whileLabel);
    s->condition->accept(this);
    emit("cmpq $0, %rax");
    emit("je " + endwhileLabel);
    if (s->b) s->b->accept(this);
    emit("jmp " + whileLabel);
    emit_label(endwhileLabel);
}

void CodeGenVisitor::visit(ForStatement* s) {
    auto memoriaAnt     = memoria;
    auto tiposAnt       = tiposVariables;
    int  offsetAnt      = offset;
    if (s->varInit) s->varInit->accept(this);
    if (s->init)    s->init->accept(this);
    int label = labelcont++;
    std::string forLabel    = "for_"    + std::to_string(label);
    std::string endforLabel = "endfor_" + std::to_string(label);
    emit_label(forLabel);
    if (s->cond) {
        s->cond->accept(this);
        emit("cmpq $0, %rax");
        emit("je " + endforLabel);
    }
    if (s->body)   s->body->accept(this);
    if (s->update) s->update->accept(this);
    emit("jmp " + forLabel);
    emit_label(endforLabel);
    memoria        = memoriaAnt;
    tiposVariables = tiposAnt;
    offset         = offsetAnt;
}

void CodeGenVisitor::visit(Body* b) {
    for (Stm* stm : b->stms) {
        stm->accept(this);
    }
}

void CodeGenVisitor::second_pass(Program* program) {
    memoria.clear();
    tiposVariables.clear();
    offset = 0;
    labelcont = 0;
    entornoFuncion = false;
    program->accept(this);
}

void CodeGenVisitor::visit(Program* p) {
    if (!firstPass) out << ".text\n";
    for (FunDec* f : p->Fundecs)
        f->accept(this);
    if (p->mainBody) {
        entornoFuncion = true;
        memoria.clear();
        tiposVariables.clear();
        nombreFuncion = "main";
        if (!firstPass) {
            out << ".globl main\n";
            emit_label("main");
            emit("pushq %rbp");
            emit("movq %rsp, %rbp");
            int nLocalsMain = countVarDecsInBody(p->mainBody);
            if (nLocalsMain > 0) {
                out << "    subq $" << (nLocalsMain * 8) << ", %rsp\n";
            }
        }
        offset = 0;
        for (Stm* s : p->mainBody->stms)
            s->accept(this);
        emit_label(".end_main");
        emit("movl $0, %eax");
        emit("leave");
        emit("ret");
        entornoFuncion = false;
    }
    if (!firstPass)
        out << ".section .note.GNU-stack,\"\",@progbits\n";
}

void CodeGenVisitor::visit(FunDec* f) {
    entornoFuncion = true;
    memoria.clear();
    tiposVariables.clear();
    offset = 0;
    nombreFuncion = f->nombre;
    functionReturnTypes[f->nombre] = f->tipo;
    emit_label(f->nombre);
    emit("pushq %rbp");
    emit("movq %rsp, %rbp");
    if (!firstPass) {
        int nLocals = countVarDecsInBody(f->cuerpo) + f->parametros.size();
        if (nLocals > 0) {
            out << "    subq $" << (nLocals * 8) << ", %rsp\n";
        }
    }
    static const std::vector<std::string> argRegs = {
        "%rdi","%rsi","%rdx","%rcx","%r8","%r9"
    };
    for (int i = 0; i < (int)f->parametros.size(); ++i) {
        offset-=8;
        memoria[f->parametros[i]] = offset;
        tiposVariables[f->parametros[i]] = f->tipos[i];
        emit("movq " + argRegs[i] + ", " + std::to_string(memoria[f->parametros[i]]) + "(%rbp)");
    }
    if (f->cuerpo) {
        for (Stm* s : f->cuerpo->stms)
            s->accept(this);
    }
    if (f->tipo == "void")
        emit("jmp .end_" + nombreFuncion);
    emit_label(".end_" + nombreFuncion);
    emit("movq %r15, %rax");
    emit("leave");
    emit("ret");
    entornoFuncion = false;
}

void CodeGenVisitor::visit(VarDec* v) {
    for (auto const& p : v->vars) {
        const std::string& name = p.first;
        Exp* init = p.second;
        offset -= 8;
        memoria[name] = offset;
        tiposVariables[name] = v->tipo;
        if (init) {
            TypeInfo ti = init->accept(this);
            emit_truncation(ti.tipo, v->tipo);
            if (ti.esNegativo() && TypeInfo(v->tipo).esTipoSinSigno())
                emit_unsigned_conversion(ti.valor, v->tipo);
            emit("movq %rax, " + std::to_string(memoria[name]) + "(%rbp)");
        } else {
            emit("movq $0, " + std::to_string(memoria[name]) + "(%rbp)");
        }
    }
}

void CodeGenVisitor::generar(Program* program) {
    first_pass(program);
    generate_used_formats();
    second_pass(program);
}

void CodeGenVisitor::first_pass(Program* program) {
    formatosUsados.clear();
    firstPass = true;
    program->accept(this);
    firstPass = false;
}

void CodeGenVisitor::emit_convert(const std::string& reg, const std::string& from_type, const std::string& to_type) {
    if (firstPass || from_type == to_type) return;
    out << "    # Converting " << reg << " from " << from_type << " to " << to_type << "\n";
    if (from_type == "int" && to_type == "long") {
        if (reg == "rax") out << "    movslq %eax, %rax\n";
        else if (reg == "rcx") {
            out << "    movl %ecx, %ecx\n";
            out << "    movslq %ecx, %rcx\n";
        }
    } else if (from_type == "unsigned" && to_type == "long") {
        if (reg == "rax") out << "    movl %eax, %eax\n";
        else if (reg == "rcx") out << "    movl %ecx, %ecx\n";
    } else if (from_type == "long" && to_type == "int") {
        if (reg == "rax") out << "    movl %eax, %eax\n";
        else if (reg == "rcx") out << "    movl %ecx, %ecx\n";
    } else if (from_type == "unsigned long" && to_type == "unsigned") {
        if (reg == "rax") out << "    movl %eax, %eax\n";
        else if (reg == "rcx") out << "    movl %ecx, %ecx\n";
    } else if (from_type == "unsigned" && to_type == "unsigned long") {
        if (reg == "rax") out << "    movl %eax, %eax\n";
        else if (reg == "rcx") out << "    movl %ecx, %ecx\n";
    }
}

