#include "network.h"

void il_bootstrap(int argc, char **argv)
{
    (void)argc, (void)argv;
#ifdef WIN32
    WSADATA WSAData;
    WSAStartup(0x101, &WSAData);
#endif
}

