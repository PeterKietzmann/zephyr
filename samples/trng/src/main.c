/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include "stdio.h"

#include <drivers/entropy.h>

void main(void)
{
    printf("trng\n");
    uint8_t buffer=0;
    const struct device *dev;
    dev = device_get_binding(DT_CHOSEN_ZEPHYR_ENTROPY_LABEL);
    if (!dev) {
		printf("error: no random device\n");
		return;
	}
    else {
        printf("successfully initialized entropy device\n");
    }

	int i=0;
	while(i<10) {
        int ret =entropy_get_entropy(dev, &buffer, 1);
        if(ret) {
            printf("error generating entropy\n");
        }
		printf("0x%x\n",buffer);
		i++;
	}
}
	

