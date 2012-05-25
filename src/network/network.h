#ifndef NETWORK_H
#define NETWORK_H

#define IL_NETWORK_VERSION 1

typedef struct {
  unsigned length;
  char* data;
} il_Network_String;

struct {
  char* data;
  unsigned length;
} il_Network_Error;

typedef struct {
  unsigned char id;
} il_Network_Packet;

typedef struct {
  il_Network_Packet packet;
  unsigned char id;
} il_Network_PacketExtra;

typedef struct il_Network_Connection_t {
  unsigned short version;
  int sock_fd;
  int (*handlers[256])(struct il_Connection_t *, il_Network_Packet *, size_t);
  int (*extrahandlers[256])(struct il_Connection_t *, il_Network_PacketExtra *, size_t);
} il_Network_Connection;

int il_Network_ping(il_Network_Connection* ptr);


typedef struct {
  struct {
    unsigned length;
    char* data;
  } authcode, key;
  long long timestamp;
} il_Network_Authcode;

#endif
