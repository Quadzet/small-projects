MAX_ADJACENT = 5 # including the roll being considered

def aoc4p2():
    res = 0
    adj_coords = [ (r, c) for r in [-1,0,1] for c in [-1,0,1] ]
    with open("aoc4_data", "r") as f:
        matrix = [[ 1 if c == '@' else 0 for c in line.strip() ] for line in f ]
        n_col = len(matrix[0]) if matrix else 0
        n_row = len(matrix)
        while True:
            removed_rolls = []
            for r in range(n_row):
                for c in range(n_col):
                    if matrix[r][c] == 0:
                        continue
                    adj = sum(matrix[r + a][c + b]
                            if 0 <= r + a < n_row and 0 <= c + b < n_col 
                            else 0
                            for a, b in adj_coords)
                    if adj < MAX_ADJACENT:
                        res += 1
                        removed_rolls.append((r, c))
            if not removed_rolls:
                break
            for (r, c) in removed_rolls:
                matrix[r][c] = 0
    print(f"Sum: {res}")

def aoc4p1():
    res = 0
    adj_coords = [ (r, c) for r in [-1,0,1] for c in [-1,0,1] ]
    with open("aoc4_data", "r") as f:
        matrix = [[ 1 if c == '@' else 0 for c in line.strip() ] for line in f ]
        n_col = len(matrix[0]) if matrix else 0
        n_row = len(matrix)
        for r in range(n_row):
            for c in range(n_col):
                if matrix[r][c] == 0:
                    continue
                adj = sum(matrix[r + a][c + b] 
                        if 0 <= r + a < n_row and 0 <= c + b < n_col
                        else 0
                        for a, b in adj_coords)
                if adj < MAX_ADJACENT:
                    res += 1
    print(f"Sum: {res}")


if __name__ == "__main__":
    aoc4p2()
