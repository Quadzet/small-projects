#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Constants
#define TOTAL_POINTS 51
#define MAX_TREE_POINTS 200  // Upper bound for individual tree calculations
#define TREE_MEMO_TABLE_SIZE 1000000  // 1M entries per tree

// Single tree state (much smaller than full state)
typedef struct {
    uint64_t bits;  // Only need 1 uint64_t for single tree (3 bits × ~20 nodes = ~60 bits)
    uint16_t total_points;
} tree_state_t;

// Memoization for single tree
typedef struct tree_memo_entry {
    tree_state_t state;
    long long result;
    struct tree_memo_entry* next;
} tree_memo_entry_t;

typedef struct {
    tree_memo_entry_t* buckets[TREE_MEMO_TABLE_SIZE];
    long long hits;
    long long misses;
    long long stores;
} tree_memo_table_t;

// Node information for a single tree
typedef struct {
    int row;
    int col;
    int max_points;
    int req_row;    // -1 if no requirement
    int req_col;    // -1 if no requirement
} tree_node_t;

// Tree structure
typedef struct {
    tree_node_t nodes[25];  // Max nodes per tree
    int node_count;
    long long* results;     // results[i] = ways to allocate i points to this tree
    tree_memo_table_t memo; // Memoization for this tree
} tree_info_t;

// Global tree data
tree_info_t trees[3];  // Arms, Fury, Protection
long long call_count = 0;

// Hash function for single tree state
uint64_t hash_tree_state(const tree_state_t* state) {
    uint64_t hash = 0x9e3779b9;  // Golden ratio hash constant
    hash ^= state->bits + (hash << 6) + (hash >> 2);
    hash ^= (uint64_t)state->total_points + (hash << 6) + (hash >> 2);
    return hash;
}

// Compare two tree states for equality
bool tree_states_equal(const tree_state_t* state1, const tree_state_t* state2) {
    return (state1->bits == state2->bits && 
            state1->total_points == state2->total_points);
}

// Initialize tree memo table
void init_tree_memo_table(tree_memo_table_t* memo) {
    memset(memo, 0, sizeof(tree_memo_table_t));
}

// Look up a tree state in memoization table
long long lookup_tree_memo(tree_memo_table_t* memo, const tree_state_t* state) {
    uint64_t hash = hash_tree_state(state);
    int bucket = hash % TREE_MEMO_TABLE_SIZE;
    
    tree_memo_entry_t* entry = memo->buckets[bucket];
    while (entry) {
        if (tree_states_equal(&entry->state, state)) {
            memo->hits++;
            return entry->result;
        }
        entry = entry->next;
    }
    
    memo->misses++;
    return -1;  // Not found
}

// Store result in tree memo table
void store_tree_memo(tree_memo_table_t* memo, const tree_state_t* state, long long result) {
    uint64_t hash = hash_tree_state(state);
    int bucket = hash % TREE_MEMO_TABLE_SIZE;
    
    // Create new entry
    tree_memo_entry_t* new_entry = malloc(sizeof(tree_memo_entry_t));
    new_entry->state = *state;
    new_entry->result = result;
    new_entry->next = memo->buckets[bucket];
    
    memo->buckets[bucket] = new_entry;
    memo->stores++;
}

// Free tree memo table
void free_tree_memo_table(tree_memo_table_t* memo) {
    for (int i = 0; i < TREE_MEMO_TABLE_SIZE; i++) {
        tree_memo_entry_t* entry = memo->buckets[i];
        while (entry) {
            tree_memo_entry_t* next = entry->next;
            free(entry);
            entry = next;
        }
    }
}

// Bit manipulation for single tree
static inline int get_tree_node_points(const tree_state_t* state, int node_idx) {
    int bit_pos = node_idx * 3;
    return (state->bits >> bit_pos) & 7;
}

static inline void set_tree_node_points(tree_state_t* state, int node_idx, int points) {
    int bit_pos = node_idx * 3;
    state->bits &= ~(7ULL << bit_pos);  // Clear 3 bits
    state->bits |= ((uint64_t)points << bit_pos);  // Set new value
}

// Find node index by row/col in a specific tree
int find_tree_node_index(tree_info_t* tree, int row, int col) {
    for (int i = 0; i < tree->node_count; i++) {
        if (tree->nodes[i].row == row && tree->nodes[i].col == col) {
            return i;
        }
    }
    return -1;
}

// Check if a node is available for allocation in a single tree
bool is_tree_node_available(tree_info_t* tree, const tree_state_t* state, int node_idx) {
    tree_node_t* node = &tree->nodes[node_idx];
    
    // Check if node is already maxed out
    if (get_tree_node_points(state, node_idx) >= node->max_points) {
        return false;
    }
    
    // Check row requirement (need row_index * 5 points in tree)
    if (state->total_points < node->row * 5) {
        return false;
    }
    
    // Check prerequisite requirement
    if (node->req_row != -1) {
        int req_node_idx = find_tree_node_index(tree, node->req_row, node->req_col);
        if (req_node_idx == -1) {
            printf("Error: Required node not found!\n");
            return false;
        }
        int req_max = tree->nodes[req_node_idx].max_points;
        if (get_tree_node_points(state, req_node_idx) < req_max) {
            return false;
        }
    }
    
    return true;
}

// Calculate number of ways to allocate points to a single tree (with memoization)
long long calculate_single_tree(tree_info_t* tree, tree_state_t current_state, int remaining_points) {
    call_count++;
    
    // Base case
    if (remaining_points == 0) return 1;
    if (remaining_points < 0) return 0;
    
    // Check memoization
    long long cached_result = lookup_tree_memo(&tree->memo, &current_state);
    if (cached_result != -1) {
        return cached_result;
    }
    
    long long total_ways = 0;
    
    // Try allocating to each available node
    for (int i = 0; i < tree->node_count; i++) {
        if (is_tree_node_available(tree, &current_state, i)) {
            int current_points = get_tree_node_points(&current_state, i);
            int max_can_add = (remaining_points < (tree->nodes[i].max_points - current_points)) ? 
                             remaining_points : (tree->nodes[i].max_points - current_points);
            
            // Try adding 1 to max_can_add points to this node
            for (int add = 1; add <= max_can_add; add++) {
                tree_state_t new_state = current_state;
                set_tree_node_points(&new_state, i, current_points + add);
                new_state.total_points += add;
                
                total_ways += calculate_single_tree(tree, new_state, remaining_points - add);
            }
        }
    }
    
    // Store in memo table
    store_tree_memo(&tree->memo, &current_state, total_ways);
    
    return total_ways;
}

// Calculate results for a single tree up to max_points
void calculate_tree_results(tree_info_t* tree, int max_points, const char* tree_name) {
    printf("Calculating %s tree for 0-%d points...\n", tree_name, max_points);
    
    tree->results = calloc(max_points + 1, sizeof(long long));
    
    for (int points = 0; points <= max_points; points++) {
        // Initialize fresh memoization for each point calculation
        init_tree_memo_table(&tree->memo);
        
        tree_state_t initial_state = {0, 0};
        call_count = 0;
        
        tree->results[points] = calculate_single_tree(tree, initial_state, points);
        
        if (points % 5 == 0 || points <= 10) {
            printf("  %s[%d points] = %lld ways (%lld calls, hits: %lld)\n", 
                   tree_name, points, tree->results[points], call_count, tree->memo.hits);
        }
        
        // Clean up memo table for this point calculation
        free_tree_memo_table(&tree->memo);
    }
}

// Initialize tree data structures
void initialize_tree_data() {
    // Arms tree
    trees[0].node_count = 0;
    // Row 0
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){0, 0, 3, -1, -1};
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){0, 1, 5, -1, -1};
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){0, 2, 3, -1, -1};
    // Row 1
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){1, 0, 2, -1, -1};
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){1, 1, 5, -1, -1};
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){1, 2, 3, -1, -1};
    // Row 2
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){2, 0, 2, -1, -1};
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){2, 1, 1, 1, 1};  // req [1,1]
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){2, 2, 3, 0, 2};  // req [0,2]
    // Row 3
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){3, 0, 5, -1, -1};
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){3, 1, 2, 2, 2};  // req [2,2]
    // Row 4
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){4, 0, 5, -1, -1};
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){4, 1, 1, -1, -1};
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){4, 2, 5, -1, -1};
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){4, 3, 5, -1, -1};
    // Row 5
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){5, 0, 5, -1, -1};
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){5, 1, 3, -1, -1};
    // Row 6
    trees[0].nodes[trees[0].node_count++] = (tree_node_t){6, 0, 1, 4, 1};  // req [4,1]

    // Fury tree
    trees[1].node_count = 0;
    // Row 0
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){0, 0, 5, -1, -1};
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){0, 1, 5, -1, -1};
    // Row 1
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){1, 0, 5, -1, -1};
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){1, 1, 5, -1, -1};
    // Row 2
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){2, 0, 3, -1, -1};
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){2, 1, 1, -1, -1};
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){2, 2, 3, -1, -1};
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){2, 3, 5, -1, -1};
    // Row 3
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){3, 0, 5, -1, -1};
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){3, 1, 2, -1, -1};
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){3, 2, 5, -1, -1};
    // Row 4
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){4, 0, 5, -1, -1};
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){4, 1, 1, -1, -1};
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){4, 2, 2, -1, -1};
    // Row 5
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){5, 0, 2, -1, -1};
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){5, 1, 5, 3, 2};  // req [3,2]
    // Row 6
    trees[1].nodes[trees[1].node_count++] = (tree_node_t){6, 0, 1, 4, 1};  // req [4,1]

    // Protection tree
    trees[2].node_count = 0;
    // Row 0
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){0, 0, 5, -1, -1};
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){0, 1, 5, -1, -1};
    // Row 1
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){1, 0, 2, -1, -1};
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){1, 1, 5, -1, -1};
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){1, 2, 5, -1, -1};
    // Row 2
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){2, 0, 1, 1, 0};  // req [1,0]
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){2, 1, 3, 0, 0};  // req [0,0]
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){2, 2, 3, -1, -1};
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){2, 3, 5, -1, -1};
    // Row 3
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){3, 0, 3, -1, -1};
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){3, 1, 3, -1, -1};
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){3, 2, 2, -1, -1};
    // Row 4
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){4, 0, 2, -1, -1};
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){4, 1, 1, -1, -1};
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){4, 2, 2, -1, -1};
    // Row 5
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){5, 0, 5, -1, -1};
    // Row 6
    trees[2].nodes[trees[2].node_count++] = (tree_node_t){6, 0, 1, 4, 1};  // req [4,1]

    printf("Initialized trees: Arms(%d nodes), Fury(%d nodes), Protection(%d nodes)\n",
           trees[0].node_count, trees[1].node_count, trees[2].node_count);
}

// Calculate total combinations using convolution
long long calculate_total_combinations(int total_points) {
    long long total = 0;
    
    printf("\nCalculating total combinations for %d points using convolution...\n", total_points);
    
    // Triple nested loop for convolution
    for (int arms_pts = 0; arms_pts <= total_points; arms_pts++) {
        for (int fury_pts = 0; fury_pts <= total_points - arms_pts; fury_pts++) {
            int prot_pts = total_points - arms_pts - fury_pts;
            
            if (prot_pts >= 0 && prot_pts <= MAX_TREE_POINTS) {
                long long combination_ways = trees[0].results[arms_pts] * 
                                           trees[1].results[fury_pts] * 
                                           trees[2].results[prot_pts];
                total += combination_ways;
                
                if (combination_ways > 0 && arms_pts + fury_pts + prot_pts <= 10) {
                    printf("  Arms:%d + Fury:%d + Prot:%d = %lld × %lld × %lld = %lld\n",
                           arms_pts, fury_pts, prot_pts,
                           trees[0].results[arms_pts], trees[1].results[fury_pts], 
                           trees[2].results[prot_pts], combination_ways);
                }
            }
        }
    }
    
    return total;
}

int main(int argc, char* argv[]) {
    printf("Skill Tree Convolution Calculator\n");
    printf("=================================\n");
    
    int target_points = TOTAL_POINTS;
    if (argc > 1) {
        target_points = atoi(argv[1]);
        if (target_points <= 0 || target_points > 200) {
            printf("Error: Point total must be between 1 and 200\n");
            return 1;
        }
    }
    
    clock_t start_time = clock();
    time_t start_wall_time = time(NULL);
    
    // Initialize tree data
    initialize_tree_data();
    
    // Calculate each tree independently up to target_points
    calculate_tree_results(&trees[0], target_points, "Arms");
    calculate_tree_results(&trees[1], target_points, "Fury");
    calculate_tree_results(&trees[2], target_points, "Protection");
    
    // Combine results using convolution
    long long result = calculate_total_combinations(target_points);
    
    clock_t end_time = clock();
    time_t end_wall_time = time(NULL);
    
    double cpu_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    double wall_time = difftime(end_wall_time, start_wall_time);
    
    printf("\nFinal Results:\n");
    printf("==============\n");
    printf("Total combinations for %d points: %lld\n", target_points, result);
    printf("CPU time: %.2f seconds\n", cpu_time);
    printf("Wall time: %.0f seconds\n", wall_time);
    
    // Cleanup
    for (int i = 0; i < 3; i++) {
        if (trees[i].results) free(trees[i].results);
    }
    
    return 0;
}
