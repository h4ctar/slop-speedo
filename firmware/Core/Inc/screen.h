/**
 * This Library was originally written by Olivier Van den Eede (4ilo) in 2016.
 * Some refactoring was done and SPI support was added by Aleksander Alekseev (afiskon) in 2018.
 *
 * https://github.com/afiskon/stm32-ssd1306
 */

#ifndef __SCREEN_H__
#define __SCREEN_H__

void initScreen();
void clearScreen();
void updateScreen();
void displayString(char *str);

#endif
