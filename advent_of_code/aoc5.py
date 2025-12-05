def aoc5p2():
    ranges = []
    with open("aoc5_data", "r") as f:
        while line := f.readline().strip():
            start, end = (int(c) for c in line.split("-"))
            ranges.append((start, end - start + 1))
    sorted_ranges = sorted(ranges, key=lambda x: x[0])

    merged_ranges = []
    for (new_start, new_len) in sorted_ranges:
        added = False
        for i, (start, len) in enumerate(merged_ranges):
            if start <= new_start <= start + len:
                merged_ranges[i] = (start, max(len, new_start - start + new_len))
                added = True
                break
        if not added:
            merged_ranges.append((new_start, new_len))

    n_fresh = sum([l for (_, l) in merged_ranges])
    print(f"Number of fresh ingredients: {n_fresh}")

def aoc5p1():
    fresh = set()
    sum = 0
    ranges = []
    with open("aoc5_data", "r") as f:
        while line := f.readline().strip():
            start, end = (int(c) for c in line.split("-"))
            ranges.append((start, end))

        while n := f.readline().strip():
            for (start, end) in ranges:
                if start <= int(n) <= end:
                    sum += 1
                    break

    print(f"Number of fresh ingredients: {sum}")

if __name__ == "__main__":
    aoc5p2()
