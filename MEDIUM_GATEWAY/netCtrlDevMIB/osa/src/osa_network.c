/**
*@Desc: Provide OS independent network socket api
*@Author: Andy-wei.hou
*@Log:	Created by Andy-wei.hou 2016.10.25
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "osa.h"
#include <unistd.h>




void osa_closeSock(SOCKET_TYPE sockfd)
{
	close(sockfd);
}

//创建socket描述符
SOCKET_TYPE osa_udpCreateSock()
{
	SOCKET_TYPE sockfd = -1;
	sockfd = socket(AF_INET,SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		OSA_ERROR("Create udp socket fail, errno[%d]",errno);
	}
	return sockfd;
}

SOCKET_TYPE osa_udpCreateBindSock(char *ipaddr, unsigned short port){
	SOCKET_TYPE local_sockfd;
	struct sockaddr_in local_address;
	int len;
	int  iSockopt;

	local_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(local_sockfd == -1)
	{
		OSA_ERROR("Create udp socket fail, errno[%d]",errno);
	}
	memset(&local_address,0x00, sizeof(local_address));
	local_address.sin_family = AF_INET;
	if (NULL == ipaddr)
	{
		local_address.sin_addr.s_addr = inet_addr("0.0.0.0");
	}
	else
	{
		local_address.sin_addr.s_addr = inet_addr(ipaddr);
	}
	local_address.sin_port = htons(port);


	iSockopt = 1;
	//设置Socket属性：SO_REUSEADDR：允许在bind过程中本地地址重复使用
    if (0  > setsockopt(local_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&iSockopt, sizeof(int)))
	{
		osa_closeSock(local_sockfd);
		return -1;
	}

	len=sizeof(local_address);
	if (-1 == bind(local_sockfd, (struct sockaddr *)&local_address, len))
	{
		OSA_ERROR("port =%d socket bind error!,errno=%d",port,errno);
		return -1;
	}
	return(local_sockfd);
}

//发送数据
int osa_udpSendData(SOCKET_TYPE sock_fd, unsigned char* buf, int length,unsigned char *ipaddr, unsigned short port)
{
    struct sockaddr_in addr;
	int addr_len;
	int len;

	/* 设置对方地址和端口信息 */
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ipaddr);

	/* 发送UDP消息 */
	addr_len = sizeof(addr);
	len = sendto(sock_fd, buf, length, 0,(struct sockaddr *) &addr, addr_len);
	if (len < 0)
	{
		return -1;
	}
	return len;
}

int osa_udpSndDataEx( unsigned char* buf, int length,unsigned char *ipaddr, unsigned short port){
	SOCKET_TYPE fd = osa_udpCreateSock();
	int iRet = -1;
	if(fd >=0){
		iRet = osa_udpSendData(fd,buf,length,ipaddr,port);
		osa_closeSock(fd);
	}
	return iRet;
}

int osa_sockRcvData(SOCKET_TYPE sock_fd, unsigned char *buf, int bufLen){
	struct sockaddr_in addr;
	int addrLen = sizeof(addr);
	int iRet = -1;
	if(sock_fd <=0 || !buf || bufLen <=0){
		OSA_ERROR("Invalid parameter");
		return -1;
	}

	iRet = recvfrom(sock_fd,buf,bufLen,0,(struct sockaddr *)&addr,&addrLen);
	if(iRet < 0){
		OSA_ERROR("recvfrom Fail, errno=%d",errno);
	}
	return iRet;

}
char *osa_inet_ntoa(unsigned int addr){
	struct in_addr address;
	address.s_addr = addr;
	return inet_ntoa(address);
}



int osa_inet_aton(const char *ip, unsigned int *addr){
	if(!addr || !ip)
		return -1;
	*addr = htonl(inet_addr(ip));
	return 0;
}


