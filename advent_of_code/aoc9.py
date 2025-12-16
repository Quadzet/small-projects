from collections import namedtuple
import time

point = namedtuple('point', ['x', 'y'])
rectangle = namedtuple('rectangle', ['left', 'right', 'top', 'bottom'])


def line_intersects_rect(t0, t1, rect):
    x0, y0 = t0
    x1, y1 = t1

    if x0 == x1:  # vertical line
        if (min(y0, y1) < rect.top < max(y0, y1) \
                or min(y0, y1) < rect.bottom < max(y0, y1) \
                or rect.bottom <= min(y0, y1) < max(y0, y1) <= rect.top) \
                and rect.left < x0 < rect.right:
            return True
    elif y0 == y1:  # horizontal line
        if (min(x0, x1) < rect.right < max(x0, x1) \
                or min(x0, x1) < rect.left < max(x0, x1) \
                or rect.left <= min(x0, x1) < max(x0, x1) <= rect.right) \
                and rect.bottom < y0 < rect.top:
            return True
    return False


def hori_line_below_rect(t0, t1, rect):
    if t0.y == t1.y:  # horizontal line
        if t0.y <= rect.bottom \
                and not (min(t0.x, t1.x) > rect.right) \
                and not (max(t0.x, t1.x) < rect.left):
            return True
    return False


def vert_line_left_of_rect(t0, t1, rect):
    if t0.x == t1.x:  # vertical line
        if t0.x <= rect.left \
                and not (min(t0.y, t1.y) > rect.top) \
                and not (max(t0.y, t1.y) < rect.bottom):
            return True
    return False


# Use raycasting to determine if all points on the rectangle
# edge are inside the polygon. Rays should pass through an
# odd number of lines when it reaches the rectangle edge.
def rect_edges_are_inside_polygon(
        rect,
        red_tiles_set,
        lines_below_rect,
        lines_left_of_rect):

    for x in range(rect.left + 1, rect.right):
        if (x, rect.bottom) in red_tiles_set:
            continue
        n_lines = sum(1 for t0, t1 in lines_below_rect \
                      if min(t0.x, t1.x) < x <= max(t0.x, t1.x))
        if n_lines % 2 == 0:
            return False

    for y in range(rect.bottom + 1, rect.top):
        if (rect.left, y) in red_tiles_set:
            continue
        n_lines = sum(1 for t0, t1 in lines_left_of_rect \
                      if min(t0[1], t1[1]) < y <= max(t0[1], t1[1]))

        if n_lines % 2 == 0:
            return False
    return True


def is_valid(p0, p1, red_tiles_set, connections):
    x0, y0 = p0
    x1, y1 = p1
    rect = rectangle(min(x0, x1), max(x0, x1), max(y0, y1), min(y0, y1))

    lines_below_rect = []
    lines_left_of_rect = []

    for t0, t1 in connections:
        if line_intersects_rect(t0, t1, rect):
            return False
        elif hori_line_below_rect(t0, t1, rect):
            lines_below_rect.append((t0, t1))
        elif vert_line_left_of_rect(t0, t1, rect):
            lines_left_of_rect.append((t0, t1))

    if rect_edges_are_inside_polygon(
            rect,
            red_tiles_set,
            lines_below_rect,
            lines_left_of_rect):
        return True
    return False

def aoc9p2():
    start_time = time.process_time()
    red_tiles = []
    with open("aoc9_data", "r") as f:
        while l := f.readline().strip():
            x, y = l.split(',')
            red_tiles.append((int(x), int(y)))

    connections = [(point(*t0), point(*t1)) for t0, t1 in zip(red_tiles[:-1], red_tiles[1:])]
    connections.append((point(*red_tiles[-1]), point(*red_tiles[0])))
    red_tiles_set = set(red_tiles)

    min_dist = min([abs(x1 - x2 + y1 - y2)
                    for (x1, y1), (x2, y2) in connections])
    if min_dist == 1:
        print(f"Two red tiles are next to eachother, "
               "this algorithm is not guaranteed to work.")
    else:
        print(f"Minimum distance between red tiles is "
              f"{min_dist} > 1. Algorithm works.")

    # [(corner1, corner2, area)...]
    rt_matrix = [[(a, b, (abs(a[0] - b[0]) + 1) * (abs(a[1] - b[1]) + 1))
                  for a in red_tiles] for b in red_tiles]
    rt_matrix = sorted([item for l in rt_matrix for item in l],
                       key=lambda x: x[2])

    i = 1
    interval = 1
    while rt_matrix:
        t0, t1, area = rt_matrix.pop()
        if i % interval == 0:
            print(f"#{i}\tChecking rect {t0}, {t1}. Area: {area}.")
        if is_valid(t0, t1, red_tiles_set, connections):
            print(f"Max: {area}, red tiles: {t0}, {t1}. (Calculated in {time.process_time() - start_time}s.)")
            return
        if i % (interval * 10) == 0:
            interval *= 10
        i += 1
    print("No valid rectangles found.")


def aoc9p1():
    red_tiles = []
    with open("aoc9_data", "r") as f:
        while l := f.readline().strip():
            x, y = l.split(',')
            red_tiles.append((int(x), int(y)))
    print(red_tiles)
    rt_matrix = [[(a, b, (abs(a[0] - b[0]) + 1) * (abs(a[1] - b[1]) + 1))
                  for a in red_tiles] for b in red_tiles]
    max_area = max([i[2] for l in rt_matrix for i in l])
    print(f"Max: {max_area}")


if __name__ == "__main__":
    aoc9p2()
