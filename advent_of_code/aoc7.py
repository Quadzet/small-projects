import math
import numpy

def aoc7p2():
    states = { }
    with open("aoc7_data", "r") as f:
        one_line_read = False
        while line := f.readline().strip():
            if one_line_read:
                print_line = [c if i not in states or c in ('^', 'S') else str(states[i]) for i, c in enumerate(line)]
            else:
                print_line = line
            one_line_read = True

            next_line = [c for c in line]
            for i, c in enumerate(line):
                if c == 'S':
                    states[i] = 1
                    next_line[i] = 1
                elif c == '^' and i in states:
                    if i < len(line) - 1:
                        states[i+1] = states[i] if i+1 not in states else states[i+1] + states[i]
                        next_line[i+1] = states[i+1] if next_line[i+1] == '.' else states[i+1] + next_line[i+1]
                    if i > 0:
                        states[i-1] = states[i] if i-1 not in states else states[i-1] + states[i]
                        next_line[i-1] = states[i-1] if next_line[i-1] == '.' else states[i-1] + next_line[i-1]
                    del states[i]
            print(''.join(print_line))

    print(f"Number of beams in final row: {sum([n for _, n in states.items()])}")

def aoc7p1():
    res = 0
    beam_ixs = set()
    with open("aoc7_data", "r") as f:
        while line := f.readline().strip():
            for i, c in enumerate(line):
                if c == 'S':
                    beam_ixs.add(i)
                elif c == '^':
                    if i in beam_ixs:
                        beam_ixs.remove(i)
                        beam_ixs.add(i+1)
                        beam_ixs.add(i-1)
                        res += 1

    print(f"Number of beams in final row: {len(beam_ixs)}")
    print(f"Beam was split {res} times.")

if __name__ == "__main__":
    aoc7p2()

