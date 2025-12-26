#ifndef NET_COMPAT_H
#define NET_COMPAT_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET talea_net_t;
#define T_INVALID_SOCKET INVALID_SOCKET
#define T_NET_ERROR      SOCKET_ERROR

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
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int talea_net_t;
#define T_INVALID_SOCKET -1
#define T_NET_ERROR      -1

inline bool net_would_block()
{
    return errno == EAGAIN || errno == EWOULDBLOCK;
}
inline void net_close(talea_net_t s)
{
    close(s);
}
#endif

#endif
