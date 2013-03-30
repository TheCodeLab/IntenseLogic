#ifndef IL_NETWORK_CONNECTION_H
#define IL_NETWORK_CONNECTION_H

#include <stdlib.h>
#include <event2/util.h>

struct evbuffer;

typedef struct ilN_connection ilN_connection;

ilN_connection * ilN_connection_new();

int ilN_connection_connect(ilN_connection *, const char * host, unsigned short port);
int ilN_connection_setSocket(ilN_connection * con, evutil_socket_t fd, int flags);
int ilN_connection_bind(ilN_connection *, const char * host, unsigned short port);
int ilN_connection_disconnect(ilN_connection *, const char *reason);

const char * ilN_connection_getError(ilN_connection *);

int ilN_connection_write(ilN_connection * con, ev_uint32_t tag, const void * data, size_t n);
int ilN_connection_write_buffer(ilN_connection * con, ev_uint32_t tag, struct evbuffer * in);

int ilN_connection_read(ilN_connection * con, ev_uint32_t * ptag, void ** data);
int ilN_connection_read_buffer(ilN_connection * con, ev_uint32_t *ptag, struct evbuffer * data);

#endif
