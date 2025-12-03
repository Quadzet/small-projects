def aoc1p1():
    res = 0
    n = 50
    l = 0
    with open(f"aoc1_data", "r") as file:
        while turn := file.readline():
            l += 1
            if turn[0] == 'L':
                n -= int(turn[1:])
            elif turn[0] == 'R':
                n += int(turn[1:])
            else:
                raise ValueError(f"Invalid line: {turn}")

            n = n%100
            if n == 0:
                res += 1
            # print(f"Line #{l}: {turn.strip()} pointing at {n}")
    print(f"Result: {res}")

def aoc1p2():
    res = 0
    n = 50
    l = 0
    with open(f"aoc1_data", "r") as file:
        while turn := file.readline():
            l += 1
            if turn[0] == 'L':
                n -= int(turn[1:])
            elif turn[0] == 'R':
                n += int(turn[1:])
            else:
                raise ValueError(f"Invalid line: {turn}")

            if n < 0:
                res += abs(n // 100)
                # We started at 0, don't add one for 'passing' 0
                if n == -1 * int(turn[1:]):
                    res -= 1
                n = n % 100
                if n == 0:
                    res += 1
            elif n > 0:
                res += abs(n // 100)
                n = n % 100
            else:
                res += 1




            print(f"Line #{l}: {turn.strip()},  pointing at {n}. Res: {res}")
    print(f"Result: {res}")




if __name__ == "__main__":
    aoc1p2()
