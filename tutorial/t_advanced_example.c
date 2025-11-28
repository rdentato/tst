//  SPDX-FileCopyrightText: © 2023 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT
//
// Advanced Example: Testing a Simple Key-Value Store
//
// This example demonstrates multiple TST features:
// - Multiple test cases with clear structure
// - Tag-based test organization (+slow, +stress, -ci)
// - Data-driven testing with tstdata
// - Sections for setup/teardown patterns
// - Error message formatting
// - Timing measurements
// - Conditional execution with tstskipif

#include "tst.h"
#include <string.h>
#include <stdlib.h>

// Simple key-value store implementation
typedef struct {
    char* key;
    char* value;
} KVPair;

typedef struct {
    KVPair* pairs;
    int count;
    int capacity;
} KVStore;

KVStore* kv_create(int capacity) {
    KVStore* store = malloc(sizeof(KVStore));
    if (!store) return NULL;
    store->pairs = malloc(capacity * sizeof(KVPair));
    if (!store->pairs) { free(store); return NULL; }
    store->count = 0;
    store->capacity = capacity;
    for (int i = 0; i < capacity; i++) {
        store->pairs[i].key = NULL;
        store->pairs[i].value = NULL;
    }
    return store;
}

void kv_destroy(KVStore* store) {
    if (!store) return;
    for (int i = 0; i < store->count; i++) {
        free(store->pairs[i].key);
        free(store->pairs[i].value);
    }
    free(store->pairs);
    free(store);
}

int kv_set(KVStore* store, const char* key, const char* value) {
    if (!store || !key || !value) return -1;
    if (store->count >= store->capacity) return -1;
    
    // Check if key exists
    for (int i = 0; i < store->count; i++) {
        if (strcmp(store->pairs[i].key, key) == 0) {
            free(store->pairs[i].value);
            store->pairs[i].value = strdup(value);
            return 0;
        }
    }
    
    // Add new pair
    store->pairs[store->count].key = strdup(key);
    store->pairs[store->count].value = strdup(value);
    store->count++;
    return 0;
}

const char* kv_get(KVStore* store, const char* key) {
    if (!store || !key) return NULL;
    for (int i = 0; i < store->count; i++) {
        if (strcmp(store->pairs[i].key, key) == 0) {
            return store->pairs[i].value;
        }
    }
    return NULL;
}

int kv_delete(KVStore* store, const char* key) {
    if (!store || !key) return -1;
    for (int i = 0; i < store->count; i++) {
        if (strcmp(store->pairs[i].key, key) == 0) {
            free(store->pairs[i].key);
            free(store->pairs[i].value);
            // Shift remaining elements
            for (int j = i; j < store->count - 1; j++) {
                store->pairs[j] = store->pairs[j + 1];
            }
            store->count--;
            return 0;
        }
    }
    return -1;
}

// Test Suite
tstsuite("Key-Value Store Tests") {
    
    // Basic functionality tests (always run)
    tstcase("Store creation and destruction") {
        KVStore* store = NULL;
        
        tstsection("Create store with valid capacity") {
            store = kv_create(10);
            tstcheck(store != NULL, "Store should be created");
            tstcheck(store->count == 0, "New store should be empty");
            tstcheck(store->capacity == 10, "Capacity should be 10, got %d", 
                     store->capacity);
        }
        
        tstsection("Create store with zero capacity") {
            store = kv_create(0);
            tstcheck(store != NULL, "Should handle zero capacity");
            if (store) {
                tstcheck(store->capacity == 0);
            }
        }
        
        // Cleanup runs after all sections
        if (store) kv_destroy(store);
    }
    
    tstcase("Basic operations") {
        KVStore* store = kv_create(5);
        tstassert(store != NULL, "Store creation failed");
        
        // Set a value
        tstcheck(kv_set(store, "name", "Alice") == 0, "Set should succeed");
        tstcheck(store->count == 1, "Count should be 1, got %d", store->count);
        
        // Get the value
        const char* value = kv_get(store, "name");
        tstcheck(value != NULL, "Get should return non-NULL");
        tstcheck(strcmp(value, "Alice") == 0, 
                 "Expected 'Alice', got '%s'", value ? value : "NULL");
        
        // Update the value
        tstcheck(kv_set(store, "name", "Bob") == 0, "Update should succeed");
        tstcheck(store->count == 1, "Count should still be 1 after update");
        value = kv_get(store, "name");
        tstcheck(strcmp(value, "Bob") == 0, "Value should be updated to 'Bob'");
        
        // Delete the value
        tstcheck(kv_delete(store, "name") == 0, "Delete should succeed");
        tstcheck(store->count == 0, "Count should be 0 after delete");
        tstcheck(kv_get(store, "name") == NULL, "Deleted key should return NULL");
        
        kv_destroy(store);
    }
    
    tstcase("Edge cases and error handling") {
        KVStore* store = kv_create(2);
        tstassert(store != NULL);
        
        // NULL pointer checks
        tstcheck(kv_set(NULL, "key", "value") == -1, "NULL store should fail");
        tstcheck(kv_set(store, NULL, "value") == -1, "NULL key should fail");
        tstcheck(kv_set(store, "key", NULL) == -1, "NULL value should fail");
        tstcheck(kv_get(NULL, "key") == NULL, "NULL store should return NULL");
        tstcheck(kv_get(store, NULL) == NULL, "NULL key should return NULL");
        
        // Capacity overflow
        tstcheck(kv_set(store, "k1", "v1") == 0);
        tstcheck(kv_set(store, "k2", "v2") == 0);
        tstcheck(kv_set(store, "k3", "v3") == -1, 
                 "Should fail when capacity exceeded");
        tstcheck(store->count == 2, "Count should be 2, got %d", store->count);
        
        // Non-existent key
        tstcheck(kv_get(store, "nonexistent") == NULL, 
                 "Non-existent key should return NULL");
        tstcheck(kv_delete(store, "nonexistent") == -1, 
                 "Deleting non-existent key should fail");
        
        kv_destroy(store);
    }
    
    // Data-driven test with multiple test cases
    tstcase("Data-driven test: Multiple key-value pairs") {
        KVStore* store = kv_create(10);
        tstassert(store != NULL);
        
        // Define test data
        struct { const char* key; const char* value; } tstdata[] = {
            {"username", "alice123"},
            {"email", "alice@example.com"},
            {"age", "25"},
            {"city", "New York"},
            {"country", "USA"}
        };
        
        tstsection("Insert and verify each pair") {
            tstnote("Testing key='%s', value='%s'", 
                    tstcurdata.key, tstcurdata.value);
            
            // Insert
            tstcheck(kv_set(store, tstcurdata.key, tstcurdata.value) == 0,
                     "Failed to insert key='%s'", tstcurdata.key);
            
            // Immediately verify
            const char* retrieved = kv_get(store, tstcurdata.key);
            tstcheck(retrieved != NULL, "Key '%s' not found", tstcurdata.key);
            tstcheck(strcmp(retrieved, tstcurdata.value) == 0,
                     "Expected '%s', got '%s'", tstcurdata.value, retrieved);
        }
        
        // After all data items processed, verify count
        tstcheck(store->count == 5, "Expected 5 items, got %d", store->count);
        
        kv_destroy(store);
    }
    
    // Performance test (tagged as +slow)
    tstcase("Performance test: Large dataset", +slow) {
        const int N = 1000;
        KVStore* store = kv_create(N);
        tstassert(store != NULL, "Failed to create large store");
        
        clock_t elapsed;
        
        tstclock("Insert %d items", N) {
            for (int i = 0; i < N; i++) {
                char key[32], value[32];
                sprintf(key, "key_%d", i);
                sprintf(value, "value_%d", i);
                tstassert(kv_set(store, key, value) == 0);
            }
        }
        elapsed = tstelapsed();
        
        tstcheck(store->count == N, "Expected %d items, got %d", N, store->count);
        tstnote("Insert rate: %.2f items/ms", (double)N / elapsed);
        
        tstclock("Retrieve %d items", N) {
            for (int i = 0; i < N; i++) {
                char key[32];
                sprintf(key, "key_%d", i);
                tstassert(kv_get(store, key) != NULL);
            }
        }
        elapsed = tstelapsed();
        tstnote("Retrieval rate: %.2f items/ms", (double)N / elapsed);
        
        kv_destroy(store);
    }
    
    // Stress test (tagged as +stress, -ci)
    tstcase("Stress test: Maximum capacity", +stress, -ci) {
        const int MAX = 10000;
        KVStore* store = NULL;
        
        // Skip if we can't allocate enough memory
        store = kv_create(MAX);
        tstskipif(store == NULL) {
            tstnote("Testing with %d capacity store", MAX);
            
            // Fill to capacity
            for (int i = 0; i < MAX; i++) {
                char key[32], value[64];
                sprintf(key, "k%d", i);
                sprintf(value, "This is a longer value for key %d", i);
                tstcheck(kv_set(store, key, value) == 0,
                         "Failed at item %d", i);
            }
            
            tstcheck(store->count == MAX, "Expected full capacity");
            
            // Verify random samples
            for (int i = 0; i < 100; i++) {
                int idx = rand() % MAX;
                char key[32];
                sprintf(key, "k%d", idx);
                tstcheck(kv_get(store, key) != NULL,
                         "Sample key %s not found", key);
            }
            
            tstnote("Successfully tested %d items", MAX);
        }
        
        if (store) kv_destroy(store);
    }
    
    // Memory leak detection (would need actual leak detector)
    tstcase("Memory management", +valgrind, -ci) {
        tstnote("Run with: valgrind --leak-check=full ./t_advanced_example +valgrind");
        
        // Create and destroy multiple stores
        for (int i = 0; i < 100; i++) {
            KVStore* store = kv_create(10);
            tstassert(store != NULL);
            
            kv_set(store, "test", "value");
            kv_get(store, "test");
            kv_delete(store, "test");
            
            kv_destroy(store);
        }
        
        tstcheck(1, "Memory operations completed");
    }
}
