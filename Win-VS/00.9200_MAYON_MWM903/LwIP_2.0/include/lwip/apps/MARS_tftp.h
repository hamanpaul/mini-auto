#ifndef     _MYTFTPSERVERIF_H_
#define     _MYTFTPSERVERIF_H_

#include "lwip/mem.h"
#include "lwip/udp.h"
#include "tftp_server.h"

extern const struct tftp_context  tftpContext;

enum
{
    Memory_mode = 0,
    SD_mode
};
#endif

