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
#include <sys/select.h>
#include <sys/timeb.h>

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

int il_Network_ping(il_Network_Connection* ptr, long long timestamp, unsigned short msec) {

  struct ping_packet_t {
    unsigned char id;
    long long timestamp;
    unsigned short msec;
  } ping_packet;
  
  ping_packet.id = 0x07;
  ping_packet.timestamp = timestamp;
  ping_packet.msec = msec;
  
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
  
  if (sizeof(struct disconnect_packet_t) < maxlen)
    return 0;
  if (sizeof(struct disconnect_packet_t) + disconnect_packet->reasonl < maxlen)
    return 0;
  
  size_t len = sizeof(struct disconnect_packet_t) + disconnect_packet->reasonl;
  
  int i;
  for (i=0; i < callbacks[0x01].length; i++) {
    if (callbacks[0x01].data[i]) {
      callbacks[0x01].data[i](ptr, packet, len);
    }
  }

  return len;
}

int il_Network_Handler_ping(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen) {
  
  struct ping_packet_t {
    unsigned char id;
    long long timestamp;
    unsigned short msec;
  } *ping_packet = (struct ping_packet_t*) packet;
  
  if (sizeof(struct ping_packet_t) < maxlen)
    return 0;
  
  int i;
  for (i=0; i < callbacks[0x07].length; i++) {
    if (callbacks[0x07].data[i]) {
      callbacks[0x07].data[i](ptr, packet, sizeof(struct ping_packet_t));
    }
  }
  
  il_Network_ping(ptr, ping_packet->timestamp, ping_packet->msec);
  
  return sizeof(struct ping_packet_t);
}

int il_Network_Handler_pong(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen) {
  
  struct pong_packet_t {
    unsigned char id;
    long long timestamp;
    unsigned short msec;
  } *pong_packet = (struct pong_packet_t*) packet;
  
  if (sizeof(struct pong_packet_t) < maxlen)
    return 0;
  
  int i;
  for (i=0; i < callbacks[0x08].length; i++) {
    if (callbacks[0x08].data[i]) {
      callbacks[0x08].data[i](ptr, packet, sizeof(struct pong_packet_t));
    }
  }
  
  struct timeb tp;
  ftime(&tp);
  
  ptr->latency = (float)(tp.time - (time_t)pong_packet->timestamp) + (tp.millitm - pong_packet->msec)/1000.0;
  
  return sizeof(struct pong_packet_t);
}

int il_Network_Handler_chat(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen) {
  
  struct chat_packet_t {
    unsigned char id;
    unsigned short chatl;
    char chat[];
  } *chat_packet = (struct chat_packet_t*) packet;
  
  if (sizeof(struct chat_packet_t) < maxlen)
    return 0;
  if (sizeof(struct chat_packet_t) + chat_packet->chatl < maxlen)
    return 0;
  
  int i;
  for (i=0; i < callbacks[0x09].length; i++) {
    if (callbacks[0x09].data[i]) {
      callbacks[0x09].data[i](ptr, packet, sizeof(struct chat_packet_t) + chat_packet->chatl);
    }
  }
  
  return sizeof(struct chat_packet_t) + chat_packet->chatl;
}

void* extrabuf;
size_t overflow;
int il_Network_update(il_Network_Connection* ptr) {

  int pos = overflow;
  void* data = malloc(1024 + overflow);
  memcpy(data, extrabuf, overflow);
  free(extrabuf);
  ssize_t size;
  
  size = recv(ptr->sock_fd, data, 1024, MSG_DONTWAIT);
  
  if (size <= 0)
    return -1;
  
  size += overflow;
  
  while (pos < size) {
    unsigned char id = (unsigned char)data+pos;
    if (!ptr->handlers[id])
      return -1;
    int res = ptr->handlers[id](ptr, (il_Network_Packet*)data+pos, size-pos);
    if (res > 0) {
      pos += res;
      continue;
    }
    overflow = size - pos;
    extrabuf = malloc(overflow);
    memcpy(extrabuf, data+pos, overflow);
    return 0;
  }
  
  return 0;
}
