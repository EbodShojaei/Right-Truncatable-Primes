#include <primesieve.h>
#include <inttypes.h>
#include <stdio.h>

int main()
{
    primesieve_iterator it;
    primesieve_init(&it);
    uint64_t prime;

    /* Iterate over the primes < 10^6 */
    while ((prime = primesieve_next_prime(&it)) < 1000000)
        printf("%" PRIu64 "\n", prime);

    primesieve_free_iterator(&it);
    return 0;
}
