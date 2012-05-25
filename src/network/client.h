#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "network.h"

int il_Network_connect(const il_Network_String address, il_Network_Connection* ptr);

int il_Netowrk_authenticate(il_Network_Connection* ptr, il_Network_Authcode auth);

typedef struct {
  unsigned short version;
  unsigned players;
  float latency;
} il_Network_ServerInfo;

int il_Network_probe(il_Network_Connection* ptr, il_Network_ServerInfo* info);

int il_Network_disconnect(il_Network_Connection* ptr);

#endif
