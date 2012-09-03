#include "connection.h"

#include <event2/event.h>
#include <event2/bufferevent.h>

struct il_Network_Connection {
  struct event_base * base;
  struct bufferevent * bufferevent;
  const char *error;
  enum {
    UNINITIALISED = 0,
    CREATED,
    CONNECTING,
    CONNECTED,
    FAILED
  } state;
};

il_Network_Connection * il_Network_Connection_new() {
  il_Network_Connection * con = calloc(sizeof(il_Network_Connection), 1);
  
  con->base = event_base_new();
  con->bufferevent = bufferevent_socket_new(con->base, -1, BEV_OPT_CLOSE_ON_FREE);
  con->state = CREATED;
  
  return con;
}

const char * il_Network_Connection_getError(il_Network_Connection * con) {
  if (con->state != FAILED) return NULL; // no error
  const char *err = con->error;
  con->error = NULL;
  return err;
}
