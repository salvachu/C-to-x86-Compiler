import subprocess, glob, shutil, os

# 1) Patrón de inputs
pattern = "inputs/input*.txt"
INPUTS = sorted(glob.glob(pattern))
print("→ Buscando tests con:", pattern)
print("→ Encontrados   :", INPUTS)

# 2) Nombres de archivos temporales
WORK_S   = "output.s"
WORK_EXE = "temp_exec"
STD_EXE  = "temp_std"

def run_std(inp_path):
    # Compila & ejecuta con GCC
    subprocess.run(
        ["gcc", "-O2", "-x", "c", inp_path, "-o", STD_EXE],
        check=True
    )
    p = subprocess.run([f"./{STD_EXE}"], stdout=subprocess.PIPE, text=True)
    os.remove(STD_EXE)
    return "".join(p.stdout.split())

def run_mine(inp_path):
    # 1) Copia test a input.txt
    shutil.copy(inp_path, "input.txt")

    # 1.5) Elimina output.s de corridas anteriores
    if os.path.exists(WORK_S):
        os.remove(WORK_S)

    # 2) Ejecuta tu compilador (silenciando stdout/stderr)
    subprocess.run(
        ["./programa"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=True
    )

    # 3) Ensambla+linkea con GCC
    subprocess.run(
        ["gcc", "-no-pie", WORK_S, "-o", WORK_EXE],
        check=True
    )

    # 4) Ejecuta y captura
    p = subprocess.run([f"./{WORK_EXE}"], stdout=subprocess.PIPE, text=True)
    os.remove(WORK_EXE)
    return "".join(p.stdout.split())

if __name__ == "__main__":
    if not INPUTS:
        print("❌ No se encontraron archivos de test en inputs/")
        exit(1)

    for inp in INPUTS:
        name = os.path.basename(inp)

        # 1) Salida esperada
        try:
            std_out = run_std(inp)
        except subprocess.CalledProcessError:
            print(f"{name}: ❌ (error compilando referencia)")
            continue

        # 2) Salida de tu compilador
        try:
            mine_out = run_mine(inp)
        except subprocess.CalledProcessError:
            print(f"{name}: ❌ (error en mi_compilador)")
            continue

        # 3) Compara
        if std_out == mine_out:
            print(f"{name}: ✅")
        else:
            print(f"{name}: ❌")
            print(f"  – std : {std_out!r}")
            print(f"  – mine: {mine_out!r}")
