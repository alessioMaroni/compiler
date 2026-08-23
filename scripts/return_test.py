"""Script to test the return value for the compiler continuously"""

import itertools
import random
import subprocess

FILE_PATH = "programs/program.c"

for i in itertools.count(1):
    return_value = random.randint(0, 255)
    code_content = f"int main() {{\n    return {return_value};\n}}"

    with open(FILE_PATH, "w", encoding="utf-8") as file:
        file.write(code_content)

    subprocess.run(["make"], check=True)

    run_result = subprocess.run(["./programs/program"])

    if run_result.returncode == return_value:
        print(f"Iterazione {i}: Success (Codice: {return_value})")
    else:
        print(
            f"Iterazione {i}: Mismatch! Ottenuto {run_result.returncode},"
            f" atteso {return_value}"
        )
        break