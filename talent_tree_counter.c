#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ROWS 7
#define MAX_COLS 4
int MAX_POINTS;

typedef struct {
    int max;
    int req_row;
    int req_col;
    bool has_req;
} Node;

typedef struct {
    Node nodes[MAX_ROWS][MAX_COLS];
    int row_sizes[MAX_ROWS];
    int num_rows;
} Tree;

typedef struct {
    int tree_id;
    int points_left;
    int current_row;
    int prereq_state[MAX_ROWS * MAX_COLS]; // TODO: Reduce this to only the few nodes with reqs
} MemoKey;

typedef struct MemoNode {
    MemoKey key;
    long long value;
    struct MemoNode* next;
} MemoNode;

#define MEMO_SIZE 100003  // large prime
MemoNode* memo_table[MEMO_SIZE];

// Hash function for memoization key
unsigned long hash_key(const MemoKey* key) {
    unsigned long hash = 5381;
    hash = hash * 33 + key->tree_id;
    hash = hash * 33 + key->points_left;
    hash = hash * 33 + key->current_row;

    for (int i = 0; i < MAX_ROWS * MAX_COLS; i++) {
        hash = hash * 33 + key->prereq_state[i];
    }

    return hash % MEMO_SIZE;
}

bool keys_equal(const MemoKey* a, const MemoKey* b) {
    return a->tree_id == b->tree_id &&
           a->points_left == b->points_left &&
           a->current_row == b->current_row &&
           memcmp(
                a->prereq_state,
                b->prereq_state,
                sizeof(a->prereq_state)) == 0;
}

long long memo_get(const MemoKey* key) {
    unsigned long hash = hash_key(key);
    MemoNode* node = memo_table[hash];

    while (node) {
        if (keys_equal(&node->key, key)) {
            return node->value;
        }
        node = node->next;
    }

    return -1;
}

void memo_put(const MemoKey* key, long long value) {
    unsigned long hash = hash_key(key);
    MemoNode* node = malloc(sizeof(MemoNode));
    node->key = *key;
    node->value = value;
    node->next = memo_table[hash];
    memo_table[hash] = node;
}

Tree trees[3];

// Function to extract prerequisite state from full allocation
void extract_prereq_state(
        int tree_id,
        int allocations[][MAX_COLS],
        int* prereq_state) {

    memset(prereq_state, 0, MAX_ROWS * MAX_COLS * sizeof(int));

    for (int i = 0; i < trees[tree_id].num_rows; i++) {
        for (int j = 0; j < trees[tree_id].row_sizes[i]; j++) {
            // Check if this node is a prerequisite for any other node
            bool is_prereq = false;
            for (int r = 0; r < trees[tree_id].num_rows && !is_prereq; r++) {
                int n_cols = trees[tree_id].row_sizes[r];
                for (int c = 0; c < n_cols && !is_prereq; c++) {
                    if (trees[tree_id].nodes[r][c].has_req && 
                        trees[tree_id].nodes[r][c].req_row == i && 
                        trees[tree_id].nodes[r][c].req_col == j) {
                        is_prereq = true;
                    }
                }
            }

            if (is_prereq) {
                prereq_state[i * MAX_COLS + j] = allocations[i][j];
            }
        }
    }
}

bool is_node_unlocked(
        int tree_id,
        int row,
        int col,
        int total_spent,
        int allocations[][MAX_COLS]) {

    if (total_spent < row * 5) return false;

    // Check prerequisite requirement
    Node *node = &trees[tree_id].nodes[row][col];
    if (node->has_req) {
        int req_row = node->req_row;
        int req_col = node->req_col;
        int max_points = trees[tree_id].nodes[req_row][req_col].max;
        if (allocations[req_row][req_col] < max_points) {
            return false;
        }
    }
    return true;
}


long long count_tree_allocations(
    int tree_id,
    int points_left,
    int total_spent,
    int current_row,
    int allocations[][MAX_COLS]);


// Helper function to distribute points within a row
long long distribute_in_row(
        int tree_id,
        int row,
        int row_points_left,
        int col,
        int points_left_after_row,
        int total_spent,
        int allocations[][MAX_COLS]) {

    if (points_left_after_row == 0 && row_points_left == 0) {
        return 1;
    }
    if (total_spent < row * 5) {
        return 0;
    }

    if (col >= trees[tree_id].row_sizes[row]) {
        if (row_points_left == 0) {
            // Move to next row
            return count_tree_allocations(
                        tree_id,
                        points_left_after_row,
                        total_spent,
                        row + 1,
                        allocations);
        }
        return 0;
    }

    long long result = 0;
    Node *node = &trees[tree_id].nodes[row][col];

    int max_alloc = (node->max < row_points_left)
            ? node->max : row_points_left;

    for (int alloc = 0; alloc <= max_alloc; alloc++) {
        bool can_allocate = true;

        if (alloc > 0) {
            if (!is_node_unlocked(
                    tree_id,
                    row,
                    col,
                    total_spent + alloc,  // should be total_spent only?
                    allocations)) {
                can_allocate = false;
            }
        }

        if (can_allocate) {
            int old_alloc = allocations[row][col];
            allocations[row][col] = alloc;
            result += distribute_in_row(
                    tree_id,
                    row,
                    row_points_left - alloc,
                    col + 1,
                    points_left_after_row,
                    total_spent + alloc,
                    allocations);

            // Restore allocation
            allocations[row][col] = old_alloc;
        }
    }

    return result;
}

// Recursive function to count allocations for a single tree
long long count_tree_allocations(
        int tree_id,
        int points_left,
        int total_spent,
        int current_row,
        int allocations[][MAX_COLS]) {

    if (total_spent < current_row * 5) {
        return 0;
    }
    if (points_left < 0) return 0;
    if (current_row >= trees[tree_id].num_rows && points_left == 0) {
        return 1;
    }

    MemoKey key;
    key.tree_id = tree_id;
    key.points_left = points_left;
    key.current_row = current_row;
    extract_prereq_state(tree_id, allocations, key.prereq_state);

    long long cached = memo_get(&key);
    if (cached != -1) {
        return cached;
    }

    long long result = 0;

    int row_size = trees[tree_id].row_sizes[current_row];

    int max_row_points = 0;
    for (int i = 0; i < row_size; i++) {
        max_row_points += trees[tree_id].nodes[current_row][i].max;
    }

    max_row_points = (max_row_points < points_left)
            ? max_row_points : points_left;

    // Try allocating 0 to max_row_points points in this row
    for (int row_points = 0; row_points <= max_row_points; row_points++) {
        int points_remaining_after_row = points_left - row_points;
        result += distribute_in_row(
                tree_id,
                current_row,
                row_points,
                0,
                points_remaining_after_row,
                total_spent,
                allocations);
    }

    memo_put(&key, result);
    return result;
}

void initialize_trees() {
    // Initialize Arms tree
    trees[0].num_rows = 7;
    int arms_row_sizes[] = {3, 3, 3, 2, 4, 2, 1};
    memcpy(trees[0].row_sizes, arms_row_sizes, sizeof(arms_row_sizes));

    Node arms_data[7][4] = {
        {{3, -1, -1, false}, {5, -1, -1, false}, {3, -1, -1, false}},
        {{2, -1, -1, false}, {5, -1, -1, false}, {3, -1, -1, false}},
        {{2, -1, -1, false}, {1, 1, 1, true}, {3, 0, 2, true}},
        {{5, -1, -1, false}, {2, 2, 2, true}},
        {{5, -1, -1, false}, {1, -1, -1, false}, {5, -1, -1, false}, {5, -1, -1, false}},
        {{5, -1, -1, false}, {3, -1, -1, false}},
        {{1, 4, 1, true}}
    };

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < arms_row_sizes[i]; j++) {
            trees[0].nodes[i][j] = arms_data[i][j];
        }
    }

    // Initialize Fury tree  
    trees[1].num_rows = 7;
    int fury_row_sizes[] = {2, 2, 4, 3, 3, 2, 1};
    memcpy(trees[1].row_sizes, fury_row_sizes, sizeof(fury_row_sizes));

    Node fury_data[7][4] = {
        {{5, -1, -1, false}, {5, -1, -1, false}},
        {{5, -1, -1, false}, {5, -1, -1, false}},
        {{3, -1, -1, false}, {1, -1, -1, false}, {3, -1, -1, false}, {5, -1, -1, false}},
        {{5, -1, -1, false}, {2, -1, -1, false}, {5, -1, -1, false}},
        {{5, -1, -1, false}, {1, -1, -1, false}, {2, -1, -1, false}},
        {{2, -1, -1, false}, {5, 3, 2, true}},
        {{1, 4, 1, true}}
    };

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < fury_row_sizes[i]; j++) {
            trees[1].nodes[i][j] = fury_data[i][j];
        }
    }

    // Initialize Protection tree
    trees[2].num_rows = 7;
    int prot_row_sizes[] = {2, 3, 4, 3, 3, 1, 1};
    memcpy(trees[2].row_sizes, prot_row_sizes, sizeof(prot_row_sizes));

    Node prot_data[7][4] = {
        {{5, -1, -1, false}, {5, -1, -1, false}},
        {{2, -1, -1, false}, {5, -1, -1, false}, {5, -1, -1, false}},
        {{1, 1, 0, true}, {3, 0, 0, true}, {3, -1, -1, false}, {5, -1, -1, false}},
        {{3, -1, -1, false}, {3, -1, -1, false}, {2, -1, -1, false}},
        {{2, -1, -1, false}, {1, -1, -1, false}, {2, -1, -1, false}},
        {{5, -1, -1, false}},
        {{1, 4, 1, true}}
    };

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < prot_row_sizes[i]; j++) {
            trees[2].nodes[i][j] = prot_data[i][j];
        }
    }
}

// Convolution to combine results from different trees
long long* convolve(long long* a, long long* b, int size) {
    long long* result = calloc(size + 1, sizeof(long long));

    for (int i = 0; i <= size; i++) {
        for (int j = 0; j <= size - i; j++) {
            result[i + j] += a[i] * b[j];
        }
    }

    return result;
}

void cleanup_memo() {
    for (int i = 0; i < MEMO_SIZE; i++) {
        MemoNode* node = memo_table[i];
        while (node) {
            MemoNode* next = node->next;
            free(node);
            node = next;
        }
        memo_table[i] = NULL;
    }
}

int main(int argc, char *argv[]) {

    MAX_POINTS = atoi(argv[1]);
    initialize_trees();

    long long* tree_distributions[3];

    for (int tree = 0; tree < 3; tree++) {
        printf("Calculating tree %d...\n", tree);
        tree_distributions[tree] = calloc(MAX_POINTS + 1, sizeof(long long));

        for (int points = 0; points <= MAX_POINTS; points++) {
            int allocations[MAX_ROWS][MAX_COLS] = {0};
            tree_distributions[tree][points] =
                    count_tree_allocations(tree, points, 0, 0, allocations);
            printf("Tree %d: calculated %lld ways for %d points\n",
                   tree, tree_distributions[tree][points], points);
        }
        cleanup_memo();
    }

    // Combine using convolution
    long long* combined = tree_distributions[0];
    for (int tree = 1; tree < 3; tree++) {
        printf("Convolving with tree %d...\n", tree);
        long long* new_combined =
                convolve(combined, tree_distributions[tree], MAX_POINTS);
        if (tree > 1) free(combined);
        combined = new_combined;
    }

    printf("\nFinal Result: Number of ways to allocate exactly %d points: %lld\n",
            MAX_POINTS, combined[MAX_POINTS]);

    printf("\nSome intermediate results:\n");
    for (int i = 0; i <= MAX_POINTS; i++) {
        printf("Ways to allocate %d points: %lld\n", i, combined[i]);
    }

    // Cleanup
    for (int tree = 0; tree < 3; tree++) {
        free(tree_distributions[tree]);
    }
    free(combined);

    return 0;
}
