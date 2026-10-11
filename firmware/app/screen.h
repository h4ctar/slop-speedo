/**
 * This Library was originally written by Olivier Van den Eede (4ilo) in 2016.
 * Some refactoring was done and SPI support was added by Aleksander Alekseev (afiskon) in 2018.
 *
 * https://github.com/afiskon/stm32-ssd1306
 */

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "stm32f1xx_hal.h"

void screen_init();
void screen_clear();
HAL_StatusTypeDef screen_update();
void screen_draw_string(char *str);
void screen_show_string(const char *str);

#endif
