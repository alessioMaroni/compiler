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
        print(f"Iteration {i}: Success (Code: {return_value})")
    else:
        print(
            f"Iteration {i}: Mismatch! Got {run_result.returncode},"
            f" expected {return_value}"
        )
        break