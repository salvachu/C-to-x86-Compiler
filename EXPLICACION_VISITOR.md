# Explicación detallada de Visitor, CodeGen y TypeInfo

Este documento explica en profundidad cómo funciona el patrón Visitor en el compilador, el rol de `TypeInfo`, la generación de código y el manejo del stack, con diagramas y ejemplos para clarificar cada parte.

---

## 1. ¿Qué es el patrón Visitor en este compilador?

El patrón Visitor permite separar la lógica de operaciones (como chequeo de tipos o generación de código) de la estructura de datos del AST (árbol de sintaxis abstracta). Así, puedes tener distintos "recorridos" sobre el mismo árbol, cada uno con un propósito distinto.

### Clases principales:
- **Visitor**: Clase base abstracta. Define métodos `visit` para cada tipo de nodo del AST.
- **TypeCheckerVisitor**: Implementa los métodos `visit` para hacer chequeo de tipos.
- **CodeGenVisitor**: Implementa los métodos `visit` para generar código ensamblador.

Cada nodo del AST (por ejemplo, `BinaryExp`, `NumberExp`, `IdentifierExp`, etc.) tiene un método `accept(visitor)` que llama al método `visit` correspondiente.

**Ejemplo de flujo:**
```
BinaryExp.accept(visitor) → visitor.visit(BinaryExp*)
```

---

## 2. ¿Qué es TypeInfo y para qué sirve?

`TypeInfo` es una estructura que encapsula la información de tipo de una expresión o variable:
- `tipo`: El tipo concreto ("int", "long", "unsigned", "unsigned long").
- `valor`: Valor constante si es conocido.
- `isConst`: Indica si el valor es constante.
- `origen`: Información adicional sobre el origen del tipo.

**¿Por qué es útil?**
- Permite propagar y verificar tipos a lo largo del AST.
- Es clave para decidir conversiones, promociones y detectar errores de tipo.
- En generación de código, ayuda a saber qué instrucciones usar y cómo manejar los registros.

---

## 3. ¿Cómo funciona la generación de código? (CodeGenVisitor)

El `CodeGenVisitor` recorre el AST y, en cada nodo, emite instrucciones de ensamblador usando funciones auxiliares como `emit`, `emit_op`, `emit_convert`, etc.

### Principales funciones de generación de código:

- **emit**: Emite una instrucción de ensamblador tal cual.
- **emit_label**: Emite una etiqueta para saltos y control de flujo.
- **emit_op**: Emite la instrucción de operación aritmética o lógica adecuada según el tipo (por ejemplo, `addl` para int, `addq` para long).
- **emit_convert**: Emite instrucciones para convertir un valor de un tipo a otro (por ejemplo, de int a long).
- **emit_truncation**: Emite instrucciones para truncar valores cuando se reduce el tamaño del tipo.
- **emit_unsigned_conversion**: Convierte valores a tipos sin signo si es necesario.

### Ejemplo de flujo para una suma:
1. Se visitan los operandos izquierdo y derecho (recursivamente).
2. Se obtiene el `TypeInfo` de ambos.
3. Si los tipos no coinciden, se llama a `emit_convert` para igualarlos.
4. Se llama a `emit_op` para emitir la suma.
5. El resultado queda en el registro adecuado.

---

## 4. ¿Cómo se maneja el stack y los registros?

### Prologo y epilogo de función

```asm
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $N, %rsp   ; Reserva espacio para variables locales
    ...
.end_main:
    movl $0, %eax
    leave
    ret
```

- El stack crece hacia abajo.
- Las variables locales se almacenan en offsets negativos respecto a `%rbp`.
- Antes de llamar a funciones como `printf`, el stack debe estar alineado a 16 bytes.

### Diagrama del stack en una función

```
+-------------------+  <-- %rbp+0
| Dirección retorno |
+-------------------+
|  %rbp anterior    |
+-------------------+  <-- %rbp
| var local 1       |
+-------------------+
| var local 2       |
+-------------------+
| ...               |
+-------------------+  <-- %rsp
```

### Ejemplo de llamada a printf

Para `printf("%ld\n", z);`:
- `%rdi` = dirección del string de formato
- `%rsi` = valor de z (tipo long)

Antes de `call printf@PLT`, el stack debe estar alineado a 16 bytes.

---

## 5. Diagrama de flujo de los visitors

```mermaid
graph TD;
    A[AST Nodo] -->|accept(visitor)| B[Visitor]
    B -->|visit(BinaryExp*)| C[CodeGenVisitor]
    B -->|visit(BinaryExp*)| D[TypeCheckerVisitor]
    C -->|emit_op, emit_convert| E[Genera código]
    D -->|Propaga TypeInfo| F[Chequea tipos]
```

---

## 6. Resumen visual de las funciones emit

| Función         | Propósito                                      |
|-----------------|------------------------------------------------|
| emit            | Emite instrucción ensamblador genérica          |
| emit_label      | Emite etiqueta para saltos                      |
| emit_op         | Emite operación aritmética/lógica según tipo    |
| emit_convert    | Convierte entre tipos (int, long, unsigned, ...) |
| emit_truncation | Trunca valor al reducir tamaño de tipo          |
| emit_unsigned_conversion | Convierte a unsigned si es necesario    |

---

## 7. Ejemplo completo: suma y printf

Supón que tienes:
```c
long a = 10;
int b = 5;
long c = a + b;
printf("%ld\n", c);
```

**Flujo:**
- Se visitan los nodos de asignación y suma.
- Se promueve `b` a `long` antes de la suma (`emit_convert`).
- Se emite la suma con `emit_op` (usa `addq` para long).
- Se almacena el resultado en la variable `c`.
- Para el `printf`, se carga el formato en `%rdi` y el valor de `c` en `%rsi`, se alinea el stack y se llama a `printf@PLT`.

---

## 8. Explicación detallada de las funciones emit (paso a paso)

### emit
- **¿Qué hace?**
  - Es la función más básica: simplemente escribe una línea de ensamblador en el archivo de salida.
- **¿Cuándo se usa?**
  - Para cualquier instrucción que no requiera lógica especial, por ejemplo: `movq $0, %rax`.
- **Ejemplo:**
  ```cpp
  emit("movq $0, %rax");
  ```

### emit_label
- **¿Qué hace?**
  - Escribe una etiqueta en el ensamblador, útil para saltos y control de flujo.
- **¿Cuándo se usa?**
  - Antes de un bloque de código al que se va a saltar (por ejemplo, el inicio o fin de un bucle o un if).
- **Ejemplo:**
  ```cpp
  emit_label(".end_main"); // genera la línea: .end_main:
  ```

### emit_op
- **¿Qué hace?**
  - Emite la instrucción de operación aritmética o lógica adecuada según el tipo de los operandos.
- **¿Cómo funciona?**
  1. Recibe el operador (por ejemplo, `add`, `sub`), los registros/valores a operar y el tipo de dato.
  2. Elige la instrucción correcta según el tipo:
     - Para `int` usa instrucciones de 32 bits (`addl`, `subl`, ...).
     - Para `long` usa instrucciones de 64 bits (`addq`, `subq`, ...).
  3. Escribe la instrucción ensamblador con los registros correctos.
- **Ejemplo:**
  ```cpp
  emit_op("add", "%eax", "%ebx", "int"); // genera: addl %ebx, %eax
  emit_op("add", "%rax", "%rbx", "long"); // genera: addq %rbx, %rax
  ```
- **¿Por qué es importante?**
  - Permite que las operaciones sean correctas para cada tipo de dato, evitando errores de tamaño o signo.

### emit_convert
- **¿Qué hace?**
  - Emite las instrucciones necesarias para convertir un valor de un tipo a otro antes de una operación o asignación.
- **¿Cómo funciona?**
  1. Recibe el registro origen, el tipo actual y el tipo destino.
  2. Si el tipo destino es más grande (por ejemplo, de `int` a `long`), emite una extensión de signo (`movslq`) o extensión cero (`movl` a `movq`).
  3. Si el tipo destino es más pequeño, puede emitir una truncación (ver `emit_truncation`).
  4. Si hay conversión entre signed y unsigned, puede requerir instrucciones adicionales.
- **Ejemplo:**
  ```cpp
  emit_convert("%eax", "int", "long"); // genera: movslq %eax, %rax
  ```
- **¿Por qué es importante?**
  - Garantiza que los valores tengan el formato correcto antes de operar, evitando bugs sutiles.

### emit_truncation
- **¿Qué hace?**
  - Emite instrucciones para truncar un valor cuando se reduce el tamaño del tipo (por ejemplo, de `long` a `int`).
- **¿Cómo funciona?**
  1. Recibe el tipo origen y destino.
  2. Emite una instrucción que copia solo los bits bajos (por ejemplo, `movl %eax, %eax` para truncar a 32 bits).
- **Ejemplo:**
  ```cpp
  emit_truncation("long", "int"); // genera: movl %eax, %eax
  ```
- **¿Por qué es importante?**
  - Evita que queden valores basura en los bits altos cuando se baja de tipo.

### emit_unsigned_conversion
- **¿Qué hace?**
  - Convierte un valor a unsigned si es necesario, asegurando que la interpretación sea correcta.
- **¿Cómo funciona?**
  - Puede usar instrucciones como `mov` o `and` para limpiar el bit de signo.
- **Ejemplo:**
  ```cpp
  emit_unsigned_conversion(3000000000, "unsigned");
  ```
- **¿Por qué es importante?**
  - En operaciones entre signed y unsigned, asegura que el resultado sea el esperado según las reglas de C.

---

**En resumen:**
- Cada función `emit` tiene un propósito específico en la generación de código.
- Usarlas correctamente garantiza que el ensamblador generado sea válido, eficiente y seguro para todos los tipos soportados por el compilador.

---

Este documento te da una visión global y detallada de cómo funciona el visitor, la generación de código, el manejo de tipos y el stack en tu compilador. Si quieres ejemplos de código más específicos o diagramas adicionales, dime y los agrego.
