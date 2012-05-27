#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "network.h"

typedef struct {
  unsigned short version;
  unsigned players;
  float latency;
} il_Network_ServerInfo;

int il_Network_RegisterClientHandlers(il_Network_Connection* ptr);

int il_Network_connect(il_Network_Connection* ptr, const il_Network_String address);
int il_Network_authenticate(il_Network_Connection* ptr, const il_Network_Authcode auth);
int il_Network_probe(il_Network_Connection* ptr);

int il_Network_Handler_connectResponse(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen);
int il_Network_Handler_probeResponse(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen);

#endif
