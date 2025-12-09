import math
import numpy

def aoc6p2():
    result = 0
    lines = []
    with open("aoc6_data", "r") as f:
        while line := f.readline():
            lines.append(line)
        operators = lines[-1]
        transposed = [''.join(chars).strip() for chars in zip(*lines[:-1])]
        operators = [op for op in operators if op.strip()]
        val_ix = 0
        for i, op in enumerate(operators):
            val = 1 if op == '*' else 0
            while n := transposed[val_ix]:
                if op == '*':
                    val *= int(n)
                elif op == '+':
                    val += int(n)
                else:
                    raise ValueError(f"Invalid operator: {op}")
                val_ix += 1
            result += val
            val_ix += 1

    print(f"Sum of results: {result}")

def aoc6p1():
    result = 0
    lines = []
    with open("aoc6_data", "r") as f:
        while line := f.readline().strip():
            entries = line.split(" ")
            lines.append(entries)
        lines = [[entry for entry in line if entry] for line in lines]
        print(f"Lines: {lines}")
        values = numpy.array(lines[:-1])
        operators = lines[-1]
        for i, _ in enumerate(operators):
            print(f"Calc col: {values[:,i]}")
            if lines[-1][i] == '*':
                col_res = math.prod([int(value) for value in values[:, i]])
            elif lines[-1][i] == '+':
                col_res = sum([int(value) for value in values[:, i]])
            else:
                raise ValueError(f"Invalid column: {values[:, i]}")
            print(f"Col res: {col_res}")
            result += col_res

    print(f"Sum of results: {result}")

if __name__ == "__main__":
    aoc6p2()
