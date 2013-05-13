#include "network.h"

#ifdef WIN32
# include <winsock.h>
#endif

void il_bootstrap(int argc, char **argv)
{
    (void)argc, (void)argv;
#ifdef WIN32
    WSADATA WSAData;
    WSAStartup(0x101, &WSAData);
#endif
}

