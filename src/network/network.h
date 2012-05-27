#ifndef NETWORK_H
#define NETWORK_H

#define IL_NETWORK_VERSION 1

typedef struct {
  unsigned length;
  char* data;
} il_Network_String;

il_Network_String il_Network_Error;

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
  int is_ready;
  float latency;
  int (*handlers[256])(struct il_Network_Connection_t *, const il_Network_Packet *, size_t);
  int (*extrahandlers[256])(struct il_Network_Connection_t *, const il_Network_PacketExtra *, size_t);
} il_Network_Connection;

typedef int (*il_Network_Callback)(il_Network_Connection *, const il_Network_Packet *, size_t);


typedef struct {
  il_Network_String authcode, key;
  long long timestamp;
} il_Network_Authcode;

struct {
  il_Network_Callback *data;
  unsigned int length;
} callbacks[256];

int il_Network_RegisterCallback(unsigned char id, il_Network_Callback callback);
int il_Network_UnregisterCallback(unsigned char id, int num);

int il_Network_disconnect(il_Network_Connection* ptr, il_Network_String reason);
int il_Network_ping(il_Network_Connection* ptr, long long timestamp, unsigned short msec);
int il_Network_chat(il_Network_Connection* ptr, const il_Network_String message);

int il_Network_Handler_disconnect(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen);
int il_Network_Handler_ping(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen);
int il_Network_Handler_pong(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen);
int il_Network_Handler_chat(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen);

int il_Network_update(il_Network_Connection* ptr);

#endif
