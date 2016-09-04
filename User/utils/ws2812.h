#ifndef MPORK_WS2812_H
#define MPORK_WS2812_H

/* Includes ------------------------------------------------------------------*/

#include <common.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

// PB8 - WS2812B data line
#define COLORLED_GPIO GPIOC
#define COLORLED_PIN GPIO15

#define RGB_RED     rgb(255,   0,   0)
#define RGB_ORANGE  rgb(255, 110,   0)
#define RGB_YELLOW  rgb(255, 255,   0)
#define RGB_LIME    rgb(160, 255,   0)
#define RGB_GREEN   rgb(  0, 255,   0)
#define RGB_CYAN    rgb(  0, 255, 120)
#define RGB_BLUE    rgb(  0,   0, 255)
#define RGB_MAGENTA rgb(255,   0, 255)
#define RGB_WHITE   rgb(255, 255, 255)
#define RGB_BLACK   rgb(  0,   0,   0)

/* Exported macros -----------------------------------------------------------*/

/**
 * @brief Compose an RGB color.
 * @param r, g, b - components 0xFF
 * @returns integer 0xRRGGBB
 */
#define rgb(r, g, b) (((0xFF & (r)) << 16) | ((0xFF & (g)) << 8) | (0xFF & (b)))

/* Get components */
#define rgb_r(rgb) (((rgb) >> 16) & 0xFF)
#define rgb_g(rgb) (((rgb) >> 8) & 0xFF)
#define rgb_b(rgb) ((rgb) & 0xFF)

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Struct for easy manipulation of RGB colors.
 *
 * Set components in the xrgb.r (etc.) and you will get
 * the hex in xrgb.num.
 */
typedef union {

	/** Struct for access to individual color components */
	struct __attribute__((packed)) {
		uint8_t b;
		uint8_t g;
		uint8_t r;
	};

	/** RGB color as a single uint32_t */
	uint32_t num;

} ws2812_rgb_t;


/**
 * @brief Macro to compose the RGB struct.
 *
 * You can also use {.num = 0xFF0000} to set the hex directly.
 */
#define WS2812_RGB(r, g, b) {.num = (((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF)}

/**
 * @brief Send a sequence of colors, then display it.
 * @param GPIOx
 * @param pin
 * @param rgbs : array of colors
 * @param count : array length
 */
void ws2812_send(GPIO_TypeDef *GPIOx, uint16_t pin, uint32_t *rgbs, uint32_t count);

/**
 * @brief Send a single color.
 * @param GPIOx
 * @param pin
 * @param rgb : color to show
 */
void ws2812_single(GPIO_TypeDef *GPIOx, uint16_t pin, uint32_t rgb);

/**
 * @brief Wait the necessary time for the colors sent using ws2812_single() to show.
 */
void ws2812_flip();

#endif //MPORK_WS2812_H
