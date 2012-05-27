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
#include "client.h"

#define set_error(E) il_Network_Error.data = (char*)realloc(il_Network_Error.data, strlen(E));\
                     strcpy(il_Network_Error.data, E);\
                     il_Network_Error.length = strlen(E);


int il_Network_RegisterClientHandlers(il_Network_Connection* ptr) {

  ptr->handlers[0x01] = &il_Network_Handler_disconnect;
  ptr->handlers[0x03] = &il_Network_Handler_connectResponse;
  ptr->handlers[0x06] = &il_Network_Handler_probeResponse;
  ptr->handlers[0x08] = &il_Network_Handler_pong;
  ptr->handlers[0x09] = &il_Network_Handler_chat;

  return 0;
}


int il_Network_connect(il_Network_Connection* ptr, const il_Network_String address) {
  
  struct addrinfo hints;
  struct addrinfo *res, *rp;
  
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* Stream socket */
  hints.ai_flags = 0;
  hints.ai_protocol = IPPROTO_TCP; /* TCP kthxbai */
  
  int s = getaddrinfo(address.data, "31173", &hints, &res);
  if (s != 0) {
      char err[80];
      snprintf((char*)&err, 80, "getaddrinfo: %s", gai_strerror(s));
      set_error((char*)&err);
      return -1;
  }
  
  int sfd;
  for (rp = res; rp != NULL; rp = rp->ai_next) {
      sfd = socket(rp->ai_family, rp->ai_socktype,
                   rp->ai_protocol);
      if (sfd == -1)
          continue;

     if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
          break;                  /* Success */

     close(sfd);
  }
  
  if (rp == NULL) {
    set_error("Failed to connect.");
    return -1;
  }
  
  ptr->sock_fd = sfd;
  ptr->version = IL_NETWORK_VERSION;
  
  freeaddrinfo(res);
  
  struct connect_packet_t {
    char id;
    unsigned short version;
    unsigned short hostnamel;
    char hostname[];
  } *connect_packet = (struct connect_packet_t*)malloc(sizeof(struct connect_packet_t) + address.length);
  
  connect_packet->id = 0x02;
  connect_packet->version = IL_NETWORK_VERSION;
  connect_packet->hostnamel = address.length;
  strncpy((char*)&connect_packet->hostname, address.data, address.length);
  
  if (write(sfd, connect_packet, sizeof(connect_packet) + address.length) == -1) {
    set_error(strerror(errno));
    return -1;
  }
  
  return 0;
}

int il_Network_authenticate(il_Network_Connection* ptr, const il_Network_Authcode auth) {
  struct auth_packet_t {
    unsigned char id;
    unsigned short authhashl;
    char authhash[];
  } *auth_packet = (struct auth_packet_t*)malloc(sizeof(struct auth_packet_t) + auth.authcode.length);
  
  auth_packet->id = 0x04;
  auth_packet->authhashl = auth.authcode.length;
  strncpy((char*)&auth_packet->authhash, auth.authcode.data, auth.authcode.length);
  
  if (write(ptr->sock_fd, auth_packet, sizeof(auth_packet) + auth.authcode.length) == -1) {
    set_error(strerror(errno));
    return -1;
  }
  
  return 0;
}

int il_Network_probe(il_Network_Connection* ptr) {

  struct probe_packet_t {
    unsigned char id;
  } probe_packet;
  
  probe_packet.id = 0x05;

  if (write(ptr->sock_fd, &probe_packet, sizeof(struct probe_packet_t)) != -1) {
    set_error(strerror(errno));
    return -1;
  }
  
  return 0;
}


int il_Network_Handler_connectResponse(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen) {

  struct connect_packet_t {
    unsigned char id;
    unsigned short version;
    unsigned int ip_address;
  } *connect_packet = (struct connect_packet_t*) packet;
  
  int i;
  for (i=0; i < callbacks[0x03].length; i++) {
    if (callbacks[0x03].data[i]) {
      callbacks[0x03].data[i](ptr, packet, sizeof(struct connect_packet_t));
    }
  }

  return 0;
}

int il_Network_Handler_probeResponse(il_Network_Connection* ptr, const il_Network_Packet* packet, size_t maxlen) {

  struct probe_packet_t {
    unsigned char id;
    unsigned short version;
    unsigned int ip_address;
    unsigned short players;
  } *probe_packet = (struct probe_packet_t*) packet;
  
  int i;
  for (i=0; i < callbacks[0x06].length; i++) {
    if (callbacks[0x06].data[i]) {
      callbacks[0x06].data[i](ptr, packet, sizeof(struct probe_packet_t));
    }
  }

  return 0;
}
