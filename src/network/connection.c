#include "connection.h"

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/tag.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/dns.h>
#include <event2/util.h>

extern int asprintf(char **str, const char *fmt, ...);

#include "common/base.h"

extern int evtag_decode_int(ev_uint32_t *pnumber, struct evbuffer *evbuf);

struct il_Network_Connection {
  struct event_base * base;
  enum {
    NONE,
    CLIENT,
    SERVER
  } type;
  struct bufferevent * bufferevent;
  struct evconnlistener * listener;
  struct client {
    il_Network_Connection * con;
    struct client* next;
  } *clients;
  const char *error;
  enum {
    CREATED,
    CONNECTING,
    CONNECTED,
    CLOSED,
    FAILED
  } state;
};

#define TEST_ERROR(con, cmp, err, ...) \
  if (cmp) { \
    char *str; \
    asprintf(&str, (err), ##__VA_ARGS__); \
    (con)->error = str;\
    (con)->state = FAILED;\
    return -1;\
  }

il_Network_Connection * il_Network_Connection_new() {
  il_Network_Connection * con = calloc(sizeof(il_Network_Connection), 1);
  if (!con) return NULL;
  
  con->type = NONE;
  con->base = event_base_new();
  con->bufferevent = NULL;
  con->listener = NULL;
  con->state = CREATED;
  con->clients = NULL;
  
  return con;
}

int il_Network_Connection_disconnect(il_Network_Connection * con, il_Common_String reason) {
  TEST_ERROR(con, con->state != CONNECTED, "Not connected");
  int res;
  res = il_Network_Connection_write(con, IL_BASE_SHUTDOWN, reason.data, reason.length);
  TEST_ERROR(con, res != 0, "Failed to write shutdown event");
  con->state = CLOSED;
  bufferevent_free(con->bufferevent);
  event_base_free(con->base);
  return 0;
}

const char * il_Network_Connection_getError(il_Network_Connection * con) {
  if (con->state != FAILED) return NULL; // no error
  const char *err = con->error;
  con->error = NULL;
  return err;
}

int il_Network_Connection_write(il_Network_Connection * con, ev_uint32_t tag, const void * data, size_t n) {
  TEST_ERROR(con, con->state != CONNECTED, "Not connected.");
  int res;
  struct evbuffer * buf = evbuffer_new();
  TEST_ERROR(con, buf == NULL, "Couldn't allocate new evbuffer");
  evtag_marshal(buf, tag, data, n);
  res = bufferevent_write_buffer(con->bufferevent, buf);
  TEST_ERROR(con, res != 0, "Couldn't write to connection");
  evbuffer_free(buf);
  return 0;
}

int il_Network_Connection_write_buffer(il_Network_Connection * con, ev_uint32_t tag, struct evbuffer * in) {
  TEST_ERROR(con, con->state != CONNECTED, "Not connected");
  int res;
  struct evbuffer * buf = evbuffer_new();
  TEST_ERROR(con, buf == NULL, "Couldn't allocate new evbuffer");
  evtag_marshal_buffer(buf, tag, in);
  res = bufferevent_write_buffer(con->bufferevent, buf);
  TEST_ERROR(con, res != 0, "Couldn't write to connection");
  evbuffer_free(buf);
  return 0;
}

int il_Network_Connection_read(il_Network_Connection * con, ev_uint32_t * ptag, void ** data) {
  TEST_ERROR(con, con->state != CONNECTED, "Not connected");
  struct evbuffer * buf = evbuffer_new();
  TEST_ERROR(con, buf == NULL, "Couldn't allocate new evbuffer");
  int res = il_Network_Connection_read_buffer(con, ptag, buf);
  TEST_ERROR(con, res != 0, "Couldn't read from connection");
  size_t len = evbuffer_get_length(buf);
  *data = calloc(1, len);
  TEST_ERROR(con, *data == NULL, "Couldn't allocate %u byte buffer", len);
  evbuffer_copyout(buf, *data, len);
  evbuffer_free(buf);
  return len;
}

int il_Network_Connection_read_buffer(il_Network_Connection * con, ev_uint32_t *ptag, struct evbuffer * data) {
  TEST_ERROR(con, con->state != CONNECTED, "Not connected");
  int res;
  struct evbuffer * src = evbuffer_new();
  TEST_ERROR(con, src == NULL, "Failed to allocate new evbuffer");
  res = bufferevent_read_buffer(con->bufferevent, src);
  TEST_ERROR(con, res != 0, "Failed to read bufferevent");
  ev_uint32_t tag;
  evtag_unmarshal(src, ptag?ptag:&tag, data);
  evbuffer_free(src);
  return 0;
}

int il_Network_Connection_connect(il_Network_Connection * con, const char * host, unsigned short port) {
  con->type = CLIENT;
  con->state = CONNECTING;
  con->bufferevent = bufferevent_socket_new(con->base, -1, BEV_OPT_CLOSE_ON_FREE);
  int res = bufferevent_socket_connect_hostname(con->bufferevent, NULL, AF_UNSPEC, host, port);
  TEST_ERROR(con, res == 0, "Unable to connect to %s:%u", host, (unsigned)port);
  con->state = CONNECTED;
  return 0;
}

int il_Network_Connection_setSocket(il_Network_Connection * con, evutil_socket_t fd, int flags) {
  con->type = CLIENT;
  con->state = CONNECTING;
  con->bufferevent = bufferevent_socket_new(con->base, fd, flags);
  con->state = CONNECTED;
  return 0;
}

void server_accept(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * addr, int socklen, void * ptr) {
  il_Network_Connection * con = il_Network_Connection_new();
  if (!con) return;
  il_Network_Connection_setSocket(con, fd, BEV_OPT_CLOSE_ON_FREE);
}

int il_Network_Connection_bind(il_Network_Connection * con, const char * host, unsigned short port) {
  int res;
  
  con->type = SERVER;
  con->state = CONNECTING;
  
  char port_str[6];
  snprintf(port_str, sizeof(port_str), "%d", (int)port);
  
  struct evutil_addrinfo hints;
  struct evutil_addrinfo *answer = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; /* v4 or v6 is fine. */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP; /* We want a TCP socket */
  /* Only return addresses we can use. */
  hints.ai_flags = EVUTIL_AI_ADDRCONFIG;
  
  res = evutil_getaddrinfo(host, port_str, &hints, &answer);
  TEST_ERROR(con, res != 0 || !answer, "Unable to resolve host %s:%s", host, port_str);
  
  con->listener = evconnlistener_new_bind(con->base, &server_accept, con, LEV_OPT_CLOSE_ON_FREE, -1, answer->ai_addr, answer->ai_addrlen);
  
  con->state = CONNECTED;
  return 0;
}
