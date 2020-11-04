#include <drivers/entropy.h>

const struct device *entropy_dev;

uint32_t random_uint32(void)
{
    uint32_t buffer = 0;
    int ret = entropy_get_entropy(entropy_dev, (uint8_t *)&buffer, 4);
    if(ret) {
        printf("error generating entropy\n");
    }
    return buffer;
}

void random_init(uint32_t seed)
{
    printf("TRNG\n");
    (void)seed;

    entropy_dev = device_get_binding(DT_CHOSEN_ZEPHYR_ENTROPY_LABEL);
    if (!entropy_dev) {
		printf("error: no random device\n");
		return;
	}
    else {
        printf("successfully initialized entropy device\n");
    }
}