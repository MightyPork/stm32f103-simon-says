//
// Created by MightyPork on 2016/9/4
//

#include <common.h>
#include "utils/ws2812.h"
#include "utils/timebase.h"
#include "utils/debug.h"
#include "handlers.h"

/**
 * Increment timebase counter each ms.
 * This is called by HAL, weak override.
 */
void HAL_SYSTICK_Callback(void)
{
	timebase_ms_cb();
}

/** Called from MX-generated HAL error handler */
void user_Error_Handler()
{
	error("HAL error occurred.\n");
	while (1);
}

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void user_assert_failed(uint8_t *file, uint32_t line)
{
	user_error_file_line("Assert failed", (const char *) file, line);
}

void user_error_file_line(const char *message, const char *file, uint32_t line)
{
	error("%s in file %s on line %d", message, file, line);
	while (1);
}
