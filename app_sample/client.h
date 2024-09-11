#ifndef CLIENT_H_
#define CLIENT_H_

#include <tk/tkernel.h>

UINT request_departure_time_ms();

void reserve_order(List *order_list, UB delay_until_departure);

#endif /* CLIENT_H_ */