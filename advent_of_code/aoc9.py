from math import dist, prod
import numpy as np

log_interval = 1
call = 0

def is_valid(p1, p2, red_tiles, connections):
    global call, log_interval
    call += 1
    # print(f"p1: {p1}, p2: {p2}")
    x1, y1 = p1
    x2, y2 = p2

    if call >= log_interval:
        print(f"Calc number {call}.")
        log_interval *= log_interval

    left = min(x1, x2)
    right = max(x1, x2)
    bot = min(y1, y2)
    top = max(y1, y2)

    x_max = max([x for x, _ in red_tiles])
    y_max = max([y for _, y in red_tiles])

    # ..............
    # .......#XXX#..
    # .......XXXXX..
    # ..OOOOOOOOXX..
    # ..OOOOOOOOXX..
    # ..OOOOOOOOXX..
    # .........XXX..
    # .........#X#..
    # ..............
    # Can be combined/optimized ofc
    # (9, 5), (2, 3)

    """
    for x in range(left + 1, right):
        lines_enc = 0
        if (x, bot) in red_tiles:
            continue
        for y in range(0, bot+1):
            for t1, t2 in connections:
                if min(t1[0], t2[0]) <= x <= max(t1[0], t2[0]) and t1[1] == y == t2[1]:
                    lines_enc += 1
    """

    lines_enc = len([1 for t1, t2 in connections for x in range(left+1, right) for y in range(0, bot+1)
    if (x, bot) not in red_tiles and min(t1[0], t2[0]) <= x <= max(t1[0], t2[0]) and t1[1] == y == t2[1]])

    if lines_enc % 2 != 1:
        return False

    lines_enc = len([1 for t1, t2 in connections for x in range(left+1, right) for y in range(bot+1, top)
    if (x, top) not in red_tiles and min(t1[0], t2[0]) <= x <= max(t1[0], t2[0]) and t1[1] == y == t2[1]])

    if lines_enc % 2 != 0:
        return False

    lines_enc = len([1 for t1, t2 in connections for y in range(bot+1, top) for x in range(0, left+1)
    if (left, y) not in red_tiles and min(t1[1], t2[1]) <= y <= max(t1[1], t2[1]) and t1[0] == x == t2[0]])

    if lines_enc % 2 != 1:
        return False

    lines_enc = len([1 for t1, t2 in connections for y in range(bot+1, top) for x in range(left+1, right)
    if (right, y) not in red_tiles and min(t1[1], t2[1]) <= y <= max(t1[1], t2[1]) and t1[0] == x == t2[0]])

    if lines_enc % 2 != 0:
        return False

    return True

def aoc9p2():
    red_tiles = [ ]
    with open("aoc9_data", "r") as f:
        r = 0
        while line := f.readline().strip():
            x, y = line.split(',')
            red_tiles.append((int(x), int(y)))
            r += 1
    connections = list(zip(red_tiles[:-1], red_tiles[1:]))
    connections.append((red_tiles[-1], red_tiles[0]))

    min_dist = min([abs(x1 - x2 + y1 - y2) for (x1, y1), (x2, y2) in connections])
    print(f"Minimum distance: {min_dist}.")

    rt_matrix = [[(a, b, (abs(a[0] - b[0])+1)*(abs(a[1] - b[1])+1)) for a in red_tiles] for b in red_tiles]
    # is_valid((9, 5), (2, 3), red_tiles, connections)
    # return
    max_area = max([i[2] for line in rt_matrix for i in line if is_valid(i[0], i[1], red_tiles, connections)])

    print(f"Max: {max_area}")

def aoc9p1():
    red_tiles = [ ]
    with open("aoc9_data", "r") as f:
        while line := f.readline().strip():
            x, y = line.split(',')
            red_tiles.append((int(x), int(y)))
    print(red_tiles)
    rt_matrix = [[(a, b, (abs(a[0] - b[0])+1)*(abs(a[1] - b[1])+1)) for a in red_tiles] for b in red_tiles]
    max_area = max([i[2] for line in rt_matrix for i in line])
    print(f"Max: {max_area}")

if __name__ == "__main__":
    aoc9p2()

