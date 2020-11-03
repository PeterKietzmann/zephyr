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

#define GPIO_PORT DT_LABEL(DT_NODELABEL(gpiob))
#define GPIO_PORT_MUX DT_LABEL(DT_NODELABEL(portb))
#define GPIO_PIN 29

#define EXP_DURATION 10 // seconds

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




void my_timer_handler(struct k_timer *dummy)
{
	running = 0;
	k_timer_stop(&my_timer);
	printf("rand_integers: %i in %i seconds\n", rand_integers, EXP_DURATION);
	rand_integers=0;
}

void main(void)
{
	printf("prng_xorshift\n");

	bool led_is_on = true;

	init_gpio();
	
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
}
	

