// Author   : Ebod Shojaei
// Updated  : 24-05-2025

// MIT License 2025

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>     // For uint64_t
#include <stddef.h>     // For size_t
#include <primesieve.h> // For primes

// Data structure for hashtable entries
typedef struct node
{
    unsigned long long value;
    struct node *next;
} node_t;

// Data structure for hashtable
typedef struct hash_table
{
    size_t size;
    size_t count;
    node_t **buckets;
} hash_table_t;

// Helper function to use the prime as a hash key
size_t hash_ull(unsigned long long key, size_t table_size)
{
    key = (~key) + (key << 21);             // key = (key << 21) - key - 1;
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8);  // key * 265
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4);  // key * 21
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key % table_size;
}

// Helper function to create a new hashtable
hash_table_t *hash_table_create(size_t table_size)
{
    hash_table_t *ht = (hash_table_t *)malloc(sizeof(hash_table_t));
    if (ht == NULL)
    {
        perror("Failed to allocate hash table");
        return NULL;
    }
    ht->size = table_size;
    ht->count = 0;
    ht->buckets = (node_t **)calloc(table_size, sizeof(node_t *));
    if (ht->buckets == NULL)
    {
        perror("Failed to allocate hash table buckets");
        free(ht);
        return NULL;
    }
    return ht;
}

// Helper function to add a new entry to the hashtable
int hash_table_add(hash_table_t *ht, unsigned long long value)
{
    if (ht == NULL)
        return 0;
    size_t index = hash_ull(value, ht->size);
    node_t *current = ht->buckets[index];
    while (current != NULL)
    {
        if (current->value == value)
        {
            return 1;
        }
        current = current->next;
    }
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (new_node == NULL)
    {
        perror("Failed to allocate hash table node");
        return 0;
    }
    new_node->value = value;
    new_node->next = ht->buckets[index];
    ht->buckets[index] = new_node;
    ht->count++;
    return 1;
}

// Helper function to check if entry exists in hashtable
int hash_table_contains(hash_table_t *ht, unsigned long long value)
{
    if (ht == NULL)
        return 0;
    size_t index = hash_ull(value, ht->size);
    node_t *current = ht->buckets[index];
    while (current != NULL)
    {
        if (current->value == value)
        {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

// Helper function to clean-up hashtable from memory
void hash_table_destroy(hash_table_t *ht)
{
    if (ht == NULL)
        return;
    for (size_t i = 0; i < ht->size; ++i)
    {
        node_t *current = ht->buckets[i];
        while (current != NULL)
        {
            node_t *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht->buckets);
    free(ht);
}

// Utility function to calculate START/END indices for prime generator
unsigned long long power_of_10(int exp)
{
    unsigned long long res = 1;
    for (int i = 0; i < exp; ++i)
    {
        res *= 10;
    }
    return res;
}

// Utility function to count the number of digits in a number
int count_digits(unsigned long long num)
{
    if (num == 0)
        return 1;
    int count = 0;
    unsigned long long temp = num;
    while (temp > 0)
    {
        temp /= 10;
        count++;
    }
    return count;
}

// Composite function to calculate total number of right-truncatable primes
int count_right_trunc_primes(int digits)
{
    if (digits < 1 || digits > 19)
    {
        fprintf(stderr, "Error: digits must be between 1 and 19 for unsigned long long.\n");
        return -1;
    }

    // 1. Generate ALL primes up to the max number of digits specified
    //    This ensures the hash table contains all possible truncated parts.
    unsigned long long MAX_END = power_of_10(digits) - 1; // e.g., for digits=3, MAX_END=999
    unsigned long long MIN_START = 2;                     // Always start from 2 for primes

    size_t all_primes_count;
    unsigned long long *all_primes_array = (unsigned long long *)primesieve_generate_primes(MIN_START, MAX_END, &all_primes_count, ULONGLONG_PRIMES);

    if (all_primes_array == NULL)
    {
        fprintf(stderr, "Error: primesieve_generate_primes for all primes returned NULL.\n");
        return -1;
    }

    // 2. Populate the hash table with ALL generated primes
    size_t initial_hash_table_size = (all_primes_count == 0) ? 17 : (all_primes_count * 2);
    hash_table_t *prime_hash_set = hash_table_create(initial_hash_table_size);
    if (prime_hash_set == NULL)
    {
        fprintf(stderr, "Error creating hash set.\n");
        primesieve_free(all_primes_array);
        return -1;
    }

    for (size_t i = 0; i < all_primes_count; ++i)
    {
        if (!hash_table_add(prime_hash_set, all_primes_array[i]))
        {
            fprintf(stderr, "Error adding prime to hash set: %llu\n", all_primes_array[i]);
            hash_table_destroy(prime_hash_set);
            primesieve_free(all_primes_array);
            return -1;
        }
    }

    // 3. Iterate through primes of the specified "digits" length
    //    These are the candidates to test for right-truncatable property.
    int right_truncatable_count = 0;
    unsigned long long current_digits_start = power_of_10(digits - 1); // e.g., 100 for digits=3
    unsigned long long current_digits_end = power_of_10(digits) - 1;   // e.g., 999 for digits=3

    for (size_t i = 0; i < all_primes_count; ++i)
    {
        unsigned long long current_prime = all_primes_array[i];

        // Skip primes outside current "digits" length window
        if (current_prime < current_digits_start || current_prime > current_digits_end)
        {
            continue;
        }

        int is_r_truncatable = 1; // Assume right-truncatable until proven otherwise
        unsigned long long temp_prime = current_prime;

        while (temp_prime > 0) // Loop until the number becomes 0 (all digits removed)
        {
            // Check if the current truncated number is prime
            if (!hash_table_contains(prime_hash_set, temp_prime))
            {
                is_r_truncatable = 0; // Not a right-truncatable prime
                break;
            }

            temp_prime /= 10; // Remove the rightmost digit
        }

        if (is_r_truncatable)
        {
            // printf("%lld\n", current_prime); // NOTE: uncomment to see primes
            right_truncatable_count++;
        }
    }

    // 4. Clean up allocated memory
    hash_table_destroy(prime_hash_set);
    primesieve_free(all_primes_array);

    return right_truncatable_count;
}

// Driver function to run the program
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <number_of_digits>\n", argv[0]);
        return 1;
    }
    // NOTE: The maximum number of digits is limited by the size of unsigned long long.
    // The maximum value for unsigned long long is 2^64 - 1, which has 20 digits.
    // However, the maximum number of digits this program handles is 19, to leave room for the left-truncation process.
    // Ignore for right-truncatable primes.
    // The largest right-truncatable primes is 8-digit long: 73939133 (sequence A024770 in the OEIS)
    
    int digits = atoi(argv[1]);
    if (digits < 1 || digits > 19)
    {
        fprintf(stderr, "Error: digits must be between 1 and 19 for unsigned long long.\n");
        return 1;
    }

    // Generating all primes up to >10^9 requires a massive amount of memory.
    // primesieve is highly optimized, but there is a physical limit to available RAM on the system.
    // Even storing the primes themselves takes up a lot of space, let alone the internal structures primesieve uses during generation.

    int result = count_right_trunc_primes(digits);
    if (result < 0)
    {
        fprintf(stderr, "Error counting right-truncatable primes.\n");
        return 1;
    }
    printf("Number of %d-digit right-truncatable primes: %d\n", digits, result);

    // Calculate total number of right-truncatable primes up to the specified number of digits
    unsigned long long total_count = 0;
    for (int i = 1; i <= digits; ++i)
    {
        int count = count_right_trunc_primes(i);
        if (count < 0)
        {
            fprintf(stderr, "Error counting right-truncatable primes for %d digits.\n", i);
            return 1;
        }
        total_count += count;
    }
    printf("Total number of right-truncatable primes up to %d digits: %llu\n", digits, total_count);
    // NOTE: The total number of right-truncatable primes is not the same as the number of primes.
    // The total number of right-truncatable primes is the sum of all right-truncatable primes

    return 0;
}
