static uint64_t _state64;

uint32_t musl_lcg(void)
{
    _state64 = 6364136223846793005ULL*_state64 + 1;
    return _state64>>32;
}

uint32_t random_uint32(void)
{
    return musl_lcg();
}

void random_init(uint32_t s)
{
    printf("Musl LCG\n");
    _state64 = s-1;
}
