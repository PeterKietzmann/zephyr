/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include "stdio.h"

static uint32_t _state32 = 534571505;

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

void main(void)
{
	printf("prng_xorshift\n");
	int i=0;
	while(i<10) {
		printf("0x%x\n", random_uint32());
		i++;
	}
}
	

