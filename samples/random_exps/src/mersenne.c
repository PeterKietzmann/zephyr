/** Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /** constant vector a */
#define UPPER_MASK 0x80000000UL /** most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /** least significant r bits */

#define MTI_UNINITIALIZED (N + 1)

static uint32_t mt[N]; /** the array for the state vector  */
static uint16_t mti = MTI_UNINITIALIZED;

void random_init(uint32_t s)
{
    printf("Mersenne\n");
    mt[0] = s;
    for (int i = 1; i < N; ++i) {
        mt[i] = 1812433253UL * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i;
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
    }

    mti = N;
}

/** generates N words at one time */
static void generate_numbers(void)
{
    if (mti == MTI_UNINITIALIZED) {
        /* if init_genrand() has not been called, a default initial seed is used */
        random_init(5489UL);
    }

    for (int k = 0; k < N; ++k) {
        uint32_t y = (mt[k] & UPPER_MASK) | (mt[(k + 1) % N] & LOWER_MASK);
        mt[k] = mt[(k + M) % N] ^ (y >> 1);
        if (y & 1) {
            mt[k] ^= MATRIX_A;
        }
    }

    mti = 0;
}

uint32_t random_uint32(void)
{
    if (mti >= N) {
        generate_numbers();
    }

    uint32_t y = mt[mti++];
    y ^= y >> 11;
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= y >> 18;
    return y;
}