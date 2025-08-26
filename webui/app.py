from flask import Flask, render_template, request, send_file, jsonify
import os
import subprocess
from werkzeug.utils import secure_filename

app = Flask(__name__)

UPLOAD_FOLDER = os.path.abspath(os.path.dirname(__file__))
ALLOWED_EXTENSIONS = {'.c', '.txt'}

app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# Utilidades

def allowed_file(filename):
    return os.path.splitext(filename)[1].lower() in ALLOWED_EXTENSIONS

def run_command(cmd, input_file=None):
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, input=input_file)
        return result.stdout, result.stderr, result.returncode
    except Exception as e:
        return '', str(e), 1

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/compile', methods=['POST'])
def compile_code():
    code = request.form['code']
    with open('input.txt', 'w') as f:
        f.write(code)
    # Ejecutar el compilador
    out, err, code_ = run_command('./programa')
    if code_ != 0:
        return jsonify({'error': err or out or 'Error ejecutando el compilador.'})
    # Leer el .s generado
    if not os.path.exists('output.s'):
        return jsonify({'error': 'No se generó output.s'})
    with open('output.s', 'r') as f:
        asm = f.read()
    return jsonify({'asm': asm})

@app.route('/run_s', methods=['POST'])
def run_s():
    # Compilar y ejecutar el .s
    out, err, code_ = run_command('gcc -no-pie -o output output.s')
    if code_ != 0:
        return jsonify({'error': err or out or 'Error compilando output.s'})
    out, err, code_ = run_command('./output')
    if code_ != 0:
        return jsonify({'error': err or out or 'Error ejecutando output'})
    return jsonify({'output': out.strip()})

@app.route('/run_gcc', methods=['POST'])
def run_gcc():
    code = request.form['code']
    with open('temp_gcc.c', 'w') as f:
        f.write(code)
    out, err, code_ = run_command('gcc -O2 -o temp_gcc_exec temp_gcc.c')
    if code_ != 0:
        return jsonify({'error': err or out or 'Error compilando con GCC'})
    out, err, code_ = run_command('./temp_gcc_exec')
    if code_ != 0:
        return jsonify({'error': err or out or 'Error ejecutando GCC'})
    return jsonify({'output': out.strip()})

@app.route('/compare', methods=['POST'])
def compare():
    code = request.form['code']
    # Ejecutar ambos y comparar
    # 1. Compilar y ejecutar con tu compilador
    with open('input.txt', 'w') as f:
        f.write(code)
    out, err, code_ = run_command('./programa')
    if code_ != 0:
        return jsonify({'error': err or out or 'Error ejecutando el compilador.'})
    out, err, code_ = run_command('gcc -no-pie -o output output.s')
    if code_ != 0:
        return jsonify({'error': err or out or 'Error compilando output.s'})
    out_mine, err, code_ = run_command('./output')
    if code_ != 0:
        return jsonify({'error': err or out_mine or 'Error ejecutando output'})
    # 2. Compilar y ejecutar con GCC
    with open('temp_gcc.c', 'w') as f:
        f.write(code)
    out, err, code_ = run_command('gcc -O2 -o temp_gcc_exec temp_gcc.c')
    if code_ != 0:
        return jsonify({'error': err or out or 'Error compilando con GCC'})
    out_gcc, err, code_ = run_command('./temp_gcc_exec')
    if code_ != 0:
        return jsonify({'error': err or out_gcc or 'Error ejecutando GCC'})
    iguales = out_mine.strip() == out_gcc.strip()
    return jsonify({'mine': out_mine.strip(), 'gcc': out_gcc.strip(), 'ok': iguales})

@app.route('/upload', methods=['POST'])
def upload():
    if 'file' not in request.files:
        return jsonify({'error': 'No se envió archivo.'})
    file = request.files['file']
    if file.filename == '':
        return jsonify({'error': 'Nombre de archivo vacío.'})
    if file and allowed_file(file.filename):
        filename = secure_filename(file.filename)
        content = file.read().decode('utf-8')
        return jsonify({'content': content})
    return jsonify({'error': 'Tipo de archivo no permitido.'})

@app.route('/download_s')
def download_s():
    if not os.path.exists('output.s'):
        return 'No se ha generado output.s', 404
    return send_file('output.s', as_attachment=True)

if __name__ == '__main__':
    app.run(debug=True)
