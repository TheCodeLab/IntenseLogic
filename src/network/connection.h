#ifndef IL_NETWORK_CONNECTION_H
#define IL_NETWORK_CONNECTION_H

#include <stdlib.h>

typedef struct il_Network_Connection il_Network_Connection;

il_Network_Connection * il_Network_Connection_new();

const char * il_Network_Connection_getError(il_Network_Connection *);

int il_Network_Connection_write(il_Network_Connection *, void * data, size_t n);

#endif
