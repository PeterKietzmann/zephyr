
#define a 48271
#define m 2147483647
#define q (m / a) /* 44488 */
#define r (m % a) /* 3399 */

static uint32_t _state32;// = 534571505;


static inline uint32_t div_u32_by_44488(uint32_t val)
{
    return ((uint64_t)val * 0xBC8F1391UL) >> (15 + 32);
}

static inline uint32_t div_u32_mod_44488(uint32_t val)
{
    return val - (div_u32_by_44488(val)*44488);
}

int rand_minstd(void)
{
    uint32_t hi = div_u32_by_44488(_state32);
    uint32_t lo = div_u32_mod_44488(_state32);
    int32_t test = (a * lo) - (r * hi);

    if(test > 0) {
        _state32 = test;
    }
    else {
        _state32 = test + m;
    }

    return _state32;
}

uint32_t random_uint32(void)
{
    /* minstd as implemented returns only values from 1 to 2147483647,
     * so run it two times to get 32bits */
    uint16_t A = (rand_minstd() >> 15);
    uint16_t B = (rand_minstd() >> 15);
    return  (((uint32_t)A) << 16) | B;
}

void random_init(uint32_t seed)
{
    printf("MINSTD\n");
    _state32 = seed;
}