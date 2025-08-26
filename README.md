# Compilador C con soporte extendido: `long` y `unsigned`

Este proyecto es un compilador para un subconjunto de C, con soporte extendido para los tipos `long`, `unsigned` y combinaciones como `unsigned long`. El objetivo es analizar, verificar y generar código ensamblador para programas escritos en este lenguaje, permitiendo trabajar con tipos de datos más allá de los básicos de C estándar. El proyecto incluye un chequeo de tipos robusto y generación de código para operaciones aritméticas, control de flujo y funciones, siguiendo una estructura modular y clara.

## Gramática

```
Program     ::= Include+ Func* Main
Include     ::= #include < library >
Func        ::= Type id ( ParamList ) '{' Block '}'
Main        ::= int main ( ) '{' Block '}'
ParamList   ::= ε | Param (',' Param)*
Block       ::= (VarDec* Stmt*)*
VarDec      ::= Type VarList ';'
VarList     ::= id [ '=' CExp ] (',' id [ '=' CExp ])*
Stmt        ::= CExp ';'
              | printf '(' [string ',' ] CExp (',' CExp)* ')' ';'
              | if '(' CExp ')' '{' Block '}' [ else '{' Block '}' ]
              | while '(' CExp ')' '{' Block '}' 
              | for '(' ForInit ';' CExp ';' CExp ')' '{' Block '}'
              | return CExp ';'
ForInit     ::= Type id | CExp
CExp        ::= AssignExpr
AssignExpr  ::= id '=' AssignExpr
              | id '+=' AssignExpr
              | id '-=' AssignExpr
              | RelExpr
RelExpr     ::= AddExpr ( RelOp AddExpr )*
RelOp       ::= '<' | '<=' | '==' | '>' | '>=' | '!='
AddExpr     ::= MulExpr (('+' | '-') MulExpr)*
MulExpr     ::= UnaryExpr (('*' | '/') UnaryExpr)*
UnaryExpr   ::= PreIncDec | PostIncDec | Primary
PreIncDec   ::= '++' id | '--' id
PostIncDec  ::= id '++' | id '--'
Primary     ::= id PrimaryTail
              | Num
              | '+' Primary
              | '-' Primary
              | '(' CExp ')'
PrimaryTail ::= '++' | '--'            // Post increment/decrement
              | '(' ArgList? ')'       // Llamada a función
              | ε                     // Simple variable (IdentifierExp)
ArgList     ::= CExp (',' CExp)*
Type        ::= ['unsigned'] long ['int'] | int | unsigned [int]
```

## Descripción de archivos principales

- `main.cpp`: Punto de entrada del compilador, orquesta el flujo general.
- `scanner.h`: Tokeniza el código fuente en una secuencia de tokens.
- `parser.h`: Implementa el parser recursivo descendente según la gramática.
- `exp.h`: Define las clases del AST (nodos de expresiones, sentencias, etc.) y la estructura `TypeInfo`.
- `visitor.h` / `visitor.cpp`: Implementan los visitors para chequeo de tipos y generación de código.
- `env.h`: Maneja los entornos de variables y funciones (scoping).
- `token.h`: Define la clase `Token` y los tipos de token.
- `no_ambigua.h`: Normaliza secuencias de tokens ambiguas (por ejemplo, `unsigned long int`).
- `test_runner.py`: Script para correr tests automáticos comparando la salida del compilador con GCC.

## TypeInfo: ¿Qué es y para qué sirve?

`TypeInfo` es una estructura que encapsula la información de tipo de una expresión o variable. Incluye:
- `tipo`: El tipo concreto ("int", "long", "unsigned", "unsigned long").
- `valor`: Valor constante si es conocido (para propagación de constantes).
- `isConst`: Indica si el valor es constante.
- `origen`: Información adicional sobre el origen del tipo (útil para warnings o conversiones).

Sirve para propagar y verificar tipos a lo largo del AST, permitiendo detectar errores de tipo, conversiones peligrosas y optimizar la generación de código. Es fundamental tanto en el chequeo de tipos como en la generación de código, ya que permite decidir cómo operar y convertir valores correctamente.

## Generación de código: `emit_op`, `emit_convert` y otros métodos `emit`

### `emit_op`
Este método genera la instrucción de ensamblador correspondiente a una operación binaria (suma, resta, multiplicación, división, comparaciones, etc.) considerando el tipo de los operandos. Se encarga de:
- Elegir la instrucción adecuada según el tipo (por ejemplo, operaciones entre `int` vs `long`).
- Realizar conversiones si los operandos son de tipos distintos (promoción de tipos).
- Emitir el código ensamblador que realiza la operación y almacena el resultado en el registro o variable correspondiente.

Es clave para que el código generado sea correcto y eficiente, respetando las reglas de C sobre promoción y conversión de tipos.

### `emit_convert`
Este método se encarga de generar el código necesario para convertir un valor de un tipo a otro (por ejemplo, de `int` a `long`, o de `unsigned` a `int`).
- Detecta si la conversión es necesaria y si puede implicar pérdida de información (truncamiento).
- Emite instrucciones de conversión o extensión de signo según corresponda.
- Es utilizado antes de operaciones o asignaciones donde los tipos no coinciden exactamente.

La correcta implementación de `emit_convert` es fundamental para evitar errores sutiles en la ejecución del código generado, especialmente cuando se mezclan tipos con y sin signo o de distinto tamaño.

### Otros métodos `emit`
- `emit`: Emite una instrucción de ensamblador genérica.
- `emit_label`: Emite una etiqueta (label) para saltos y control de flujo.
- `emit_truncation`: Emite instrucciones para truncar valores cuando se reduce el tamaño del tipo.
- `emit_unsigned_conversion`: Convierte valores a tipos sin signo.

Estos métodos ayudan a modularizar la generación de código y mantener el control sobre los detalles de bajo nivel.

## Métodos `visit` que retornan `TypeInfo` (Chequeo de tipos y CodeGen)

Los métodos `visit` que retornan `TypeInfo` son los encargados de recorrer el AST y determinar el tipo resultante de cada expresión. Son fundamentales tanto en el chequeo de tipos como en la generación de código.

### En el chequeo de tipos (`TypeCheckerVisitor`):
- `visit(UnaryExp*)`: Determina el tipo resultante de una operación unaria (por ejemplo, `-x`, `++x`).
- `visit(BinaryExp*)`: Calcula el tipo resultante de una operación binaria, aplicando reglas de promoción y verificando compatibilidad.
- `visit(NumberExp*)`: Retorna el tipo del número literal (por defecto `int`, pero puede variar según el contexto).
- `visit(IdentifierExp*)`: Busca el tipo de una variable en el entorno.
- `visit(FCallExp*)`: Verifica el tipo de retorno de la función y los tipos de los argumentos.

Estos métodos permiten detectar errores de tipo antes de la generación de código y asegurar que las operaciones sean válidas.

### En la generación de código (`CodeGenVisitor`):
- Los mismos métodos (`visit(UnaryExp*)`, `visit(BinaryExp*)`, etc.) no solo determinan el tipo, sino que además generan el código ensamblador necesario para calcular el valor de la expresión.
- Se encargan de:
  - Emitir instrucciones para evaluar operandos.
  - Realizar conversiones de tipo si es necesario (usando `emit_convert`).
  - Guardar el resultado en el registro o variable adecuada.
  - Asegurar que el tipo resultante sea el correcto para el contexto donde se usa.

En resumen, los métodos `visit` que retornan `TypeInfo` son el núcleo de la semántica del compilador: propagan tipos, detectan errores y generan el código correcto para cada expresión, respetando las reglas de C y las extensiones implementadas.

---

