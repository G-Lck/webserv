# man: select()

## Prototype

```cpp
#include <sys/select.h>
int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

FD_SET(int fd, fd_set *set);
FD_CLR(int fd, fd_set *set);
FD_ISSET(int fd, fd_set *set);
FD_ZERO(fd_set *set);
```

## Description

The **select()** function gives you a way to simultaneously check multiple sockets to see if they have data waiting to be **recv()**d, or if you can **send()** data to them without blocking, or if some exception has occurred.

You populate your sets of socket descriptors using macros like **FD_SET()**.
Then you pass one or more of these sets to **select()**:

- **readfds** to know when sockets are ready to **recv()** data
- **writefds** to know when sockets are ready to **send()** data
- **exceptfds** to know when socket exceptions occur

Any of these parameters can be **NULL** if you are not interested in that event type.
After **select()** returns, these sets are modified to show which descriptors are ready.

The first parameter, **n**, is the highest-numbered file descriptor in any set, plus one.

**timeout** controls how long **select()** waits before returning.
It uses **struct timeval**:

- **tv_sec**: seconds
- **tv_usec**: microseconds

**select()** returns when at least one event is ready, or when the timeout expires.

## Helper macros

| Macro | Description |
|---|---|
| **FD_SET(int fd, fd_set *set);** | Add **fd** to the set. |
| **FD_CLR(int fd, fd_set *set);** | Remove **fd** from the set. |
| **FD_ISSET(int fd, fd_set *set);** | Return true if **fd** is in the set. |
| **FD_ZERO(fd_set *set);** | Clear all entries from the set. |

## Note for Linux users

Linux **select()** can return ready-to-read and then not actually be ready to read, causing the subsequent **read()** call to block.

A common workaround is to set **O_NONBLOCK** on the receiving socket so that **read()** fails with **EWOULDBLOCK**. If that happens, ignore this specific error and continue.

See **fcntl()** man page for details on non-blocking mode.

## Return Value

- Number of ready descriptors on success
- **0** if timeout was reached
- **-1** on error (**errno** is set)

The descriptor sets are modified in-place to indicate readiness.

## Example

```cpp
int s1, s2, n, rv;
fd_set readfds;
struct timeval tv;
char buf1[256], buf2[256];

// Pretend both sockets are already connected to a server.
// s1 = socket(...);
// s2 = socket(...);
// connect(s1, ...);
// connect(s2, ...);

// Clear the set, then add both sockets.
FD_ZERO(&readfds);
FD_SET(s1, &readfds);
FD_SET(s2, &readfds);

// n must be highest fd + 1.
n = s2 + 1;

// Wait up to 10.5 seconds for readability events.
tv.tv_sec = 10;
tv.tv_usec = 500000;
rv = select(n, &readfds, NULL, NULL, &tv);

if (rv == -1) {
	perror("select");
} else if (rv == 0) {
	printf("Timeout occurred! No data after 10.5 seconds.\n");
} else {
	if (FD_ISSET(s1, &readfds)) {
		recv(s1, buf1, sizeof(buf1), 0);
	}
	if (FD_ISSET(s2, &readfds)) {
		recv(s2, buf2, sizeof(buf2), 0);
	}
}
```
