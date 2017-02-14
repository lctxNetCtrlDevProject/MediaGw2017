
#ifndef __OSA_NETWORK_H__
#define __OSA_NETWORK_H__
#include "commonTypes.h"



extern void osa_closeSock(SOCKET_TYPE sockfd);
extern SOCKET_TYPE osa_udpCreateSock();
extern SOCKET_TYPE osa_udpCreateBindSock(char *ipaddr, unsigned short port);
extern int osa_udpSendData(SOCKET_TYPE sock_fd, unsigned char* buf, int length,unsigned char *ipaddr, unsigned short port);
extern int osa_udpSndDataEx( unsigned char* buf, int length,unsigned char *ipaddr, unsigned short port);
extern int osa_sockRcvData(SOCKET_TYPE sock_fd, unsigned char *buf, int bufLen);

extern char *osa_inet_ntoa(unsigned int addr);
extern int osa_inet_aton(const char *ip, unsigned int *addr);

#endif

