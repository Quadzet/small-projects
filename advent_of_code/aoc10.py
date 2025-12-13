from math import dist, prod
from scipy.optimize import linprog
import re
import numpy as np
from collections import namedtuple
import functools
from pulp import LpProblem, LpMinimize, LpVariable, lpSum, LpInteger, PULP_CBC_CMD

QUICKEST_SOLVE = 100
machine = namedtuple('machine', ['state', 'goal_state', 'buttons', 'joltage'])

def button_memo(func):
    button_cache = func.cache = { }

    @functools.wraps(func)
    def memoized_func(*args, **kwargs):
        key = str(args) + str(kwargs)
        if key not in button_cache:
            button_cache[key] = func(*args, **kwargs)
        return button_cache[key]
    return memoized_func

@button_memo
def press_buttons(m: machine, depth: int):
    global QUICKEST_SOLVE
    if depth >= QUICKEST_SOLVE:
        return QUICKEST_SOLVE

    results = []
    for buttons in m.buttons:
        new_state = m.state.copy()
        for b in [c for c in buttons if c not in (',', '(', ')')]:
            new_state[int(b)] = '#' if new_state[int(b)] == '.' else '.'

        if ''.join(new_state) == m.goal_state:
            if depth < QUICKEST_SOLVE:
                QUICKEST_SOLVE = depth
            return depth
        else:
            new_m = machine(new_state, m.goal_state, m.buttons, m.joltage)
            results.append(press_buttons(new_m, depth + 1))

    return min(results)

def aoc10p2():
    res = 0
    with open("aoc10_data", "r") as f:
        while line := f.readline().strip():
            buttons = [b for b in re.search('(\(.*\))+', line).group().split(' ')]
            joltage = [int(j) for j in re.search('\{(.*)\}', line).group(1).split(',')]
            state = [0 for j in joltage]

            A = [[0 for b in buttons] for a in joltage]

            for c, b in enumerate(buttons):
                for r in [int(c) for c in b if c not in (',', '(', ')')]:
                    A[r][c] = 1

            A_mat = np.array(A)
            n_vars = A_mat.shape[1]
            prob = LpProblem("MinSum", LpMinimize)
            x = [LpVariable(f"x{i}", lowBound=0, cat=LpInteger) for i in range(n_vars)]
            prob += lpSum(x)

            for i, target_j in enumerate(joltage):
                prob += lpSum(A_mat[i,j] * x[j] for j in range(n_vars)) == target_j

            prob.solve(PULP_CBC_CMD(msg=0))

            solution = [v.varValue for v in x]
            res += sum(solution)
            print(f"Solution sum: {sum(solution)}.")

    print(f"Sum: {res}")


def aoc10p1():
    global QUICKEST_SOLVE
    sum = 0
    with open("aoc10_data", "r") as f:
        while line := f.readline().strip():
            QUICKEST_SOLVE = 100
            goal_state = re.search('\[(.*)\]', line)
            buttons = re.search('(\(.*\))+', line)
            joltage = re.search('\{.*\}', line)
            state = ['.' for n in goal_state.group() if n not in ('[', ']')]
            m = machine(state, goal_state.group(1), buttons.group().split(' '), joltage.group().split(','))
            print(m)
            presses = press_buttons(m, 1)
            sum += presses
            print(f"Presses: {presses}.")
    print(f"Sum: {sum}")

if __name__ == "__main__":
    aoc10p2()
