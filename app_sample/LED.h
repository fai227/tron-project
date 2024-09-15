#ifndef LED_H_
#define LED_H_

#include <tk/tkernel.h>


void initialize_led(UB timer_number);
void clear_led();

void turn_on_led(UW row, UW column);

void show_number(UB number);

void show_circle();
void show_cross();

void show_S();
void show_C();
void show_T();

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