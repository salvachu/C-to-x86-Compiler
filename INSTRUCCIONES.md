# Guía rápida para correr el proyecto

## 1. Abrir WSL
Abre tu terminal de Ubuntu (WSL) desde el menú de Windows o usando `wsl` en la terminal de Windows.

## 2. Instalar dependencias necesarias

### Dependencias del sistema
- Compilador GCC y G++:
  ```bash
  sudo apt update
  sudo apt install build-essential
  ```
- Python 3 y pip:
  ```bash
  sudo apt install python3 python3-pip
  ```
- Para entornos virtuales de Python:
  ```bash
  sudo apt install python3-venv
  ```

### Dependencias de Python para la interfaz web
- Flask y Werkzeug:
  ```bash
  pip3 install flask werkzeug
  ```

## 3. Compilar el compilador
Desde la carpeta raíz del proyecto:
```bash
cd /ruta/a/tu/proyecto/proyecto_mio
# Compila el ejecutable
g++ main.cpp visitor.cpp -o programa
```

## 4. Probar con el script de test_runner
Puedes correr los tests automáticos para comparar la salida de tu compilador con GCC:
```bash
python3 test_runner.py
```
Esto buscará los archivos de entrada en la carpeta `inputs/` y mostrará si la salida de tu compilador coincide con la de GCC.

## 5. Usar la interfaz web

### a) Crear y activar un entorno virtual (opcional)
```bash
cd webui
python3 -m venv venv
source venv/bin/activate
```

### b) Instalar dependencias en el entorno virtual
```bash
pip install flask werkzeug
```

### c) Asegúrate de tener el ejecutable `programa` en la carpeta `webui`
Si no está, desde la raíz del proyecto:
```bash
cp programa webui/
cd webui
chmod +x programa
```

### d) Ejecutar la interfaz web
Desde la carpeta `webui`:
```bash
python3 app.py
```
Abre tu navegador y entra a `http://localhost:5000`.

### e) Usar la interfaz
- Escribe o sube tu código C en el editor.
- Usa los botones para compilar, ejecutar, comparar salidas, o guardar el ensamblador.

---


