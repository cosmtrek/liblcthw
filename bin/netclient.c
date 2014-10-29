#undef NDEBUG
#include <stdlib.h>
#include <sys/select.h>
#include <stdio.h>
#include <lcthw/ring_buffer.h>
#include <lcthw/dbg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

struct tagbstring NL = bsStatic("\n");
struct tagbstring CRLF = bsStatic("\r\n");

int nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    check(flags >= 0, "Invalid flags on nonblock.");

    int rc = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    check(rc == 0, "Can't set nonblocking.");

    return 0;

error:
    return -1;
}

int client_connet(char *host, char *port)
{
    int rc = 0;
    struct addrinfo *addr = NULL;

    // int getaddrinfo(const char *restrict host,
    //                 const char *restrict service,
    //                 const struct addrinfo *restrict hint,
    //                 struct addrinfo **restrict res)
    // rc:
    //   - success: 0
    //   - fail: !0
    rc = getaddrinfo(host, port, NULL, &addr);
    check(rc == 0, "Failed to look up %s:%s", host, port);

    // int socket(int domain, int type, int protocol)
    // AF_INET: IPv4
    // SOCK_STREAM: 有序的、可靠的、双向的、面向连接的字节流
    // 0: 默认协议
    // rc:
    //   - success: fd
    //   - fail: -1
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    check(sock >= 0, "Cannot create a socket.");

    // int connect(int sockfd, const struct sockaddr *addr, socklen_t len)
    // rc:
    //   - success: 0
    //   - fail: -1
    rc = connect(sock, addr->ai_addr, addr->ai_addrlen);
    check(rc == 0, "Connect failed.");

    rc = nonblock(sock);
    check(rc == 0, "Can't set nonblock.");

    freeaddrinfo(addr);
    return sock;

error:
    freeaddrinfo(addr);
    return -1;
}

int read_some(RingBuffer *buffer, int fd, int is_socket)
{
    int rc = 0;

    if (RingBuffer_available_data(buffer) == 0) {
        buffer->start = buffer->end = 0;
    }

    // ssize_t recv(int sockfd, void *buf, size_t nbytes, int flags)
    // rc:
    //   - success: nbytes
    //   - fail: -1
    //   - nil: 0
    if (is_socket) {
        rc = recv(fd, RingBuffer_starts_at(buffer), RingBuffer_available_space(buffer), 0);
    } else {
        rc = read(fd, RingBuffer_starts_at(buffer), RingBuffer_available_space(buffer));
    }

    check(rc >= 0, "Failed to read from fd: %d", fd);

    RingBuffer_commit_write(buffer, rc);

    return rc;

error:
    return -1;
}

int write_some(RingBuffer *buffer, int fd, int is_socket)
{
    int rc = 0;
    bstring data = RingBuffer_get_all(buffer);

    check(data != NULL, "Failed to get from buffer.");
    check(bfindreplace(data, &NL, &CRLF, 0) == BSTR_OK, "Failed ro replace NL.");

    // ssize_t send(int sockfd, const void *buf, size_t nbytes, int flags)
    // rc:
    //   - success: nbytes
    //   - fail: -1

    if (is_socket) {
        rc = send(fd, bdata(data), blength(data), 0);
    } else {
        rc = write(fd, bdata(data), blength(data));
    }

    check(rc == blength(data), "Failed to write anything to fd:%d", fd);
    bdestroy(data);

    return rc;

error:
    return -1;
}

int main(int argc, char *argv[])
{
    fd_set allreads;
    fd_set readmask;

    int socket = 0;
    int rc = 0;
    RingBuffer *in_rb = RingBuffer_create(1024 * 10);
    RingBuffer *sock_rb = RingBuffer_create(1024 * 10);

    check(argc == 3, "USAGE: netclient host port");

    socket = client_connet(argv[1], argv[2]);
    check(socket >= 0, "connected to %s:%s failed", argv[1], argv[2]);

    FD_ZERO(&allreads);
    FD_SET(socket, &allreads);
    FD_SET(0, &allreads);

    // int select(int maxfdp1, fd_set *restrict readfds,
    //         fd_set *restrict writefds, fd_set *restrict exceptfds,
    //         struct timeval *restrict tvptr)
    // maxfdp1: 最大描述符 + 1
    // rc:
    //   - success: fd
    //   - fail: -1
    //   - timeout: 0
    while (1) {
        readmask = allreads;
        rc = select(socket + 1, &readmask, NULL, NULL, NULL);
        check(rc >= 0, "select failed");

        if (FD_ISSET(0, &readmask)) {
            rc = read_some(in_rb, 0, 0);
            check_debug(rc != -1, "Failed to read from stdin.");
        }

        if (FD_ISSET(socket, &readmask)) {
            rc = read_some(sock_rb, socket, 0);
            check_debug(rc != -1, "Failed to read from socket.");
        }

        while (!RingBuffer_empty(sock_rb)) {
            rc = write_some(sock_rb, 1, 0);
            check_debug(rc != -1, "Failed to write to stdout.");
        }

        while (!RingBuffer_empty(in_rb)) {
            rc = write_some(in_rb, socket, 1);
            check_debug(rc != -1, "Failed to write to socket.");
        }
    }

    return 0;

error:
    return -1;
}
