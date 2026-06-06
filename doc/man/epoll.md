# man: epoll

## Purpose

Monitor many file descriptors for I/O readiness (Linux-specific), with good scalability for large numbers of watched descriptors.

## Synopsis

```cpp
#include <sys/epoll.h>

int epoll_create(int size);
int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

## Core idea

The epoll API is conceptually close to poll(), but with a different kernel model:

- Interest list: descriptors you registered
- Ready list: subset currently ready for I/O

Main syscalls:

- **epoll_create1()** creates an epoll instance (returns an fd)
- **epoll_ctl()** adds/modifies/removes watched descriptors
- **epoll_wait()** waits for events and returns ready descriptors

## Data structures

```cpp
typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t events; // epoll flags
    epoll_data_t data;
};
```

## epoll_create1 flags

| Flag | Description |
|---|---|
| **EPOLL_CLOEXEC** | Set close-on-exec on epoll fd (like O_CLOEXEC). |

## epoll_ctl operations

| op | Description |
|---|---|
| **EPOLL_CTL_ADD** | Add fd to interest list. |
| **EPOLL_CTL_MOD** | Modify event mask/data for an existing fd. |
| **EPOLL_CTL_DEL** | Remove fd from interest list. |

## Event flags (events / revents)

| Flag | Description |
|---|---|
| **EPOLLIN** | Readable data available. |
| **EPOLLOUT** | Writable without blocking. |
| **EPOLLPRI** | Urgent/OOB data available. |
| **EPOLLERR** | Error condition (always reported). |
| **EPOLLHUP** | Hang up detected. |
| **EPOLLRDHUP** | Peer closed read-half (useful for TCP). |
| **EPOLLET** | Edge-triggered mode. |
| **EPOLLONESHOT** | Disable fd after one event until re-armed with MOD. |
| **EPOLLWAKEUP** | Keep system awake while processing event (power-management use case). |

## Level-triggered vs Edge-triggered

- Default (no EPOLLET): level-triggered, behavior close to poll().
- With EPOLLET: edge-triggered, event delivered on state transitions.

Important rule in edge-triggered mode:

- Use non-blocking fds.
- Read/write in a loop until read()/write() returns EAGAIN.

If you do partial reads and stop too early, epoll_wait() can sleep while data is still buffered.

## Return Value

### epoll_create / epoll_create1

- New epoll fd on success
- -1 on error (errno set)

### epoll_ctl

- 0 on success
- -1 on error (errno set)

### epoll_wait

- Number of ready events (> 0)
- 0 on timeout
- -1 on error (errno set)

## Example (typical server loop)

```cpp
#define MAX_EVENTS 10

struct epoll_event ev, events[MAX_EVENTS];
int listen_sock, conn_sock, epollfd, nfds, n;

// socket(), bind(), listen() done before this point.

epollfd = epoll_create1(0);
if (epollfd == -1) {
    perror("epoll_create1");
    exit(EXIT_FAILURE);
}

ev.events = EPOLLIN;
ev.data.fd = listen_sock;
if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
    perror("epoll_ctl: listen_sock");
    exit(EXIT_FAILURE);
}

for (;;) {
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }

    for (n = 0; n < nfds; ++n) {
        if (events[n].data.fd == listen_sock) {
            conn_sock = accept(listen_sock, NULL, NULL);
            if (conn_sock == -1) {
                perror("accept");
                continue;
            }

            // setnonblocking(conn_sock);
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = conn_sock;
            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                perror("epoll_ctl: conn_sock");
                close(conn_sock);
            }
        } else {
            // do_use_fd(events[n].data.fd);
        }
    }
}
```
