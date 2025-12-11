from math import dist, prod
import numpy as np

def aoc8p1():
    red_tiles = [ ]
    with open("aoc9_data", "r") as f:
        r = 0
        while line := f.readline().strip():
            # red_tiles.append((r, i) for i, c in enumerate(line) if c == '#')
            # for item in [(r, i) for i, c in enumerate(line) if c == '#']:
                # red_tiles.append(item)
            x, y = line.split(',')
            red_tiles.append((int(x), int(y)))
            r += 1
    print(red_tiles)
    rt_matrix = [[(a, b, (abs(a[0] - b[0])+1)*(abs(a[1] - b[1])+1)) for a in red_tiles] for b in red_tiles]
    max_area = max([i[2] for line in rt_matrix for i in line])
    print(f"Max: {max_area}")
    # print('\n'.join([''.join(str(c[2])) for line in rt_matrix for c in line]))



if __name__ == "__main__":
    aoc8p1()

