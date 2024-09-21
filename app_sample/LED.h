#ifndef LED_H_
#define LED_H_

#include <tk/tkernel.h>

EXPORT void initialize_led(UB timer_number);
EXPORT void clear_led();

EXPORT void turn_on_led(UW row, UW column);

EXPORT void show_number(UB number);

EXPORT void show_circle();
EXPORT void show_cross();

EXPORT void show_S();
EXPORT void show_V();
EXPORT void show_T();

EXPORT void show_logo();

EXPORT void show_strait();
EXPORT void show_left();
EXPORT void show_right();
EXPORT void show_stop();

#define COL1 28
#define COL2 11
#define COL3 31
#define COL4 05
#define COL5 30
#define ROW1 21
#define ROW2 22
#define ROW3 15
#define ROW4 24
#define ROW5 19

#endif /* LED_H_ */