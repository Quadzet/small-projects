import re

def aoc12p1():
    with open("aoc12_data", "r") as f:
        index = 0
        presents = [ ]
        regions = [ ]
        while line := f.readline():
            line = line.strip()
            if line == '':
                continue
            elif match := re.match("(\d):", line):
                index = int(match.group(1))
                present = [ ]
                size = 0
                for i in range(0, 3):
                    line = f.readline().strip()
                    present.append([1 if c == '#' else 0 for c in line])
                    size += sum(present[i])
                presents.append((size, present))
            elif match := re.match(f"(\d+)x(\d+): ([\d+\s?]*)", line):
                width = match.group(1)
                length = match.group(2)
                n_presents = [int(i) for i in match.group(3).split(' ')]
                regions.append((int(width), int(length), n_presents))

    n_large = 0
    n_small = 0
    n_undec = 0
    for r_ix, region in enumerate(regions):
        presents_size = 0
        n_presents = sum([a for a in region[2]])
        for p_ix, p_n in enumerate(region[2]):
            presents_size += presents[p_ix][0] * p_n
        if region[0] * region[1] > n_presents * 9:
            n_large += 1
            continue
        elif region[0] * region[1] < presents_size:
            n_small += 1
            continue
        else:
            n_undec += 1
    print(f"Total number of undecided: {n_undec}. (Too small: {n_small}, big enough: {n_large})")
    print(f"Since it is NP-hard to prove computationally that a solution does not exist,")
    print(f"maybe one is free to assume that the undecided do in fact have a solution...")
    print(f"If so, the answer is {n_large + n_undec}")

if __name__ == "__main__":
    aoc12p1()

