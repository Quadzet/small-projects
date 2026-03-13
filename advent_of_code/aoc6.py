import math
    
# === NOTES ===
# 
# Transpose by treating each line as a list of chars, then zipping
# them together to create lists of all the first chars, seconds chars, etc.
# [
#   '123 328  51 64 ',
#   ' 45 64  387 23 ',
#   '  6 98  215 314',
# ]
#   ==>
# [
#   '1',   '24',  '356', '',
#   '369', '248', '8',   '',
#   '32',  '581', '175', '',
#   '623', '431', '4',   '',
# ]
# Note that each column in the data is separated by a space, we use that
# to determine when the column ends, since the stripped value is ''.

def aoc6p2():
    result = 0
    lines = []
    with open("aoc6_data", "r") as f:
        while line := f.readline():
            lines.append(line)

    transposed = [''.join(chars).strip() for chars in zip(*lines[:-1])]
    operators = [op for op in lines[-1] if op.strip()] # ' *  +\n' -> ['*', '+']

    val_ix = 0
    for op in operators:
        col_val = 1 if op == '*' else 0
        # Stop at each empty string, which indicates the end of the column.
        while n := transposed[val_ix]:
            if op == '*':
                col_val *= int(n)
            elif op == '+':
                col_val += int(n)
            else:
                raise ValueError(f"Invalid operator: {op}")
            val_ix += 1
        result += col_val
        val_ix += 1

    print(f"Sum of results: {result}")

def aoc6p1():
    result = 0
    lines = []
    with open("aoc6_data", "r") as f:
        while line := f.readline().strip():
            words = line.split(" ")
            lines.append(words)
    lines = [[word for word in line if word] for line in lines]

    values = lines[:-1]
    operators = lines[-1]
    for col_ix, op in enumerate(operators):
        col_vals = [row[col_ix] for row in values]
        if op == '*':
            col_res = math.prod([int(value) for value in col_vals])
        elif op == '+':
            col_res = sum([int(value) for value in col_vals])
        else:
            raise ValueError(f"Invalid column: {col_vals}")
        result += col_res

    print(f"Sum of results: {result}")

if __name__ == "__main__":
    aoc6p2()
