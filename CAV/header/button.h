#ifndef BUTTON_H_
#define BUTTON_H_

#include <tk/tkernel.h>

#define BUTTON_A 14
#define BUTTON_B 23

EXPORT void button_setup()
{
    out_w(GPIO(P0, PIN_CNF(BUTTON_A)), 0);
    out_w(GPIO(P0, PIN_CNF(BUTTON_B)), 0);
}

EXPORT BOOL is_a_pressed()
{
    return (in_w(GPIO(P0, IN)) & (1 << BUTTON_A)) == 0;
}

EXPORT BOOL is_b_pressed()
{
    return (in_w(GPIO(P0, IN)) & (1 << BUTTON_B)) == 0;
}

#endif /* BUTTON_H_ */