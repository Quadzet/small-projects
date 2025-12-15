def rect_intersects_connection(left, right, top, bot, t1, t2):
    x1, y1 = t1
    x2, y2 = t2

    if x1 == x2:  # vertical
        if (min(y1, y2) < top < max(y1, y2) and left < x1 < right) \
                or (min(y1, y2) < bot < max(y1, y2) and left < x1 < right):
            return True
    elif y1 == y2:  # horizontal
        if (min(x1, x2) < right < max(x1, x2) and bot < y1 < top) \
                or (min(x1, x2) < left < max(x1, x2) and bot < y1 < top):
            return True
    return False


def is_valid(p1, p2, red_tiles, connections):
    x1, y1 = p1
    x2, y2 = p2

    left = min(x1, x2)
    right = max(x1, x2)
    bot = min(y1, y2)
    top = max(y1, y2)

    for t1, t2 in connections:
        if rect_intersects_connection(left, right, top, bot, t1, t2):
            return False

    red_tiles_set = set(red_tiles)

    horizontal_lines = []
    vertical_lines = []

    for t1, t2 in connections:
        if t1[1] == t2[1]:  # horizontal
            y = t1[1]
            if max(t1[0], t2[0]) < left and bot < y < top:
                vertical_lines.append((t1, t2))
            if y <= bot and not (t1[0] > right and t2[0] > right) \
                    and not (t1[0] < left and t2[0] < left):
                horizontal_lines.append((t1, t2))
            elif bot < y < top and not (t1[0] >= right and t2[0] >= right) \
                    and not (t1[0] <= left and t2[0] <= left):
                return False
        elif t1[0] == t2[0]:  # vertical
            x = t1[0]
            if max(t1[1], t2[1]) < bot and left < x < right:
                horizontal_lines.append((t1, t2))
            if x <= left and not (t1[1] > top and t2[1] > top) \
                    and not (t1[1] < bot and t2[1] < bot):
                vertical_lines.append((t1, t2))
            elif left < x < right and not (t1[1] >= top and t2[1] >= top) \
                    and not (t1[1] <= bot and t2[1] <= bot):
                return False

    x_to_check = [x for x in range(left + 1, right)
                  if (x, bot) not in red_tiles_set]
    y_to_check = [y for y in range(bot + 1, top)
                  if (left, y) not in red_tiles_set]

    lines_enc = 0
    for x in x_to_check:
        for t1, t2 in horizontal_lines:
            if min(t1[0], t2[0]) <= x <= max(t1[0], t2[0]):
                lines_enc += 1

    if lines_enc % 2 == 1:
        return False

    lines_enc = 0
    for y in y_to_check:
        for t1, t2 in vertical_lines:
            if min(t1[1], t2[1]) <= y <= max(t1[1], t2[1]):
                lines_enc += 1

    if lines_enc % 2 == 1:
        return False

    print("OK")
    return True


def aoc9p2():
    red_tiles = []
    with open("aoc9_data", "r") as f:
        r = 0
        while line := f.readline().strip():
            x, y = line.split(',')
            red_tiles.append((int(x), int(y)))
            r += 1
    connections = list(zip(red_tiles[:-1], red_tiles[1:]))
    connections.append((red_tiles[-1], red_tiles[0]))

    min_dist = min([abs(x1 - x2 + y1 - y2)
                    for (x1, y1), (x2, y2) in connections])
    print(f"Minimum distance: {min_dist}.")

    rt_matrix = [[(a, b, (abs(a[0] - b[0]) + 1) * (abs(a[1] - b[1]) + 1))
                  for a in red_tiles] for b in red_tiles]
    rt_matrix = sorted([item for line in rt_matrix for item in line],
                       key=lambda x: x[2])

    i = 1
    while rt_matrix:
        rect = rt_matrix.pop()
        print(f"#{i}\tChecking rect {rect[0]}, {rect[1]}. Area: {rect[2]}.")
        if is_valid(rect[0], rect[1], red_tiles, connections):
            print(f"Max: {rect[2]}")
            return
        i += 1
    print("No valid rectangles found.")


def aoc9p1():
    red_tiles = []
    with open("aoc9_data", "r") as f:
        while line := f.readline().strip():
            x, y = line.split(',')
            red_tiles.append((int(x), int(y)))
    print(red_tiles)
    rt_matrix = [[(a, b, (abs(a[0] - b[0]) + 1) * (abs(a[1] - b[1]) + 1))
                  for a in red_tiles] for b in red_tiles]
    max_area = max([i[2] for line in rt_matrix for i in line])
    print(f"Max: {max_area}")


if __name__ == "__main__":
    aoc9p2()
