/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include "stdio.h"

// seed used in RIOT experiments
static uint64_t _seed = 534571505;
static uint64_t _state64 = 534571505;

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
    _state64 = s-1;
}

void main(void)
{
	printf("prng_musl_lcg\n");
    random_init(_seed);
	int i=0;
	while(i<10) {
		printf("0x%x\n", random_uint32());
		i++;
	}
}
	

