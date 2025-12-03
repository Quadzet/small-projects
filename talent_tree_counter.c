#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_POINTS 51
#define MAX_TALENTS_PER_ROW 4
#define MAX_ROWS 7
#define MAX_TREES 3

// Talent structure
typedef struct {
    int max_points;
    bool has_req;
    char req_type;  // '/' or '\'
    int max_above;
} Talent;

// Row structure
typedef struct {
    Talent talents[MAX_TALENTS_PER_ROW];
    int num_talents;
} Row;

// Tree structure
typedef struct {
    Row rows[MAX_ROWS];
    int num_rows;
} Tree;

// Distribution entry for memoization
typedef struct {
    int points;
    long long count;
} DistEntry;

// Hash table for memoization
#define HASH_SIZE 100003
typedef struct HashNode {
    int row_idx;
    int points;
    int parent_alloc[MAX_TALENTS_PER_ROW];
    int parent_len;
    DistEntry* dist;
    int dist_size;
    struct HashNode* next;
} HashNode;

HashNode* hash_table[HASH_SIZE];

// Hash function
unsigned int hash_func(int row_idx, int points, int* parent_alloc, int parent_len) {
    unsigned int hash = row_idx * 1000 + points;
    for (int i = 0; i < parent_len; i++) {
        hash = hash * 31 + parent_alloc[i];
    }
    return hash % HASH_SIZE;
}

// Check if allocation vectors match
bool alloc_equal(int* a1, int len1, int* a2, int len2) {
    if (len1 != len2) return false;
    for (int i = 0; i < len1; i++) {
        if (a1[i] != a2[i]) return false;
    }
    return true;
}

// Initialize trees with talent data
void init_trees(Tree trees[MAX_TREES]) {
    // Arms tree
    Tree* arms = &trees[0];
    arms->num_rows = 7;
    
    // Row 0
    arms->rows[0].num_talents = 3;
    arms->rows[0].talents[0] = (Talent){3, false, 0, 0};
    arms->rows[0].talents[1] = (Talent){5, false, 0, 0};
    arms->rows[0].talents[2] = (Talent){3, false, 0, 0};
    
    // Row 1
    arms->rows[1].num_talents = 3;
    arms->rows[1].talents[0] = (Talent){2, false, 0, 0};
    arms->rows[1].talents[1] = (Talent){5, false, 0, 0};
    arms->rows[1].talents[2] = (Talent){3, false, 0, 0};
    
    // Row 2
    arms->rows[2].num_talents = 3;
    arms->rows[2].talents[0] = (Talent){2, false, 0, 0};
    arms->rows[2].talents[1] = (Talent){1, true, '\\', 1};
    arms->rows[2].talents[2] = (Talent){3, true, '\\', 3};
    
    // Row 3
    arms->rows[3].num_talents = 2;
    arms->rows[3].talents[0] = (Talent){5, false, 0, 0};
    arms->rows[3].talents[1] = (Talent){2, true, '\\', 2};
    
    // Row 4
    arms->rows[4].num_talents = 4;
    arms->rows[4].talents[0] = (Talent){5, false, 0, 0};
    arms->rows[4].talents[1] = (Talent){1, false, 0, 0};
    arms->rows[4].talents[2] = (Talent){5, false, 0, 0};
    arms->rows[4].talents[3] = (Talent){5, false, 0, 0};
    
    // Row 5
    arms->rows[5].num_talents = 2;
    arms->rows[5].talents[0] = (Talent){5, false, 0, 0};
    arms->rows[5].talents[1] = (Talent){3, false, 0, 0};
    
    // Row 6
    arms->rows[6].num_talents = 1;
    arms->rows[6].talents[0] = (Talent){1, true, '\\', 1};

    // Fury tree
    Tree* fury = &trees[1];
    fury->num_rows = 7;
    
    // Row 0
    fury->rows[0].num_talents = 2;
    fury->rows[0].talents[0] = (Talent){5, false, 0, 0};
    fury->rows[0].talents[1] = (Talent){5, false, 0, 0};
    
    // Row 1
    fury->rows[1].num_talents = 2;
    fury->rows[1].talents[0] = (Talent){5, false, 0, 0};
    fury->rows[1].talents[1] = (Talent){5, false, 0, 0};
    
    // Row 2
    fury->rows[2].num_talents = 4;
    fury->rows[2].talents[0] = (Talent){3, false, 0, 0};
    fury->rows[2].talents[1] = (Talent){1, false, 0, 0};
    fury->rows[2].talents[2] = (Talent){3, false, 0, 0};
    fury->rows[2].talents[3] = (Talent){5, false, 0, 0};
    
    // Row 3
    fury->rows[3].num_talents = 3;
    fury->rows[3].talents[0] = (Talent){5, false, 0, 0};
    fury->rows[3].talents[1] = (Talent){2, false, 0, 0};
    fury->rows[3].talents[2] = (Talent){5, false, 0, 0};
    
    // Row 4
    fury->rows[4].num_talents = 3;
    fury->rows[4].talents[0] = (Talent){5, false, 0, 0};
    fury->rows[4].talents[1] = (Talent){1, false, 0, 0};
    fury->rows[4].talents[2] = (Talent){2, false, 0, 0};
    
    // Row 5
    fury->rows[5].num_talents = 2;
    fury->rows[5].talents[0] = (Talent){2, false, 0, 0};
    fury->rows[5].talents[1] = (Talent){5, true, '\\', 5};
    
    // Row 6
    fury->rows[6].num_talents = 1;
    fury->rows[6].talents[0] = (Talent){1, true, '\\', 1};

    // Protection tree
    Tree* prot = &trees[2];
    prot->num_rows = 7;
    
    // Row 0
    prot->rows[0].num_talents = 2;
    prot->rows[0].talents[0] = (Talent){5, false, 0, 0};
    prot->rows[0].talents[1] = (Talent){5, false, 0, 0};
    
    // Row 1
    prot->rows[1].num_talents = 3;
    prot->rows[1].talents[0] = (Talent){2, false, 0, 0};
    prot->rows[1].talents[1] = (Talent){5, false, 0, 0};
    prot->rows[1].talents[2] = (Talent){5, false, 0, 0};
    
    // Row 2
    prot->rows[2].num_talents = 4;
    prot->rows[2].talents[0] = (Talent){1, true, '\\', 1};
    prot->rows[2].talents[1] = (Talent){3, true, '\\', 3};
    prot->rows[2].talents[2] = (Talent){3, false, 0, 0};
    prot->rows[2].talents[3] = (Talent){5, false, 0, 0};
    
    // Row 3
    prot->rows[3].num_talents = 3;
    prot->rows[3].talents[0] = (Talent){3, false, 0, 0};
    prot->rows[3].talents[1] = (Talent){3, false, 0, 0};
    prot->rows[3].talents[2] = (Talent){2, false, 0, 0};
    
    // Row 4
    prot->rows[4].num_talents = 3;
    prot->rows[4].talents[0] = (Talent){2, false, 0, 0};
    prot->rows[4].talents[1] = (Talent){1, false, 0, 0};
    prot->rows[4].talents[2] = (Talent){2, false, 0, 0};
    
    // Row 5
    prot->rows[5].num_talents = 1;
    prot->rows[5].talents[0] = (Talent){5, false, 0, 0};
    
    // Row 6
    prot->rows[6].num_talents = 1;
    prot->rows[6].talents[0] = (Talent){1, true, '\\', 1};
}

// Check if prerequisite is satisfied
bool check_prerequisite(Talent* talent, int parent_val) {
    if (!talent->has_req) return true;
    
    if (talent->req_type == '/') {
        return parent_val > 0;
    } else if (talent->req_type == '\\') {
        return parent_val == talent->max_above;
    }
    return false;
}

// Structure to hold row options
typedef struct {
    int points;
    int allocation[MAX_TALENTS_PER_ROW];
    int alloc_len;
} RowOption;

typedef struct {
    RowOption* options;
    int count;
    int capacity;
} RowOptions;

// Generate all valid allocations for a row
void row_options(Row* row, int* parent_row, int parent_len, RowOptions* result) {
    result->count = 0;
    result->capacity = 1000;
    result->options = malloc(result->capacity * sizeof(RowOption));
    
    int allocation[MAX_TALENTS_PER_ROW];
    
    void backtrack(int i, int cur_sum) {
        if (i == row->num_talents) {
            if (result->count >= result->capacity) {
                result->capacity *= 2;
                result->options = realloc(result->options, result->capacity * sizeof(RowOption));
            }
            result->options[result->count].points = cur_sum;
            result->options[result->count].alloc_len = row->num_talents;
            memcpy(result->options[result->count].allocation, allocation, row->num_talents * sizeof(int));
            result->count++;
            return;
        }
        
        Talent* talent = &row->talents[i];
        int parent_val = (parent_row && i < parent_len) ? parent_row[i] : 0;
        
        for (int r = 0; r <= talent->max_points; r++) {
            if (r == 0) {
                allocation[i] = 0;
                backtrack(i + 1, cur_sum);
            } else {
                if (!talent->has_req || (parent_row && i < parent_len && check_prerequisite(talent, parent_val))) {
                    allocation[i] = r;
                    backtrack(i + 1, cur_sum + r);
                }
            }
        }
    }
    
    backtrack(0, 0);
}

// Find cached result in hash table
HashNode* find_cached(int row_idx, int points, int* parent_alloc, int parent_len) {
    unsigned int hash = hash_func(row_idx, points, parent_alloc, parent_len);
    HashNode* node = hash_table[hash];
    
    while (node) {
        if (node->row_idx == row_idx && node->points == points &&
            alloc_equal(node->parent_alloc, node->parent_len, parent_alloc, parent_len)) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

// Cache result in hash table
void cache_result(int row_idx, int points, int* parent_alloc, int parent_len, DistEntry* dist, int dist_size) {
    unsigned int hash = hash_func(row_idx, points, parent_alloc, parent_len);
    HashNode* node = malloc(sizeof(HashNode));
    
    node->row_idx = row_idx;
    node->points = points;
    node->parent_len = parent_len;
    memcpy(node->parent_alloc, parent_alloc, parent_len * sizeof(int));
    node->dist_size = dist_size;
    node->dist = malloc(dist_size * sizeof(DistEntry));
    memcpy(node->dist, dist, dist_size * sizeof(DistEntry));
    
    node->next = hash_table[hash];
    hash_table[hash] = node;
}

// Merge two distributions
void merge_distributions(DistEntry* dist1, int size1, DistEntry* dist2, int size2, DistEntry** result, int* result_size) {
    // Simple approach: combine and deduplicate
    DistEntry* temp = malloc((size1 + size2) * sizeof(DistEntry));
    int temp_size = 0;
    
    // Copy first distribution
    for (int i = 0; i < size1; i++) {
        temp[temp_size++] = dist1[i];
    }
    
    // Add second distribution, combining duplicates
    for (int i = 0; i < size2; i++) {
        bool found = false;
        for (int j = 0; j < temp_size; j++) {
            if (temp[j].points == dist2[i].points) {
                temp[j].count += dist2[i].count;
                found = true;
                break;
            }
        }
        if (!found) {
            temp[temp_size++] = dist2[i];
        }
    }
    
    *result = temp;
    *result_size = temp_size;
}

// Global variables for the DP function to communicate results
static DistEntry global_temp_result[MAX_POINTS + 1];
static int global_temp_size;

// Calculate distribution for a tree using dynamic programming
void tree_distribution(Tree* tree, DistEntry** result, int* result_size) {
    // Clear hash table
    memset(hash_table, 0, sizeof(hash_table));
    
    DistEntry* dp(int row_idx, int pts, int* parent_alloc, int parent_len) {
        // Check cache
        HashNode* cached = find_cached(row_idx, pts, parent_alloc, parent_len);
        if (cached) {
            memcpy(global_temp_result, cached->dist, cached->dist_size * sizeof(DistEntry));
            global_temp_size = cached->dist_size;
            return global_temp_result;
        }
        
        if (row_idx == tree->num_rows) {
            global_temp_result[0] = (DistEntry){pts, 1};
            global_temp_size = 1;
            cache_result(row_idx, pts, parent_alloc, parent_len, global_temp_result, global_temp_size);
            return global_temp_result;
        }
        
        bool unlocked = (pts >= 5 * row_idx);
        global_temp_size = 0;
        
        if (unlocked) {
            RowOptions options;
            row_options(&tree->rows[row_idx], parent_alloc, parent_len, &options);
            
            for (int i = 0; i < options.count; i++) {
                int new_pts = pts + options.options[i].points;
                if (new_pts > MAX_POINTS) continue;
                
                DistEntry* sub_dist = dp(row_idx + 1, new_pts, 
                                       options.options[i].allocation, 
                                       options.options[i].alloc_len);
                
                // Merge subdistribution into current result
                DistEntry* merged;
                int merged_size;
                merge_distributions(global_temp_result, global_temp_size, sub_dist, 1, &merged, &merged_size);
                
                // Copy merged result back to global buffer
                memcpy(global_temp_result, merged, merged_size * sizeof(DistEntry));
                global_temp_size = merged_size;
                free(merged);
            }
            
            free(options.options);
        } else {
            // Cannot spend in this row yet
            int no_alloc[1] = {-1};  // Special marker for no allocation
            DistEntry* sub_dist = dp(row_idx + 1, pts, no_alloc, 1);
            memcpy(global_temp_result, sub_dist, sizeof(DistEntry));
            global_temp_size = 1;
        }
        
        cache_result(row_idx, pts, parent_alloc, parent_len, global_temp_result, global_temp_size);
        return global_temp_result;
    }
    
    int no_parent[1] = {-1};
    DistEntry* dist = dp(0, 0, no_parent, 1);
    
    *result = malloc(global_temp_size * sizeof(DistEntry));
    memcpy(*result, dist, global_temp_size * sizeof(DistEntry));
    *result_size = global_temp_size;
}

// Convolve two distributions
void convolve_counts(DistEntry* a, int size_a, DistEntry* b, int size_b, DistEntry** result, int* result_size) {
    DistEntry* temp = malloc(size_a * size_b * sizeof(DistEntry));
    int temp_size = 0;
    
    for (int i = 0; i < size_a; i++) {
        for (int j = 0; j < size_b; j++) {
            int k = a[i].points + b[j].points;
            if (k <= MAX_POINTS) {
                // Check if this point total already exists
                bool found = false;
                for (int l = 0; l < temp_size; l++) {
                    if (temp[l].points == k) {
                        temp[l].count += a[i].count * b[j].count;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    temp[temp_size] = (DistEntry){k, a[i].count * b[j].count};
                    temp_size++;
                }
            }
        }
    }
    
    *result = temp;
    *result_size = temp_size;
}

// Free hash table
void cleanup_hash_table() {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode* node = hash_table[i];
        while (node) {
            HashNode* next = node->next;
            free(node->dist);
            free(node);
            node = next;
        }
    }
}

int main() {
    Tree trees[MAX_TREES];
    init_trees(trees);
    
    DistEntry* arms_dist;
    DistEntry* fury_dist;
    DistEntry* prot_dist;
    int arms_size, fury_size, prot_size;
    
    printf("Calculating Arms distribution...\n");
    tree_distribution(&trees[0], &arms_dist, &arms_size);
    
    printf("Calculating Fury distribution...\n");
    tree_distribution(&trees[1], &fury_dist, &fury_size);
    
    printf("Calculating Protection distribution...\n");
    tree_distribution(&trees[2], &prot_dist, &prot_size);
    
    printf("Convolving Arms and Fury...\n");
    DistEntry* af_dist;
    int af_size;
    convolve_counts(arms_dist, arms_size, fury_dist, fury_size, &af_dist, &af_size);
    
    printf("Computing final result...\n");
    long long total = 0;
    for (int i = 0; i < prot_size; i++) {
        int need = MAX_POINTS - prot_dist[i].points;
        for (int j = 0; j < af_size; j++) {
            if (af_dist[j].points == need) {
                total += (long long)af_dist[j].count * prot_dist[i].count;
                break;
            }
        }
    }
    
    printf("Total valid combinations spending exactly %d points: %lld\n", MAX_POINTS, total);
    
    // Cleanup
    free(arms_dist);
    free(fury_dist);
    free(prot_dist);
    free(af_dist);
    cleanup_hash_table();
    
    return 0;
}
