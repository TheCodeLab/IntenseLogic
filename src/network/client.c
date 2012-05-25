#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "network.h"
#include "client.h"

#define set_error(E) il_Network_Error.data = (char*)realloc(il_Network_Error.data, strlen(E));\
                     strcpy(il_Network_Error.data, E);\
                     il_Network_Error.length = strlen(E);

int il_Network_connect(const il_Network_String address, il_Network_Connection* ptr) {
  
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
  
  write(sfd, connect_packet, sizeof(connect_packet));
  
  return 0;
}
