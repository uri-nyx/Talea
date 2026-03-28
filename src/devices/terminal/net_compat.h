#ifndef NET_COMPAT_H
#define NET_COMPAT_H

#ifdef _WIN32
#include <stdbool.h>
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET talea_net_t;
#define T_INVALID_SOCKET INVALID_SOCKET
#define T_NET_ERROR      SOCKET_ERROR

#define T_EINPROGRESS WSAEWOULDBLOCK

#define net_set_nonblocking(s)          \
    {                                   \
        u_long mode = 1;                \
        ioctlsocket(s, FIONBIO, &mode); \
    }

// Check if the error is just "nothing ready yet"
inline bool net_would_block()
{
    int err = WSAGetLastError();
    // printf("NET WOULD BLOCK ERROR %d\n", err);
    return err == WSAEWOULDBLOCK;
}

inline void net_close(talea_net_t s)
{
    closesocket(s);
}

#else
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int talea_net_t;
#define T_INVALID_SOCKET       -1
#define T_NET_ERROR            -1
#define T_EINPROGRESS          EINPROGRESS
#define net_set_nonblocking(s) fcntl(s, F_SETFL, O_NONBLOCK)

inline bool net_would_block()
{
  //TALEA_LOG_TRACE("NET WOULD BLOCK POSIX (must be %d or %d); %d\n", EAGAIN, EWOULDBLOCK, errno);
    return errno == EAGAIN || errno == EWOULDBLOCK || EINPROGRESS;
}
inline void net_close(talea_net_t s)
{
    close(s);
}

#endif

#endif
