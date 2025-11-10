#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// Constants
#define TOTAL_POINTS 51
#define MAX_NODES_PER_TREE 20
#define TOTAL_TREES 3

// Global progress tracking
long long recursion_calls = 0;
int max_depth = 0;

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

// Bit-packed state (3 bits per node, up to 64 nodes)
typedef struct {
    uint64_t bits[3];  // 192 bits total, enough for 64 nodes
} state_t;

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

// Helper function to find total points in a tree
int calculate_tree_total(const state_t* state, int tree_id) {
    int total = 0;
    for (int i = 0; i < total_nodes; i++) {
        if (nodes[i].tree == tree_id) {
            total += get_node_points(state, i);
        }
    }
    return total;
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

// Check if a node is available for point allocation
bool is_node_available(const state_t* state, int node_idx) {
    const node_info_t* node = &nodes[node_idx];
    
    // Check if node is already maxed out
    if (get_node_points(state, node_idx) >= node->max_points) {
        return false;
    }
    
    // Check row requirement (need row_index * 5 points in tree)
    int tree_total = calculate_tree_total(state, node->tree);
    if (tree_total < node->row * 5) {
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

// Main recursive counting function
long long count_allocations(state_t current_state, int remaining_points) {
    recursion_calls++;
    int depth = TOTAL_POINTS - remaining_points;
    if (depth > max_depth) max_depth = depth;
    
    // Progress indicator every million calls
    if (recursion_calls % 1000000 == 0) {
        printf("Progress: %lld million calls, max depth %d, remaining points %d\n", 
               recursion_calls / 1000000, max_depth, remaining_points);
    }
    
    // Base case
    if (remaining_points == 0) return 1;
    if (remaining_points < 0) return 0;
    
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
                
                total_ways += count_allocations(new_state, remaining_points - add);
            }
        }
    }
    
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
    
    // Create initial empty state
    state_t initial_state;
    memset(&initial_state, 0, sizeof(initial_state));
    
    printf("Calculating number of ways to allocate %d points...\n", points_to_allocate);
    printf("This may take a while...\n\n");
    
    // Count all possible allocations
    long long result = count_allocations(initial_state, points_to_allocate);
    
    printf("\nResults:\n");
    printf("========\n");
    printf("Total number of valid skill tree allocations: %lld\n", result);
    printf("Total recursive calls made: %lld\n", recursion_calls);
    printf("Maximum recursion depth reached: %d\n", max_depth);
    
    return 0;
}
