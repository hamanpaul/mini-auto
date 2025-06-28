#ifndef TCP_CLIENT_API_H
#define TCP_CLIENT_API_H

#include <stdint.h>

#define INVALID_SOCKET -1

bool TcpDumpMemClient(const u8 *mem, int mem_size, const char *host, uint16_t port);
int TcpConnect(const char *host, uint16_t port, uint32_t timeout, int keepalvie, int nodelay);
int TcpConnect2(struct sockaddr* addr, uint32_t timeout, int keepalvie, int nodelay);
bool TcpSetSendTimeout(int fd, uint32_t timeout);
int TcpSendAll(int fd, const char *buf, int size);
int TcpRecvAll(int sk, char *buf, int len);


#endif
