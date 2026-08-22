"""Script to test the return value for the compiler"""

import random
import subprocess

FILE_PATH = "programs/program.c"

for i in range(100):
    return_value = random.randint(0, 255)
    code_content = f"int main() {{\n    return {return_value};\n}}"
    with open(FILE_PATH, "w", encoding="utf-8") as file:
        file.write(code_content)
    subprocess.run(["make"], check=True)
    run_result = subprocess.run(["./programs/program"], check="True")
    if run_result.returncode == return_value:
        print(f"Iterazione {i+1}: Success (Codice: {return_value})")
    else:
        print(f"Iterazione {i+1}: Mismatch: got {run_result.returncode}, expected {return_value}")
        break
