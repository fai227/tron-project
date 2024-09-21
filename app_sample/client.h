#ifndef CLIENT_H_
#define CLIENT_H_

#include <tk/tkernel.h>
#include "list.h"

void client_setup();

EXPORT UINT request_departure_time_ms();

EXPORT void reserve_order(List* order_list, UB delay_until_departure);
//LOCAL void receive_interrupt_handler(UINT interrupt_number);
//LOCAL void transfer_task(INT stacd, void *exinf);



#endif /* CLIENT_H_ */