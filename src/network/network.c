#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#include "network.h"

#define set_error(E) il_Network_Error.data = (char*)realloc(il_Network_Error.data, strlen(E));\
                     strcpy(il_Network_Error.data, E);\
                     il_Network_Error.length = strlen(E);

                     
int il_Network_RegisterCallback(unsigned char id, il_Network_Callback callback) {
  il_Network_Callback *newp = (il_Network_Callback*)malloc(callbacks[id].length + 1);
  memcpy(newp, callbacks[id].data, callbacks[id].length);
  newp[callbacks[id].length] = callback;
  free(callbacks[id].data);
  callbacks[id].data = newp;
  callbacks[id].length++;
  
  return callbacks[id].length-1;
}

int il_Network_UnregisterCallback(unsigned char id, int num) {
  callbacks[id].data[num] = (il_Network_Callback)NULL;
  
  return 0;
}


int il_Network_disconnect(il_Network_Connection* ptr, il_Network_String reason) {

  struct disconnect_packet_t {
    unsigned char id;
    unsigned short reasonl;
    char reason[];
  } *disconnect_packet = (struct disconnect_packet_t*)malloc(sizeof(struct disconnect_packet_t) + reason.length);
  
  disconnect_packet->id = 0x01;
  disconnect_packet->reasonl = reason.length;
  strncpy((char*)&disconnect_packet->reason, reason.data, reason.length);
  
  if (write(ptr->sock_fd, disconnect_packet, sizeof(struct disconnect_packet_t) + reason.length) == -1) {
    set_error(strerror(errno));
    return -1;
  }
  
  if (close(ptr->sock_fd) == -1) {
    set_error(strerror(errno));
    return -1;
  }

  return 0;
}

int il_Network_ping(il_Network_Connection* ptr, long long timestamp) {

  struct ping_packet_t {
    unsigned char id;
    long long timestamp;
  } ping_packet;
  
  ping_packet.id = 0x07;
  ping_packet.timestamp = timestamp;
  
  if (write(ptr->sock_fd, &ping_packet, sizeof(struct ping_packet_t)) != -1) {
    set_error(strerror(errno));
    return -1;
  }

  return 0;
}

int il_Network_chat(il_Network_Connection* ptr, const il_Network_String message) {

  struct chat_packet_t {
    unsigned char id;
    unsigned short chatl;
    char chat[];
  } *chat_packet = (struct chat_packet_t*)malloc(sizeof(struct chat_packet_t) + message.length);
  
  chat_packet->id = 0x09;
  chat_packet->chatl = message.length;
  strncpy((char*)&chat_packet->chat, message.data, message.length);
  
  if (write(ptr->sock_fd, &chat_packet, sizeof(struct chat_packet_t)) != -1) {
    set_error(strerror(errno));
    return -1;
  }

  return 0;
}

int il_Network_Handler_disconnect(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen) {

  struct disconnect_packet_t {
    unsigned char id;
    unsigned short reasonl;
    char reason[];
  } *disconnect_packet = (struct disconnect_packet_t*) packet;
  
  size_t len = sizeof(struct disconnect_packet_t) + disconnect_packet->reasonl;
  
  int i;
  for (i=0; i < callbacks[0x01].length; i++) {
    if (callbacks[0x01].data[i]) {
      callbacks[0x01].data[i](ptr, packet, len);
    }
  }

  return 0;
}

int il_Network_Handler_ping(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen) {
  
  struct ping_packet_t {
    unsigned char id;
    long long timestamp;
  } *ping_packet = (struct ping_packet_t*) packet;
  
  int i;
  for (i=0; i < callbacks[0x07].length; i++) {
    if (callbacks[0x07].data[i]) {
      callbacks[0x07].data[i](ptr, packet, sizeof(struct ping_packet_t));
    }
  }
  
  il_Network_ping(ptr, ping_packet->timestamp);
  
  return 0;
}

int il_Network_Handler_pong(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen) {
  
  struct pong_packet_t {
    unsigned char id;
    long long timestamp;
  } *pong_packet = (struct pong_packet_t*) packet;
  
  int i;
  for (i=0; i < callbacks[0x08].length; i++) {
    if (callbacks[0x08].data[i]) {
      callbacks[0x08].data[i](ptr, packet, sizeof(struct pong_packet_t));
    }
  }
  
  return 0;
}

int il_Network_Handler_chat(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen) {
  
  struct chat_packet_t {
    unsigned char id;
    unsigned short chatl;
    char chat[];
  } *chat_packet = (struct chat_packet_t*) packet;
  
  int i;
  for (i=0; i < callbacks[0x09].length; i++) {
    if (callbacks[0x09].data[i]) {
      callbacks[0x09].data[i](ptr, packet, sizeof(struct chat_packet_t) + chat_packet->chatl);
    }
  }
  
  return 0;
}
