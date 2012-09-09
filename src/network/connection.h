#ifndef IL_NETWORK_CONNECTION_H
#define IL_NETWORK_CONNECTION_H

#include <stdlib.h>
#include <event2/util.h>

struct evbuffer;

#include "common/string.h"

typedef struct il_Network_Connection il_Network_Connection;

il_Network_Connection * il_Network_Connection_new();

int il_Network_Connection_connect(il_Network_Connection *, const char * host, unsigned short port);
int il_Network_Connection_setSocket(il_Network_Connection * con, evutil_socket_t fd, int flags);
int il_Network_Connection_bind(il_Network_Connection *, const char * host, unsigned short port);
int il_Network_Connection_disconnect(il_Network_Connection *, il_Common_String reason);

const char * il_Network_Connection_getError(il_Network_Connection *);

int il_Network_Connection_write(il_Network_Connection * con, ev_uint32_t tag, const void * data, size_t n);
int il_Network_Connection_write_buffer(il_Network_Connection * con, ev_uint32_t tag, struct evbuffer * in);

int il_Network_Connection_read(il_Network_Connection * con, ev_uint32_t * ptag, void ** data);
int il_Network_Connection_read_buffer(il_Network_Connection * con, ev_uint32_t *ptag, struct evbuffer * data);

#endif
