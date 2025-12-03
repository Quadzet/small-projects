def aoc3p1():
    res = 0
    with open("aoc3_data", "r") as f:
        while l := f.readline().strip():
            ints = [int(c) for c in l]
            digit_1 = max(ints[0:-1])
            digit_1_ix = ints.index(digit_1)
            digit_2 = max(ints[digit_1_ix + 1:])
            res += digit_1 * 10 + digit_2
            print(f"Max value of line '{l}' is {digit_1 * 10 + digit_2}.")
    print(f"Sum: {res}")

def aoc3p2():
    res = 0
    with open("aoc3_data", "r") as f:
        while l := f.readline().strip():
            ints = [int(c) for c in l]
            ix = 0
            line_res = ""
            for i in range(0, 12):
                if i < 11:
                    digit = max(ints[ix:-12+i+1])
                else:
                    digit = max(ints[ix:])
                ix += ints[ix:].index(digit) + 1
                line_res += str(digit)
            res += int(line_res)
            print(f"Max value of line '{l}' is {line_res}.")

    print(f"Sum: {res}")

if __name__ == "__main__":
    aoc3p2()
