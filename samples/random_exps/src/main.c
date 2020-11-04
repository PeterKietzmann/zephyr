/*
 * Copyright (c) 2020 HAW Hamburg
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/pinmux.h>
#include <fsl_port.h>

#if CONFIG_XORSHIFT
#include "xorshift.c"
#elif CONFIG_MINSTD
#include "minstd.c"
#elif CONFIG_MUSLLCG
#include "musllcg.c" 
#elif CONFIG_TINYMT32
#include "tinymt32.c"
#elif CONFIG_MERSENNE
#include "mersenne.c" 
#elif CONFIG_TRNG
#include "trng.c"
#endif

#define GPIO_PORT DT_LABEL(DT_NODELABEL(gpiob))
#define GPIO_PORT_MUX DT_LABEL(DT_NODELABEL(portb))
#define GPIO_PIN 29

#define EXP_DURATION 10 // seconds

#define NUM_MEAS 50

uint32_t seed = 534571505;

struct device *dev;
unsigned running;
unsigned rand_integers=0;

void my_timer_handler(struct k_timer *);
K_TIMER_DEFINE(my_timer, my_timer_handler, NULL);

static inline void init_gpio(void)
{
	pinmux_pin_set(device_get_binding(GPIO_PORT_MUX), GPIO_PIN, PORT_PCR_MUX(kPORT_MuxAsGpio));
	dev = device_get_binding(GPIO_PORT);

	if (dev == NULL) {
		printf("device_get_binding FAILED\n");
		return;
	}

	int ret = gpio_pin_configure(dev, GPIO_PIN, GPIO_OUTPUT);
	if (ret < 0) {
		printf("gpio_pin_configure FAILED\n");
		return;
	}
}

void my_timer_handler(struct k_timer *dummy)
{
	running = 0;
	// k_timer_stop(&my_timer);
	// printf("rand_integers: %i in %i seconds\n", rand_integers, EXP_DURATION);
	// rand_integers=0;
	// if(num_meas < NUM_MEAS) {
	// 	k_timer_start(&my_timer, K_SECONDS(EXP_DURATION), K_SECONDS(1));
	// }
}

void main(void)
{
	printf("RNG\n");
	random_init(seed);

	// bool led_is_on = true;
	init_gpio();
	
	printf("EXP_DURATION [s]: %i\n", EXP_DURATION);
	for (int i = 0; i < NUM_MEAS; i++)
	{
		rand_integers = 0;
		running = 1;
		k_timer_start(&my_timer, K_SECONDS(EXP_DURATION), K_SECONDS(1));
		while(running) {
			// gpio_pin_set(dev, GPIO_PIN, (int)led_is_on);	
			// led_is_on = !led_is_on;
			// printf("0x%x\n", random_uint32());
			random_uint32();
			rand_integers++;
		}
		k_timer_stop(&my_timer);
		printf("%3i/%i: rand_integers: %i\n", i, NUM_MEAS, rand_integers);
	}
}
	

