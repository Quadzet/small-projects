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
def output2(current_device: str, current_path: str, has_fft: bool, has_dac):
    res = 0
    global PATHS
    for dest in PATHS[current_device]:
        if dest == "fft":
            res += output2(dest, True, has_dac)
        if dest == "dac":
            if has_fft:
                res += output2(dest, has_fft, True)
            else:
                pass
        elif dest == "out":
            if has_fft and has_dac:
                res += 1
        else:
            res += output2(dest, has_fft, has_dac)
    return res


def aoc11p2():
    global PATHS
    with open("aoc11_data", "r") as f:
        while line := f.readline().strip():
            src = re.search('^([^:]*):', line).group(1)
            print(f"src: {src}")
            dests = re.search(':(.*)', line).group(1).strip().split(' ')
            print(f"dests: {dests}")

            PATHS[src] = dests
    # print(PATHS)
    res = output2("svr", False, False)
    print(f"Number of paths: {res}")

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
