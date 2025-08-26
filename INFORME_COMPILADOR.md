# Compilador C con soporte para `long` y `unsigned`

Este proyecto es un compilador para un subconjunto de C, al que se le ha agregado soporte para los tipos extendidos `long`, `unsigned` y combinaciones como `unsigned long`. El objetivo es analizar, verificar tipos y generar código ensamblador para programas escritos en este mini-C, permitiendo trabajar con variables y operaciones de estos tipos, y acercándose a cómo lo haría un compilador real.

## Gramática

La gramática que maneja el compilador es la siguiente:

```
Program     ::= Include+ Func* Main

Include     ::= #include < library >

Func        ::= Type id ( ParamList ) '{' Block '}'

Main        ::= int main ( ) '{' Block '}'

ParamList   ::= ε
              | Param (',' Param)*

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

PrimaryTail ::= '++' | '--'
              | '(' ArgList? ')'
              | ε

ArgList     ::= CExp (',' CExp)*

Type        ::= ['unsigned'] long ['int'] | int | unsigned [int]
```

## Descripción de archivos principales

- `main.cpp`: Punto de entrada, orquesta el proceso de compilación.
- `scanner.h`: Escanea el código fuente y produce los tokens.
- `parser.h`: Parsea los tokens y construye el AST.
- `exp.h`: Define las clases del AST y la estructura `TypeInfo`.
- `visitor.h` y `visitor.cpp`: Implementan el patrón visitor para recorrer el AST, hacer chequeo de tipos y generar código.
- `env.h`: Maneja los entornos de variables y funciones.
- `token.h`: Define los tipos de tokens.
- `no_ambigua.h`: Resuelve ambigüedades en los tokens de tipos.
- `test_runner.py`: Script para correr tests y comparar salidas.
- `correr.txt`: Instrucciones rápidas para compilar y ejecutar.
- `README.md`: Este informe.

## ¿Cómo funciona el compilador? (Resumen y ejemplos)

El compilador sigue el flujo clásico: primero escanea el código fuente, luego lo parsea y construye un AST, realiza chequeo de tipos y finalmente genera código ensamblador. Se han implementado extensiones para soportar tipos como `long`, `unsigned` y combinaciones, manejando correctamente promociones, conversiones y truncamientos.

### Ejemplo 1: Promoción de tipos

```c
int a = 5;
long b = 10;
int c = a + b;
printf("%ld", c);
```

**¿Qué hace el compilador?**

- Detecta que `a` es `int` y `b` es `long`.
- Al hacer `a + b`, promueve `a` a `long` antes de la suma.
- El resultado es `long`, pero se asigna a `int c`, así que se trunca el valor si es necesario.
- En el `printf`, como se usa `%ld`, el compilador se asegura de que el valor se pase como `long`.

### Ejemplo 2: Conversión entre signed y unsigned

```c
unsigned x = 3000000000;
int y = -1;
unsigned z = x + y;
printf("%u", z);
```

**¿Qué hace el compilador?**

- `x` es `unsigned`, `y` es `int`.
- Antes de la suma, `y` se convierte a `unsigned` (lo que en C es una conversión de dos's complement).
- El resultado es `unsigned`, y se almacena en `z`.
- El `printf` usa `%u`, así que el valor se imprime correctamente como unsigned.

### Ejemplo 3: Uso de `unsigned long`

```c
unsigned long big = 4294967296;
printf("%lu", big);
```

**¿Qué hace el compilador?**

- Reconoce el tipo `unsigned long` y lo maneja como un entero de 64 bits.
- El valor se almacena y se imprime usando el formato correcto.

## ¿Qué es TypeInfo y para qué sirve?

`TypeInfo` es una estructura que encapsula la información de tipo de una expresión o variable. Guarda el tipo (`tipo`), el valor si es constante (`valor`), si es constante (`isConst`), y un campo de origen (`origen`). Es fundamental para el chequeo de tipos y para la generación de código, ya que permite saber cómo tratar cada valor (por ejemplo, si hay que hacer conversiones, si es unsigned, etc).

Por ejemplo, cuando se evalúa una expresión binaria, se obtienen los `TypeInfo` de ambos lados y se decide el tipo resultante, si hay que hacer promociones o conversiones, y si hay posibles truncamientos peligrosos.

## Funciones emit: generación de código

### emit_op

Esta función es clave en la generación de código. Se encarga de emitir la instrucción de ensamblador correspondiente a una operación binaria (`+`, `-`, `*`, `/`, etc.) considerando el tipo de los operandos. Según el tipo (`int`, `long`, `unsigned`, etc.), elige la instrucción adecuada (por ejemplo, `addl` para `int`, `addq` para `long`). Además, si los operandos son de tipos distintos, puede llamar a `emit_convert` para hacer la conversión necesaria antes de la operación.

En resumen, `emit_op` traduce una operación de alto nivel a la instrucción de bajo nivel correcta, cuidando los detalles de tipos y registros.

### emit_convert

Esta función se encarga de convertir un valor de un tipo a otro antes de una operación o asignación. Por ejemplo, si tienes un `int` y necesitas un `long`, emite la instrucción para extender el valor correctamente (`movslq` para extender con signo, etc.). También maneja conversiones entre signed y unsigned, y se asegura de que el valor en el registro sea válido para el tipo destino.

`emit_convert` es esencial para evitar errores de interpretación de los datos en el ensamblador, sobre todo cuando se mezclan tipos en las expresiones.

### Otros emit

- `emit`: Emite una instrucción de ensamblador tal cual.
- `emit_label`: Emite una etiqueta (label) para saltos.
- `emit_truncation`: Emite instrucciones para truncar un valor cuando se baja de tipo (por ejemplo, de `long` a `int`).
- `emit_unsigned_conversion`: Convierte un valor a unsigned si es necesario.
- `generate_used_formats`: Genera las cadenas de formato usadas en los `printf`.

Estas funciones ayudan a modularizar la generación de código y a mantener el control sobre los detalles de bajo nivel.

## Métodos visit que retornan TypeInfo

Estos métodos son los que recorren el AST y, en vez de solo ejecutar acciones, devuelven información de tipo (`TypeInfo`). Son fundamentales tanto para el chequeo de tipos como para la generación de código, porque permiten saber el tipo de cada subexpresión y decidir cómo operar con ellas.

Los principales métodos que retornan `TypeInfo` son:

- `visit(UnaryExp* e)`: Evalúa una expresión unaria (`-x`, `++x`, etc.), determina el tipo resultante y emite el código correspondiente. En codegen, puede emitir instrucciones como `neg`, `inc`, etc., y si es un pre/post incremento, también maneja la actualización de la variable.
- `visit(BinaryExp* e)`: Evalúa una expresión binaria (`a + b`, `a < b`, etc.), obtiene los `TypeInfo` de ambos lados, decide el tipo dominante, hace conversiones si es necesario (usando `emit_convert`), y luego llama a `emit_op` para la operación. También maneja asignaciones y operadores compuestos (`+=`, `-=`).
- `visit(NumberExp* e)`: Devuelve el tipo y valor de un número literal. En codegen, carga el valor en un registro.
- `visit(IdentifierExp* e)`: Devuelve el tipo de una variable. En codegen, carga el valor de la variable desde memoria.
- `visit(FCallExp* e)`: Evalúa una llamada a función, chequea los tipos de los argumentos, y devuelve el tipo de retorno de la función. En codegen, prepara los argumentos y emite la llamada.

En la generación de código (`CodeGenVisitor`), estos métodos no solo devuelven el tipo, sino que también emiten el código necesario para calcular el valor de la expresión y dejarlo en el registro adecuado. Así, se encadenan las operaciones y conversiones necesarias para que el ensamblador resultante sea correcto y eficiente.

## Instrucciones para compilar, ejecutar y testear

### Compilar el compilador

```sh
g++ main.cpp visitor.cpp -o programa
```

### Ejecutar el compilador

```sh
./programa
```

Esto toma el archivo `input.txt` como entrada y genera el archivo `output.s` con el código ensamblador.

### Compilar y ejecutar el ensamblador generado

```sh
gcc -no-pie -o output output.s
./output
```

### Probar con el script de tests

El script `test_runner.py` automatiza la comparación entre la salida de tu compilador y la de GCC para todos los archivos en `inputs/`.

```sh
python3 test_runner.py
```

Esto compila y ejecuta cada input tanto con GCC como con tu compilador, y compara las salidas.

---

Si quieres probar un archivo C suelto (por ejemplo, para comparar con GCC):

```sh
gcc xd.c -o o
./o
```

---


