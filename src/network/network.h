#ifndef NETWORK_H
#define NETWORK_H

#define IL_NETWORK_VERSION 1

typedef struct {
  unsigned char id;
} il_Network_Packet;

typedef struct {
  unsigned short version;
  int sock_fd;
  int (*handlers[256])(il_Network_Packet*);
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
