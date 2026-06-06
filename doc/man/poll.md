# man: poll()

## Purpose

Test for events on multiple sockets simultaneously.

## Synopsis

```cpp
#include <sys/poll.h>
int poll(struct pollfd *ufds, unsigned int nfds, int timeout);
```

## Description

This function is very similar to **select()**: both watch sets of file descriptors for events, such as incoming data ready to **recv()**, socket ready to **send()** data, out-of-band data, errors, and more.

The basic idea is to pass an array of **nfds** elements of **struct pollfd** in **ufds**, with a timeout in milliseconds (**1000 ms = 1 s**).

- A negative timeout means wait forever.
- If no event happens before the timeout, **poll()** returns.

Each element in the array represents one descriptor:

```cpp
struct pollfd {
    int fd;       // the socket descriptor
    short events; // bitmap of events we want
    short revents;// bitmap of events that occurred
};
```

Before calling **poll()**:

- set **fd** with the descriptor
- set **events** by bitwise-ORing event macros
- if **fd** is negative, that entry is ignored and **revents** is set to zero

## Event macros (input in events)

| Macro | Description |
|---|---|
| **POLLIN** | Alert when data is ready to **recv()** on this socket. |
| **POLLOUT** | Alert when you can **send()** data without blocking. |
| **POLLPRI** | Alert when out-of-band data is ready to **recv()**. |

After **poll()** returns, **revents** contains a bitwise-OR of events that actually occurred.

## Additional flags (output in revents)

| Macro | Description |
|---|---|
| **POLLERR** | An error occurred on this socket. |
| **POLLHUP** | The remote side hung up. |
| **POLLNVAL** | Invalid descriptor (for example uninitialized **fd**). |

## Return Value

- Number of elements in **ufds** that have events
- **0** if timeout occurred
- **-1** on error (**errno** is set)

## Example

```cpp
int s1, s2;
int rv;
char buf1[256], buf2[256];
struct pollfd ufds[2];

s1 = socket(PF_INET, SOCK_STREAM, 0);
s2 = socket(PF_INET, SOCK_STREAM, 0);

// Pretend both sockets are already connected to a server.
// connect(s1, ...);
// connect(s2, ...);

// We want normal and OOB data on s1, normal data on s2.
ufds[0].fd = s1;
ufds[0].events = POLLIN | POLLPRI;

ufds[1].fd = s2;
ufds[1].events = POLLIN;

// Wait up to 3.5 seconds.
rv = poll(ufds, 2, 3500);

if (rv == -1) {
    perror("poll");
} else if (rv == 0) {
    printf("Timeout occurred! No data after 3.5 seconds.\n");
} else {
    if (ufds[0].revents & POLLIN) {
        recv(s1, buf1, sizeof(buf1), 0);
    }
    if (ufds[0].revents & POLLPRI) {
        recv(s1, buf1, sizeof(buf1), MSG_OOB);
    }

    if (ufds[1].revents & POLLIN) {
        recv(s2, buf2, sizeof(buf2), 0);
    }
}
```
