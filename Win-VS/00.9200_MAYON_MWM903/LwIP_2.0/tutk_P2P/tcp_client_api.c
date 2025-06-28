/**
 * @file tcp_client_api.c
 * @author Barry Lee <barry_lee@mars-semi.com.tw>
 * @date 2018/04/27 (initial)
 * @date 2018/06/19 (last revision)
 * @version 1.0
 */
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "tcp_client_api.h"


static bool TcpVerifyConnection(int sockfd, int *error);
static bool SetNonBlockingMode(int sockfd, bool flag);

/** dump memory block to remote TCP server(netcat)
 * @param[in] mem memory starting address
 * @param[in] mem_size memory size for dump(unit: byte)
 * @param[in] host remote TCP server IP address
 * @param[in] port remote TCP server listen port number
 * @return true: Return true if the function succeeds; otherwise return false.
 */
bool TcpDumpMemClient(const u8 *mem, int mem_size, const char *host, uint16_t port)
{
    int sockfd;
    int total;
    int on = 1;
    struct	sockaddr_in server;
    
    sockfd = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0)
        return false;
    
    setsockopt(sockfd, IPPROTO_TCP, 0x01, &on, sizeof(int)); //TCP_NODELAY    
    server.sin_addr.s_addr = inet_addr((char*)host);
    server.sin_port = htons(port);    
    server.sin_family = AF_INET;    
    
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) != 0)
    {                               
        printf("\x1B[91mdump mem...\x1B[0m\n");
        close(sockfd);
        return false;
    }
    
    total = 0;
    while(total < mem_size)
    {
        int rc = send(sockfd, (char*)&mem[total], mem_size - total, 0);
        if(rc <= 0)
        {
            printf("Video reconnect4!\n");
			close(sockfd);
			return false;
        }
        
        total += rc;
    }

    close(sockfd);
    return true;
}


bool SetNonBlockingMode(int sockfd, bool flag)
{
    int mode;
    
    if(sockfd < 0)
        return false;
    
    mode = fcntl(sockfd, F_GETFL, 0);
    if(flag)
        return (fcntl(sockfd, F_SETFL, mode | O_NONBLOCK) == 0);
    else
        return (fcntl(sockfd, F_SETFL, mode & (~O_NONBLOCK)) == 0);
    
}


/** connect TCP server
 * @param[in] TCP server IP address.
 * @param[in] TCP server listening port number.
 * @param[in] connect timeout (unit: ms)
 * @return Return socket handle(>= 0) if the function succeeds; otherwise return < 0.
 */
int TcpConnect(const char *host, uint16_t port, uint32_t timeout, int keepalvie, int nodelay)
{
    struct sockaddr_in sin;
    
    memset(&sin, 0, sizeof(sin));
    sin.sin_addr.s_addr = inet_addr(host);
    sin.sin_family = AF_INET;        
    sin.sin_port = htons(port);
    return TcpConnect2((struct sockaddr*)&sin, timeout, keepalvie, nodelay);
}


bool TcpVerifyConnection(int sockfd, int *error)
{
    int err;
    socklen_t err_size;
    bool rc;
    
    if(sockfd < 0)
        return false;

    err = 0;
    err_size = sizeof(err);
    if(0 != getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&err, &err_size))
        return false;
        
    if(0 == err || EISCONN == err)
        rc = true;
    else
        rc = false;
    if(error != 0)
        *error = err;        
        
    return rc;
}   


/** Set TCP send() timeout
 * @param[in] sockfd socket handle.
 * @param[in] timeout timeout vlaue(unit: ms)
 * @return Return true if the function succeeds; otherwise return false.
 */
bool TcpSetSendTimeout(int sockfd, u32 timeout)
{    
    return (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == 0);
}


/** Send the data until all done
 * @param[in] sockfd socket handle
 * @param[in] buf output data
 * @param[in] size output data length(unit: byte)
 * @return Return the number of bytes sent.
 */ 
int TcpSendAll(int sk, const char *buf, int size)
{
    int total = 0;
    
    while(total < size)
    {
        int rc = send(sk, &buf[total], size - total, 0);
        if(rc <= 0)
            break;
        
        total += rc;
    }
    
    return total;
}


/** Receive the data until all done
 * @param[in] sockfd socket handle
 * @param[in] buf output data
 * @param[in] size output data length(unit: byte)
 * @return Return the number of bytes sent.
 */ 
int TcpRecvAll(int sk, char *buf, int len)
{
    int total = 0;
    
    while(total < len)
    {
        int rc = recv(sk, &buf[total], len - total, 0);
        if(rc <= 0)
            break;
        
        total += rc;
    }
    
    return total;
}


/** connect TCP server
 * @param[in] TCP server IP address.
 * @param[in] TCP server listening port number.
 * @param[in] connect timeout (unit: ms)
 * @return Return socket handle(>= 0) if the function succeeds; otherwise return < 0.
 */
int TcpConnect2(struct sockaddr* addr, uint32_t timeout, int keepalvie, int nodelay)
{
    struct sockaddr_in sin;
    int sockfd;
    int rc;
    
    fd_set rdevents, wrevents;
    struct timeval tv;
    
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0)
        return INVALID_SOCKET;
    
    //Check the TCP connection whether alive
    if(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalvie, sizeof(keepalvie)) != 0)
        goto TCP_CONNECT_FAIL;
    
    //TCP_NODELAY
    if(setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) != 0) 
        goto TCP_CONNECT_FAIL;
    
    if(!SetNonBlockingMode(sockfd, true))
        goto TCP_CONNECT_FAIL;
    
    rc = connect(sockfd, addr, sizeof(sin));
    if(!rc) //succeed
    {
        if(TcpVerifyConnection(sockfd, 0))
            goto TCP_CONNECT_SUCCESS;
        else
            goto TCP_CONNECT_FAIL;
    }
    else //fail
    {
        int err, err_size = sizeof(int);
        
        if(0 != getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&err, &err_size))
            goto TCP_CONNECT_FAIL;
        
        switch(err)
        {
            case EINPROGRESS:
            case EWOULDBLOCK:
                break;
            default:
                printf("unknown errno=%d\n", err);
            
        }
    }
    
    FD_ZERO(&rdevents);
    FD_SET(sockfd, &rdevents);
    wrevents = rdevents;
    if(timeout >= 1000)
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }
    else
    {
        tv.tv_sec = 0;
        tv.tv_usec = timeout * 1000;
    }    
    
    rc = select(sockfd + 1, &rdevents, &wrevents, 0, &tv);
    if(rc <= 0)
        goto TCP_CONNECT_FAIL;
    
    if(!FD_ISSET(sockfd, &rdevents) && !FD_ISSET(sockfd, &wrevents))
        goto TCP_CONNECT_FAIL;
    
    if(!TcpVerifyConnection(sockfd, 0))
        goto TCP_CONNECT_FAIL;
    
    if(SetNonBlockingMode(sockfd, false))
        goto TCP_CONNECT_SUCCESS;
    
TCP_CONNECT_FAIL:
    if(sockfd >= 0)
    {
        closesocket(sockfd);
        sockfd = -1;
    }

TCP_CONNECT_SUCCESS:
    
    return sockfd;
}