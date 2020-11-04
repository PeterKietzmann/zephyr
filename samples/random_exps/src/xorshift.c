static uint32_t _state32;// = 534571505;

uint32_t xorshift32(uint32_t *state)
{
    uint32_t x = *state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;

    return x;
}

uint32_t random_uint32(void)
{
    return xorshift32(&_state32);
}

void random_init(uint32_t seed)
{
    printf("XORSHIFT\n");
    _state32 = seed;
}