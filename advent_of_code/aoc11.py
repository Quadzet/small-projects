import re
from functools import lru_cache


PATHS = { }
full_paths = set()
def output(current_device: str, current_path: list, paths: dict):
    for dest in paths[current_device]:
        current_path.append(dest)
        if dest == "out":
            full_paths.add(str(current_path))
        else:
            output(dest, current_path, paths)

@lru_cache
def output2(current_device: str, end_device: str):
    if current_device == end_device:
        return 1
    elif current_device == "out":
        return 0

    res = 0
    global PATHS
    for dest in PATHS[current_device]:
        res += output2(dest, end_device)
    return res

def aoc11p2():
    global PATHS
    with open("aoc11_data", "r") as f:
        while line := f.readline().strip():
            src = re.search('^([^:]*):', line).group(1)
            dests = re.search(':(.*)', line).group(1).strip().split(' ')
            PATHS[src] = dests
    res1 = output2("svr", "fft")
    print(f"From svr to fft: {res1}")
    res2 = output2("fft", "dac")
    print(f"From fft to dac: {res2}")
    res3 = output2("dac", "out")
    print(f"From dac to out: {res3}")
    print(f"Total: {res1 * res2 * res3}.")

def aoc11p1():
    sum = 0
    paths = { }
    with open("aoc11_data", "r") as f:
        while line := f.readline().strip():
            src = re.search('^([^:]*):', line).group(1)
            print(f"src: {src}")
            dests = re.search(':(.*)', line).group(1).strip().split(' ')
            print(f"dests: {dests}")

            paths[src] = dests
    print(paths)
    output("you", ["you"], paths)
    print(f"Number of paths: {len(full_paths)}")

if __name__ == "__main__":
    aoc11p2()
