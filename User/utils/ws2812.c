#include "ws2812.h"
#include "utils/timebase.h"

#define LONG_DELAY() for (volatile uint32_t __j = 4; __j > 0; __j--)
#define SHORT_DELAY() for (volatile uint32_t __j = 1; __j > 0; __j--)

static inline
__attribute__((always_inline))
void ws2812_byte(GPIO_TypeDef *GPIOx, uint16_t pin, uint8_t b)
{
	for (register volatile uint8_t i = 0; i < 8; i++) {
		GPIOx->BSRR = pin; // set pin high

		// duty cycle determines bit value
		if (b & 0x80) {
			LONG_DELAY();
			GPIOx->BRR = pin; // set pin low
			SHORT_DELAY();
		} else {
			SHORT_DELAY();
			GPIOx->BRR = pin; // set pin low
			LONG_DELAY();
		}

		b <<= 1; // shift to next bit
	}
}

/** Set many RGBs */
void ws2812_send(GPIO_TypeDef *GPIOx, uint16_t pin, uint32_t *rgbs, uint32_t count)
{
	__disable_irq();

	for (int i = 0; i < count; i++) {
		uint32_t rgb = *rgbs++;
		ws2812_byte(GPIOx, pin, rgb_g(rgb));
		ws2812_byte(GPIOx, pin, rgb_r(rgb));
		ws2812_byte(GPIOx, pin, rgb_b(rgb));
	}

	__enable_irq();

	ws2812_flip(); // show
}

/** Send a single color. */
void ws2812_single(GPIO_TypeDef *GPIOx, uint16_t pin, uint32_t rgb)
{
	__disable_irq();

	ws2812_byte(GPIOx, pin, rgb_g(rgb));
	ws2812_byte(GPIOx, pin, rgb_r(rgb));
	ws2812_byte(GPIOx, pin, rgb_b(rgb));

	__enable_irq();
}

/**
 * @brief Wait the necessary time for the colors sent using ws2812_single() to show.
 */
void ws2812_flip()
{
	delay_us(50); // show
}
