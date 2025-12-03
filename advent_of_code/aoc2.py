import re

def aoc2p1():
    sum_invalid = 0
    with open("aoc2_data") as file:
        for range_str in file.read().split(","):
            start, end = range_str.split("-")
            for n in range(int(start), int(end) + 1):
                strlen = len(str(n))
                if len(str(n)) % 2 != 0:
                    continue
                pattern = r"(\d{" + str(strlen // 2) + r"})\1"
                if re.fullmatch(pattern, str(n)):
                    sum_invalid += n
                    print(f"{n} is invalid.")
    print(f"The sum of all invalid numbers is {sum_invalid}.")

def aoc2p2():
    sum_invalid = 0
    with open("aoc2_data") as file:
        for range_str in file.read().split(","):
            start, end = range_str.split("-")
            for n in range(int(start), int(end) + 1):
                strlen = len(str(n))
                # Could prime factor also, with memoization
                for i in range(1, strlen // 2 + 1):
                    pattern = r"^(\d{" + str(i) + r"})(\1)+$"
                    if re.fullmatch(pattern, str(n)):
                        sum_invalid += n
                        print(f"{n} is invalid.")
                        break
    print(f"The sum of all invalid numbers is {sum_invalid}.")


if __name__ == "__main__":
    aoc2p2()
