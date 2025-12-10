from math import dist
import numpy as np

def aoc8p1():
    boxes = [ ]
    with open("aoc8_data", "r") as f:
        while line := f.readline().strip():
            boxes.append(tuple(int(n) for n in line.split(',')))
    n_connections = 1000
    distances = np.array([[dist(a, b) for a in boxes] for b in boxes])

    connections = [ ]
    for i in range(n_connections):
        min_dist = np.min([c for r in distances for c in r if c > 0])
        r_v, c_v = np.where(np.isclose(distances, min_dist))
        r = r_v[0]
        c = c_v[0]
        print(f"r: {r}, c: {c}")
        # invalidate all further connections
        distances[r, c] = 0
        distances[c, r] = 0
        """
        indices1 = np.where(distances[r, :] == 0)[0]
        indices2 = np.where(distances[c, :] == 0)[0]
        new_conns = set(indices1) | set(indices2)
        for a in new_conns:
            for b in new_conns:
                distances[a, b] = 0
        """

    counts = { }
    for row in distances:
        n = (row == 0).sum()
        counts[n] = counts.get(n, 0) + 1
    product = 1
    i = 0
    n_significant = 3
    for zeros_count, frequency in sorted(counts.items(), key=lambda x: -x[0]):
        print(f"zeros_count: {zeros_count}, frequency: {frequency // zeros_count}")
        i += min((frequency // zeros_count), n_significant - i)
        # print(f"i: {i}")
        if i < n_significant:
            product *= (zeros_count ** min((frequency // zeros_count), n_significant - i))

    # print('\n'.join([' '.join([str(int(c)) if c == 0 else '1' for c in r]) for r in distances]))
    print(f"Product of top {n_significant} circuit lengths: {product}")


if __name__ == "__main__":
    aoc8p1()
