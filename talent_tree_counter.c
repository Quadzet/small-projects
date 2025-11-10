#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Constants
#define TOTAL_POINTS 51
#define MAX_NODES_PER_TREE 20
#define TOTAL_TREES 3
#define MEMO_INTERVAL 1  // Cache every iteration now
#define MEMO_TABLE_SIZE 10000000  // 10M entries

// Bit-packed state with tree totals (3 bits per node + 8 bits per tree total)
typedef struct {
    uint64_t bits[3];        // 192 bits total, enough for 64 nodes (3 bits each)
    uint16_t tree_totals[3]; // Arms, Fury, Protection point totals
} state_t;

// Memoization structures (simplified - no remaining_points needed)
typedef struct memo_entry {
    state_t state;
    long long result;
    struct memo_entry* next;
} memo_entry_t;

typedef struct {
    memo_entry_t* buckets[MEMO_TABLE_SIZE];
    long long hits;
    long long misses;
    long long stores;
} memo_table_t;

// Global progress tracking and memoization
long long recursion_calls = 0;
int max_depth = 0;
memo_table_t memo_table;

// Node information structure
typedef struct {
    int tree;        // 0=Arms, 1=Fury, 2=Protection
    int row;
    int col;
    int max_points;
    int req_tree;    // -1 if no requirement
    int req_row;     // -1 if no requirement
    int req_col;     // -1 if no requirement
    int linear_idx;  // Linear index for bit manipulation
} node_info_t;

// Global node information
node_info_t nodes[64];  // More than enough for our trees
int total_nodes = 0;

// Inline functions for bit manipulation
static inline int get_node_points(const state_t* state, int node_idx) {
    int word = node_idx / 21;  // 64/3 = 21 nodes per uint64_t (with 1 bit leftover)
    int bit_pos = (node_idx % 21) * 3;
    return (state->bits[word] >> bit_pos) & 7;
}

static inline void set_node_points(state_t* state, int node_idx, int points) {
    int word = node_idx / 21;
    int bit_pos = (node_idx % 21) * 3;
    state->bits[word] &= ~(7ULL << bit_pos);  // Clear 3 bits
    state->bits[word] |= ((uint64_t)points << bit_pos);  // Set new value
}

// Hash function for memoization (state only)
uint64_t hash_state(const state_t* state) {
    uint64_t hash = 0x9e3779b9;  // Golden ratio hash constant
    
    // Hash the state bits
    hash ^= state->bits[0] + (hash << 6) + (hash >> 2);
    hash ^= state->bits[1] + (hash << 6) + (hash >> 2);
    hash ^= state->bits[2] + (hash << 6) + (hash >> 2);
    
    // Hash the tree totals
    hash ^= ((uint64_t)state->tree_totals[0] << 16 | 
             (uint64_t)state->tree_totals[1] << 8 | 
             (uint64_t)state->tree_totals[2]) + (hash << 6) + (hash >> 2);
    
    return hash;
}

// Compare two states for equality (updated for new state structure)
bool states_equal(const state_t* state1, const state_t* state2) {
    return (state1->bits[0] == state2->bits[0] && 
            state1->bits[1] == state2->bits[1] && 
            state1->bits[2] == state2->bits[2] &&
            state1->tree_totals[0] == state2->tree_totals[0] &&
            state1->tree_totals[1] == state2->tree_totals[1] &&
            state1->tree_totals[2] == state2->tree_totals[2]);
}

// Initialize memoization table
void init_memo_table() {
    memset(&memo_table, 0, sizeof(memo_table));
}

// Look up a state in the memoization table
long long lookup_memo(const state_t* state) {
    uint64_t hash = hash_state(state);
    int bucket = hash % MEMO_TABLE_SIZE;
    
    memo_entry_t* entry = memo_table.buckets[bucket];
    while (entry) {
        if (states_equal(&entry->state, state)) {
            memo_table.hits++;
            return entry->result;
        }
        entry = entry->next;
    }
    
    memo_table.misses++;
    return -1;  // Not found
}

// Store a result in the memoization table
void store_memo(const state_t* state, long long result) {
    uint64_t hash = hash_state(state);
    int bucket = hash % MEMO_TABLE_SIZE;
    
    // Create new entry
    memo_entry_t* new_entry = malloc(sizeof(memo_entry_t));
    new_entry->state = *state;
    new_entry->result = result;
    new_entry->next = memo_table.buckets[bucket];
    
    memo_table.buckets[bucket] = new_entry;
    memo_table.stores++;
}

// Free memoization table
void free_memo_table() {
    for (int i = 0; i < MEMO_TABLE_SIZE; i++) {
        memo_entry_t* entry = memo_table.buckets[i];
        while (entry) {
            memo_entry_t* next = entry->next;
            free(entry);
            entry = next;
        }
    }
}

// Helper function to find node index by tree/row/col
int find_node_index(int tree, int row, int col) {
    for (int i = 0; i < total_nodes; i++) {
        if (nodes[i].tree == tree && nodes[i].row == row && nodes[i].col == col) {
            return i;
        }
    }
    return -1;  // Not found
}

// Check if a node is available for point allocation (optimized)
bool is_node_available(const state_t* state, int node_idx) {
    const node_info_t* node = &nodes[node_idx];
    
    // Check if node is already maxed out
    if (get_node_points(state, node_idx) >= node->max_points) {
        return false;
    }
    
    // Check row requirement (need row_index * 5 points in tree)
    // Use pre-calculated tree total instead of recalculating
    if (state->tree_totals[node->tree] < node->row * 5) {
        return false;
    }
    
    // Check prerequisite requirement
    if (node->req_tree != -1) {
        int req_node_idx = find_node_index(node->req_tree, node->req_row, node->req_col);
        if (req_node_idx == -1) {
            printf("Error: Required node not found!\n");
            return false;
        }
        int req_max = nodes[req_node_idx].max_points;
        if (get_node_points(state, req_node_idx) < req_max) {
            return false;
        }
    }
    
    return true;
}

// Main recursive counting function (optimized with simplified memoization)
long long count_allocations(state_t current_state, int remaining_points) {
    recursion_calls++;
    int depth = TOTAL_POINTS - remaining_points;
    if (depth > max_depth) max_depth = depth;
    
    // Progress indicator every million calls
    if (recursion_calls % 1000000 == 0) {
        printf("Progress: %lld million calls, max depth %d, remaining points %d\n", 
               recursion_calls / 1000000, max_depth, remaining_points);
        printf("  Memo stats - Hits: %lld, Misses: %lld, Stores: %lld, Hit rate: %.2f%%\n",
               memo_table.hits, memo_table.misses, memo_table.stores,
               memo_table.hits + memo_table.misses > 0 ? 
               100.0 * memo_table.hits / (memo_table.hits + memo_table.misses) : 0.0);
    }
    
    // Base case
    if (remaining_points == 0) return 1;
    if (remaining_points < 0) return 0;
    
    // Check memoization (state only - remaining_points is implicit)
    long long cached_result = lookup_memo(&current_state);
    if (cached_result != -1) {
        return cached_result;
    }
    
    long long total_ways = 0;
    
    // Find all currently available nodes and try allocating points
    for (int i = 0; i < total_nodes; i++) {
        if (is_node_available(&current_state, i)) {
            int current_points = get_node_points(&current_state, i);
            int max_can_add = (remaining_points < (nodes[i].max_points - current_points)) ? 
                             remaining_points : (nodes[i].max_points - current_points);
            
            // Try adding 1 to max_can_add points to this node
            for (int add = 1; add <= max_can_add; add++) {
                state_t new_state = current_state;
                set_node_points(&new_state, i, current_points + add);
                
                // Update the tree total for the modified tree
                new_state.tree_totals[nodes[i].tree] += add;
                
                total_ways += count_allocations(new_state, remaining_points - add);
            }
        }
    }
    
    // Store in memo table
    store_memo(&current_state, total_ways);
    
    return total_ways;
}

// Initialize node data from the given tree structures
void initialize_nodes() {
    // Arms tree data
    int arms_data[][4] = {
        // Row 0: {max, req_row, req_col, has_req}
        {3, -1, -1, 0}, {5, -1, -1, 0}, {3, -1, -1, 0},
        // Row 1
        {2, -1, -1, 0}, {5, -1, -1, 0}, {3, -1, -1, 0},
        // Row 2
        {2, -1, -1, 0}, {1, 1, 1, 1}, {3, 0, 2, 1},
        // Row 3
        {5, -1, -1, 0}, {2, 2, 2, 1},
        // Row 4
        {5, -1, -1, 0}, {1, -1, -1, 0}, {5, -1, -1, 0}, {5, -1, -1, 0},
        // Row 5
        {5, -1, -1, 0}, {3, -1, -1, 0},
        // Row 6
        {1, 4, 1, 1}
    };
    
    int arms_structure[] = {3, 3, 3, 2, 4, 2, 1}; // nodes per row
    
    // Fury tree data
    int fury_data[][4] = {
        // Row 0
        {5, -1, -1, 0}, {5, -1, -1, 0},
        // Row 1
        {5, -1, -1, 0}, {5, -1, -1, 0},
        // Row 2
        {3, -1, -1, 0}, {1, -1, -1, 0}, {3, -1, -1, 0}, {5, -1, -1, 0},
        // Row 3
        {5, -1, -1, 0}, {2, -1, -1, 0}, {5, -1, -1, 0},
        // Row 4
        {5, -1, -1, 0}, {1, -1, -1, 0}, {2, -1, -1, 0},
        // Row 5
        {2, -1, -1, 0}, {5, 3, 2, 1},
        // Row 6
        {1, 4, 1, 1}
    };
    
    int fury_structure[] = {2, 2, 4, 3, 3, 2, 1};
    
    // Protection tree data
    int prot_data[][4] = {
        // Row 0
        {5, -1, -1, 0}, {5, -1, -1, 0},
        // Row 1
        {2, -1, -1, 0}, {5, -1, -1, 0}, {5, -1, -1, 0},
        // Row 2
        {1, 1, 0, 1}, {3, 0, 0, 1}, {3, -1, -1, 0}, {5, -1, -1, 0},
        // Row 3
        {3, -1, -1, 0}, {3, -1, -1, 0}, {2, -1, -1, 0},
        // Row 4
        {2, -1, -1, 0}, {1, -1, -1, 0}, {2, -1, -1, 0},
        // Row 5
        {5, -1, -1, 0},
        // Row 6
        {1, 4, 1, 1}
    };
    
    int prot_structure[] = {2, 3, 4, 3, 3, 1, 1};
    
    total_nodes = 0;
    
    // Process Arms tree (tree 0)
    int data_idx = 0;
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < arms_structure[row]; col++) {
            nodes[total_nodes].tree = 0;
            nodes[total_nodes].row = row;
            nodes[total_nodes].col = col;
            nodes[total_nodes].max_points = arms_data[data_idx][0];
            
            if (arms_data[data_idx][3] == 1) { // has requirement
                nodes[total_nodes].req_tree = 0; // Same tree
                nodes[total_nodes].req_row = arms_data[data_idx][1];
                nodes[total_nodes].req_col = arms_data[data_idx][2];
            } else {
                nodes[total_nodes].req_tree = -1;
                nodes[total_nodes].req_row = -1;
                nodes[total_nodes].req_col = -1;
            }
            
            nodes[total_nodes].linear_idx = total_nodes;
            total_nodes++;
            data_idx++;
        }
    }
    
    // Process Fury tree (tree 1)
    data_idx = 0;
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < fury_structure[row]; col++) {
            nodes[total_nodes].tree = 1;
            nodes[total_nodes].row = row;
            nodes[total_nodes].col = col;
            nodes[total_nodes].max_points = fury_data[data_idx][0];
            
            if (fury_data[data_idx][3] == 1) { // has requirement
                nodes[total_nodes].req_tree = 1; // Same tree
                nodes[total_nodes].req_row = fury_data[data_idx][1];
                nodes[total_nodes].req_col = fury_data[data_idx][2];
            } else {
                nodes[total_nodes].req_tree = -1;
                nodes[total_nodes].req_row = -1;
                nodes[total_nodes].req_col = -1;
            }
            
            nodes[total_nodes].linear_idx = total_nodes;
            total_nodes++;
            data_idx++;
        }
    }
    
    // Process Protection tree (tree 2)
    data_idx = 0;
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < prot_structure[row]; col++) {
            nodes[total_nodes].tree = 2;
            nodes[total_nodes].row = row;
            nodes[total_nodes].col = col;
            nodes[total_nodes].max_points = prot_data[data_idx][0];
            
            if (prot_data[data_idx][3] == 1) { // has requirement
                nodes[total_nodes].req_tree = 2; // Same tree
                nodes[total_nodes].req_row = prot_data[data_idx][1];
                nodes[total_nodes].req_col = prot_data[data_idx][2];
            } else {
                nodes[total_nodes].req_tree = -1;
                nodes[total_nodes].req_row = -1;
                nodes[total_nodes].req_col = -1;
            }
            
            nodes[total_nodes].linear_idx = total_nodes;
            total_nodes++;
            data_idx++;
        }
    }
    
    printf("Initialized %d total nodes\n", total_nodes);
}

// Debug function to print current state
void print_state(const state_t* state) {
    printf("Current allocation:\n");
    for (int tree = 0; tree < 3; tree++) {
        const char* tree_names[] = {"Arms", "Fury", "Protection"};
        printf("%s tree:\n", tree_names[tree]);
        for (int i = 0; i < total_nodes; i++) {
            if (nodes[i].tree == tree) {
                int points = get_node_points(state, i);
                if (points > 0) {
                    printf("  Row %d, Col %d: %d/%d points\n", 
                           nodes[i].row, nodes[i].col, points, nodes[i].max_points);
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    printf("Skill Tree Allocation Counter\n");
    printf("============================\n");
    
    // Reset global counters
    recursion_calls = 0;
    max_depth = 0;
    
    // Initialize memoization table
    init_memo_table();
    
    // Allow custom point total from command line
    int points_to_allocate = TOTAL_POINTS;
    if (argc > 1) {
        points_to_allocate = atoi(argv[1]);
        if (points_to_allocate <= 0 || points_to_allocate > 200) {
            printf("Error: Point total must be between 1 and 200\n");
            return 1;
        }
    }
    
    // Initialize the node data
    initialize_nodes();
    
    // Create initial empty state with zero tree totals
    state_t initial_state;
    memset(&initial_state, 0, sizeof(initial_state));
    // Tree totals start at 0 for all trees
    initial_state.tree_totals[0] = 0;  // Arms
    initial_state.tree_totals[1] = 0;  // Fury  
    initial_state.tree_totals[2] = 0;  // Protection
    
    printf("Calculating number of ways to allocate %d points...\n", points_to_allocate);
    printf("Using simplified memoization (state-only keys) with tree total caching.\n");
    printf("This may take a while...\n\n");
    
    // Start timing
    clock_t start_time = clock();
    time_t start_wall_time = time(NULL);
    
    // Count all possible allocations
    long long result = count_allocations(initial_state, points_to_allocate);
    
    // End timing
    clock_t end_time = clock();
    time_t end_wall_time = time(NULL);
    
    double cpu_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    double wall_time = difftime(end_wall_time, start_wall_time);
    
    printf("\nResults:\n");
    printf("========\n");
    printf("Total number of valid skill tree allocations: %lld\n", result);
    printf("Total recursive calls made: %lld\n", recursion_calls);
    printf("Maximum recursion depth reached: %d\n", max_depth);
    printf("\nTiming:\n");
    printf("CPU time: %.2f seconds\n", cpu_time);
    printf("Wall time: %.0f seconds (%.2f minutes)\n", wall_time, wall_time / 60.0);
    printf("Calls per second: %.0f\n", recursion_calls / wall_time);
    printf("\nMemoization Statistics:\n");
    printf("Cache hits: %lld\n", memo_table.hits);
    printf("Cache misses: %lld\n", memo_table.misses);
    printf("Cache stores: %lld\n", memo_table.stores);
    printf("Hit rate: %.2f%%\n", 
           memo_table.hits + memo_table.misses > 0 ? 
           100.0 * memo_table.hits / (memo_table.hits + memo_table.misses) : 0.0);
    
    // Cleanup
    free_memo_table();
    
    return 0;
}
