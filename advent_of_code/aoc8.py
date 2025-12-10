from math import dist, prod
import numpy as np

def aoc8p2():
    boxes = [ ]
    with open("aoc8_data", "r") as f:
        while line := f.readline().strip():
            boxes.append(tuple(int(n) for n in line.split(',')))
    n_connections = 1000
    distances = [[(b_i, a_i, dist(a, b), a, b) for a_i, a in enumerate(boxes) if a_i < b_i] for b_i, b in enumerate(boxes)]
    distances = [d for l in distances for d in l]
    distances = sorted(distances, key=lambda x: -x[2])


    circuits = [ ]
    for i in range(len(distances)):
        r, c, min_dist, a, b = distances.pop()
        common_circuits = [circ for circ in circuits if r in circ or c in circ]
        if len(common_circuits) == 0:
            circuits.append({r, c})
        elif len(common_circuits) == 1:
            circuits.remove(common_circuits[0])
            common_circuits[0].add(r)
            common_circuits[0].add(c)
            circuits.append(common_circuits[0])
        elif len(common_circuits) == 2:
            for cc in common_circuits:
                circuits.remove(cc)
            circuits.append(set().union(*common_circuits))
        if len(circuits) == 1 and len(circuits[0]) == 1000:
            print(f"Every node is connected, last nodes to be connected: {a}, {b}. Product of x-coords: {a[0] * b[0]}.")
            return
    print("Error, all nodes connected to every other node.")


def aoc8p1():
    boxes = [ ]
    with open("aoc8_data", "r") as f:
        while line := f.readline().strip():
            boxes.append(tuple(int(n) for n in line.split(',')))
    n_connections = 1000
    distances = np.array([[dist(a, b) for a in boxes] for b in boxes])

    circuits = [ ]
    for i in range(n_connections):
        min_dist = np.min([c for r in distances for c in r if c > 0])
        r_v, c_v = np.where(np.isclose(distances, min_dist))
        r = r_v[0]
        c = c_v[0]
        print(f"r: {r}, c: {c}")
        # invalidate all further connections
        distances[r, c] = 0
        distances[c, r] = 0
        common_circuits = [circ for circ in circuits if r in circ or c in circ]
        print(common_circuits)
        if len(common_circuits) == 0:
            circuits.append({r, c})
        elif len(common_circuits) == 1:
            circuits.remove(common_circuits[0])
            common_circuits[0].add(r)
            common_circuits[0].add(c)
            circuits.append(common_circuits[0])
        elif len(common_circuits) == 2:
            for cc in common_circuits:
                circuits.remove(cc)
            circuits.append(set().union(*common_circuits))

        print(circuits)
    len_circuits = sorted([len(c) for c in circuits], key=lambda x: -x)
    print(f"len_circuits: {len_circuits}")
    result = prod(len_circuits[-4:-1])
    print(f"Product: {result}")


if __name__ == "__main__":
    aoc8p2()
