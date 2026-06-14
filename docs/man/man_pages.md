# Man Pages - Webserv (42)

## Table des matieres
- [socket()](#socket)
- [bind()](#bind)
- [listen()](#listen)
- [accept()](#accept)
- [send() / sendto()](#send--sendto)
- [recv() / recvfrom()](#recv--recvfrom)
- [close()](#close)
- [fcntl()](#fcntl)
- [poll()](#man-poll)
- [select()](#man-select)
- [byte-order](#byte-order-htons-htonl-ntohs-ntohl)
- [epol](#man-epoll)
---

## `socket()`

Create a socket descriptor

### Synopsis

```c
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

### Description

Returns a new socket descriptor that you can use to do sockety things with. This is generally the first
call in the whopping process of writing a socket program, and you can think of it as the "birth" of a
socket.

The old-school way of doing things was to call `socket()` with the specific values for `domain`, `type`, and
`protocol`. The new and improved way is to use `getaddrinfo()` and plug the results directly into
`socket()`:

```c
struct addrinfo *res;
// ... getaddrinfo() call ...
int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
```

| Parameter | Description |
|-----------|-------------|
| `domain` | `PF_INET` for IPv4, `PF_INET6` for IPv6 |
| `type` | `SOCK_STREAM` for TCP, `SOCK_DGRAM` for UDP |
| `protocol` | Set to `0` to auto-choose, or use `getprotobyname()` |

### Return Value

- **Success:** new socket descriptor (a non-negative integer)
- **Failure:** `-1`, `errno` set accordingly

### Example

```c
struct addrinfo hints, *res;
int sockfd;

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;

getaddrinfo("www.example.com", "3490", &hints, &res);

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
```

### See Also

`accept()`, `bind()`, `getaddrinfo()`, `listen()`

[↑ Back to top](#table-des-matieres)

---

## `bind()`

Associate a socket with an IP address and port number

### Synopsis

```c
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
```

### Description

When a remote machine wants to connect to your server program, it needs two pieces of information: the
IP address and the port number. The `bind()` call allows you to do just that.

First, you call `getaddrinfo()` to load up a `struct sockaddr` with the destination address and port
information. Then you call `socket()` to get a socket descriptor, and then you pass the socket and
address into `bind()`, and the IP address and port are magically (using actual magic) bound to the socket!

If you don't know your IP address, or you know you only have one IP address on the machine, or you
don't care which of the machine's IP addresses is used, you can simply pass the `AI_PASSIVE` flag in the
hints parameter to `getaddrinfo()`. What this does is fill in the IP address part of the `struct sockaddr`
with a special value that tells `bind()` that it should automatically fill in this host's IP address.

What special value is loaded into the `struct sockaddr`'s IP address to cause it to auto-fill? In IPv4,
the `sin_addr.s_addr` field of the `struct sockaddr_in` structure is set to `INADDR_ANY`. In IPv6,
the `sin6_addr` field of the `struct sockaddr_in6` structure is assigned from the global variable
`in6addr_any`. Or, if you're declaring a new `struct in6_addr`, you can initialize it to
`IN6ADDR_ANY_INIT`.

Lastly, the `addrlen` parameter should be set to `sizeof my_addr`.

### Return Value

- **Success:** `0`
- **Failure:** `-1`, `errno` set accordingly

### Example

```c
// modern way of doing things with getaddrinfo()

struct addrinfo hints, *res;
int sockfd;

// first, load up address structs with getaddrinfo():

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE; // fill in my IP for me

getaddrinfo(NULL, "3490", &hints, &res);

// make a socket:
// (you should actually walk the "res" linked list and error-check!)

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// bind it to the port we passed in to getaddrinfo():

bind(sockfd, res->ai_addr, res->ai_addrlen);
```

```c
// example of packing a struct by hand, IPv4

struct sockaddr_in myaddr;
int s;

myaddr.sin_family = AF_INET;
myaddr.sin_port = htons(3490);

// you can specify an IP address:
inet_pton(AF_INET, "63.161.169.137", &(myaddr.sin_addr));

// or you can let it automatically select one:
myaddr.sin_addr.s_addr = INADDR_ANY;

s = socket(PF_INET, SOCK_STREAM, 0);
bind(s, (struct sockaddr*)&myaddr, sizeof myaddr);
```

### See Also

`getaddrinfo()`, `socket()`, `struct sockaddr_in`, `struct in_addr`

[↑ Back to top](#table-des-matieres)

---

## `listen()`

Tell a socket to listen for incoming connections

### Synopsis

```c
#include <sys/socket.h>

int listen(int s, int backlog);
```

### Description

You can take your socket descriptor (made with the `socket()` system call) and tell it to listen for
incoming connections. This is what differentiates the servers from the clients, guys.

The `backlog` parameter can mean a couple different things depending on the system you on, but loosely
it is how many pending connections you can have before the kernel starts rejecting new ones. So as the
new connections come in, you should be quick to `accept()` them so that the backlog doesn't fill. Try
setting it to `10` or so, and if your clients start getting "Connection refused" under heavy load, set it
higher.

Before calling `listen()`, your server should call `bind()` to attach itself to a specific port number. That
port number (on the server's IP address) will be the one that clients connect to.

### Return Value

- **Success:** `0`
- **Failure:** `-1`, `errno` set accordingly

### Example

```c
struct addrinfo hints, *res;
int sockfd;

// first, load up address structs with getaddrinfo():

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE; // fill in my IP for me

getaddrinfo(NULL, "3490", &hints, &res);

// make a socket:

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// bind it to the port we passed in to getaddrinfo():

bind(sockfd, res->ai_addr, res->ai_addrlen);

listen(sockfd, 10); // set sockfd up to be a server socket

// then have an accept() loop down here somewhere
```

### See Also

`accept()`, `bind()`, `socket()`

[↑ Back to top](#table-des-matieres)

---

## `accept()`

Accept an incoming connection on a listening socket

### Synopsis

```c
#include <sys/types.h>
#include <sys/socket.h>

int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
```

### Description

Once you've gone through the trouble of getting a `SOCK_STREAM` socket and setting it up for incoming
connections with `listen()`, then you call `accept()` to actually get yourself a new socket descriptor to
use for subsequent communication with the newly connected client.

The old socket that you are using for listening is still there, and will be used for further `accept()` calls
as they come in.

| Parameter | Description |
|-----------|-------------|
| `s` | The `listen()`ing socket descriptor |
| `addr` | This is filled in with the address of the site that's connecting to you |
| `addrlen` | This is filled in with the `sizeof()` the structure returned in the `addr` parameter. You can safely ignore it if you assume you're getting a `struct sockaddr_in` back, which you know you are, because that's the type you passed in for `addr` |

`accept()` will normally block, and you can use `select()` to peek on the listening socket descriptor
ahead of time to see if it's "ready to read". If so, then there's a new connection waiting to be `accept()`ed!

Alternatively, you could set the `O_NONBLOCK` flag on the listening socket using `fcntl()`, and then it
will never block, choosing instead to return `-1` with `errno` set to `EWOULDBLOCK`.

The socket descriptor returned by `accept()` is a bona fide socket descriptor, open and connected to the
remote host. You have to `close()` it when you're done with it.

### Return Value

- **Success:** newly connected socket descriptor
- **Failure:** `-1`, `errno` set accordingly

### Example

```c
struct sockaddr_storage their_addr;
socklen_t addr_size;
struct addrinfo hints, *res;
int sockfd, new_fd;

// first, load up address structs with getaddrinfo():

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE; // fill in my IP for me

getaddrinfo(NULL, MYPORT, &hints, &res);

// make a socket, bind it, and listen on it:

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
bind(sockfd, res->ai_addr, res->ai_addrlen);
listen(sockfd, BACKLOG);

// now accept an incoming connection:

addr_size = sizeof their_addr;
new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

// ready to communicate on socket descriptor new_fd!
```

### See Also

`socket()`, `getaddrinfo()`, `listen()`, `struct sockaddr_in`

[↑ Back to top](#table-des-matieres)

---

## `send()` / `sendto()`

Send data out over a socket

### Synopsis

```c
#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int s, const void *buf, size_t len, int flags);
ssize_t sendto(int s, const void *buf, size_t len,
               int flags, const struct sockaddr *to,
               socklen_t tolen);
```

### Description

These functions send data to a socket. Generally speaking, `send()` is used for TCP `SOCK_STREAM`
connected sockets, and `sendto()` is used for UDP `SOCK_DGRAM` unconnected datagram sockets. With
the unconnected sockets, you must specify the destination of a packet each time you send one, and that's
why the last parameters of `sendto()` define where the packet is going.

With both `send()` and `sendto()`, the parameter `s` is the socket, `buf` is a pointer to the data you want
to send, `len` is the number of bytes you want to send, and `flags` allows you to specify more information
about how the data is to be sent. Set `flags` to zero if you want it to be "normal" data. Here are some of
the commonly used flags:

| Macro | Description |
|-------|-------------|
| `MSG_OOB` | Send as "out of band" data. TCP supports this, and it's a way to tell the receiving system that this data has a higher priority than the normal data. The receiver will receive the signal `SIGURG` and it can then receive this data without first receiving all the rest of the normal data in the queue. |
| `MSG_DONTROUTE` | Don't send this data over a router, just keep it local. |
| `MSG_DONTWAIT` | If `send()` would block because outbound traffic is clogged, have it return `EAGAIN`. This is like a "enable non-blocking just for this send." |
| `MSG_NOSIGNAL` | If you `send()` to a remote host which is no longer `recv()`ing, you'll typically get the signal `SIGPIPE`. Adding this flag prevents that signal from being raised. |

### Return Value

Returns the number of bytes actually sent, or `-1` on error (and `errno` will be set accordingly). Note
that the number of bytes actually sent might be less than the number you asked it to send! See the section
on handling partial `send()`s for a helper function to get around this.

Also, if the socket has been closed by either side, the process calling `send()` will get the signal `SIGPIPE`.
(Unless `send()` was called with the `MSG_NOSIGNAL` flag.)

### Example

```c
int spatula_count = 3490;
char *secret_message = "The Cheese is in The Toaster";

int stream_socket, dgram_socket;
struct sockaddr_in dest;
int temp;

// first with TCP stream sockets:

// assume sockets are made and connected
//stream_socket = socket(...
//connect(stream_socket, ...

// convert to network byte order
temp = htonl(spatula_count);
// send data normally:
send(stream_socket, &temp, sizeof temp, 0);

// send secret message out of band:
send(stream_socket, secret_message, strlen(secret_message)+1, MSG_OOB);

// now with UDP datagram sockets:
//getaddrinfo(...
//dest = ... // assume "dest" holds the address of the destination
//dgram_socket = socket(...

// send secret message normally:
sendto(dgram_socket, secret_message, strlen(secret_message)+1, 0,
       (struct sockaddr*)&dest, sizeof dest);
```

### See Also

`recv()`, `recvfrom()`

[↑ Back to top](#table-des-matieres)

---

## `recv()` / `recvfrom()`

Receive data on a socket

### Synopsis

```c
#include <sys/types.h>
#include <sys/socket.h>

ssize_t recv(int s, void *buf, size_t len, int flags);
ssize_t recvfrom(int s, void *buf, size_t len, int flags,
                 struct sockaddr *from, socklen_t *fromlen);
```

### Description

Once you have a socket up and connected, you can read incoming data from the remote side using the
`recv()` (for TCP `SOCK_STREAM` sockets) and `recvfrom()` (for UDP `SOCK_DGRAM` sockets).

Both functions take the socket descriptor `s`, a pointer to the buffer `buf`, the size (in bytes) of the
buffer `len`, and a set of `flags` that control how the functions work.

Additionally, the `recvfrom()` takes a `struct sockaddr*`, `from` that will tell you where the data came
from, and will fill in `fromlen` with the size of `struct sockaddr`. (You must also initialize `fromlen` to
be the size of `from` or `struct sockaddr`.)

| Macro | Description |
|-------|-------------|
| `MSG_OOB` | Receive Out of Band data. This is how to get data that has been sent to you with the `MSG_OOB` flag in `send()`. As the receiving side, you will have had signal `SIGURG` raised telling you there is urgent data. In your handler for that signal, you could call `recv()` with this `MSG_OOB` flag. |
| `MSG_PEEK` | If you want to call `recv()` "just for pretend", you can call it with this flag. This will tell you what's waiting in the buffer for when you call `recv()` "for real" (i.e. without the `MSG_PEEK` flag.) It's like a sneak preview into the next `recv()` call. |
| `MSG_WAITALL` | Tell `recv()` to not return until all the data you specified in the `len` parameter has been received. It will ignore your wishes in extreme circumstances, however, like if a signal interrupts the call or if some error occurs or if the remote side closes the connection, etc. Don't be mad with it. |

When you call `recv()`, it will block until there is some data to read. If you want to not block, set the
socket to non-blocking or check with `select()` or `poll()` to see if there is incoming data before calling
`recv()` or `recvfrom()`.

### Return Value

Returns the number of bytes actually received (which might be less than you requested in the `len`
parameter), or `-1` on error (and `errno` will be set accordingly).

If the remote side has closed the connection, `recv()` will return `0`. This is the normal method for
determining if the remote side has closed the connection. Normality is good, rebel!

### Example

```c
// stream sockets and recv()

struct addrinfo hints, *res;
int sockfd;
char buf[512];
int byte_count;

// get host info, make socket, and connect it
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
hints.ai_socktype = SOCK_STREAM;
getaddrinfo("www.example.com", "3490", &hints, &res);
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
connect(sockfd, res->ai_addr, res->ai_addrlen);

// all right! now that we're connected, we can receive some data!
byte_count = recv(sockfd, buf, sizeof buf, 0);
printf("recv()'d %d bytes of data in buf\n", byte_count);
```

```c
// datagram sockets and recvfrom()

struct addrinfo hints, *res;
int sockfd;
int byte_count;
socklen_t fromlen;
struct sockaddr_storage addr;
char buf[512];
char ipstr[INET6_ADDRSTRLEN];

// get host info, make socket, bind it to port 4950
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
hints.ai_socktype = SOCK_DGRAM;
hints.ai_flags = AI_PASSIVE;
getaddrinfo(NULL, "4950", &hints, &res);
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
bind(sockfd, res->ai_addr, res->ai_addrlen);

// no need to accept(), just recvfrom():

fromlen = sizeof addr;
byte_count = recvfrom(sockfd, buf, sizeof buf, 0, &addr, &fromlen);

printf("recv()'d %d bytes of data in buf\n", byte_count);
printf("from IP address %s\n",
    inet_ntop(addr.ss_family,
        addr.ss_family == AF_INET?
            (void*)((struct sockaddr_in *)&addr)->sin_addr:
            (void*)((struct sockaddr_in6 *)&addr)->sin6_addr,
        ipstr, sizeof ipstr));
```

### See Also

`send()`, `sendto()`, `select()`, `poll()`, Blocking

[↑ Back to top](#table-des-matieres)

---

## `close()`

Close a socket descriptor

### Synopsis

```c
#include <unistd.h>

int close(int s);
```

### Description

After you've finished using the socket, you can simply close it using the regular Unix file descriptor
`close()` function. This will prevent any more reads and writes to the socket. Anyone attempting to read
or write the socket on the remote end will receive an error.

### Return Value

- **Success:** `0`
- **Failure:** `-1`, `errno` set accordingly

### Example

```c
s = socket(PF_INET, SOCK_DGRAM, 0);

// ...do some stuff...

close(s);
```

### See Also

`shutdown()`

[↑ Back to top](#table-des-matieres)

---

## `fcntl()`

Control socket descriptors

### Synopsis

```c
#include <sys/unistd.h>
#include <sys/fcntl.h>

int fcntl(int s, int cmd, ...);
```

### Description

This function is typically used to do file locking and other file-oriented stuff, but it also has a couple
socket-related functions that you might see or use from time to time.

Three of the most common `cmd` options for sockets are:

| `cmd` | Description |
|-------|-------------|
| `F_SETFL` | Set the file descriptor flags. Used with `O_NONBLOCK` to set a socket non-blocking. |
| `F_GETFL` | Get the file descriptor flags. |
| `F_SETFD` | Set the file descriptor flags including `FD_CLOEXEC` (close on exec). |

To set a socket to non-blocking:

```c
fcntl(sockfd, F_SETFL, O_NONBLOCK);
```

To set a socket back to blocking:

```c
fcntl(sockfd, F_SETFL, 0);
```

If you try to read from a non-blocking socket and there's no data there, it will return `-1` and `errno`
will be set to `EAGAIN` or `EWOULDBLOCK`.

### Return Value

- **Success:** depends on `cmd` (often `0`)
- **Failure:** `-1`, `errno` set accordingly

### Example

```c
#include <unistd.h>
#include <fcntl.h>

// ...

sockfd = socket(PF_INET, SOCK_STREAM, 0);
fcntl(sockfd, F_SETFL, O_NONBLOCK);

// ...
```

### See Also

Blocking

[↑ Back to top](#table-des-matieres)

---

## man: `poll()`

### Purpose

Test for events on multiple sockets simultaneously.

### Synopsis

```cpp
#include <sys/poll.h>
int poll(struct pollfd *ufds, unsigned int nfds, int timeout);
```

### Description

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

### Event macros (input in events)

| Macro | Description |
|---|---|
| **POLLIN** | Alert when data is ready to **recv()** on this socket. |
| **POLLOUT** | Alert when you can **send()** data without blocking. |
| **POLLPRI** | Alert when out-of-band data is ready to **recv()**. |

After **poll()** returns, **revents** contains a bitwise-OR of events that actually occurred.

### Additional flags (output in revents)

| Macro | Description |
|---|---|
| **POLLERR** | An error occurred on this socket. |
| **POLLHUP** | The remote side hung up. |
| **POLLNVAL** | Invalid descriptor (for example uninitialized **fd**). |

### Return Value

- Number of elements in **ufds** that have events
- **0** if timeout occurred
- **-1** on error (**errno** is set)

### Example

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

### See Also

`select()`, `fcntl()`

----------------------------

## man: `select()`

### Prototype

```cpp
#include <sys/select.h>

int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

FD_SET(int fd, fd_set *set);
FD_CLR(int fd, fd_set *set);
FD_ISSET(int fd, fd_set *set);
FD_ZERO(fd_set *set);
```

### Description

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

### Helper macros

| Macro | Description |
|---|---|
| **FD_SET(int fd, fd_set *set);** | Add **fd** to the set. |
| **FD_CLR(int fd, fd_set *set);** | Remove **fd** from the set. |
| **FD_ISSET(int fd, fd_set *set);** | Return true if **fd** is in the set. |
| **FD_ZERO(fd_set *set);** | Clear all entries from the set. |

### Note for Linux users

Linux **select()** can return ready-to-read and then not actually be ready to read, causing the subsequent **read()** call to block.

A common workaround is to set **O_NONBLOCK** on the receiving socket so that **read()** fails with **EWOULDBLOCK**. If that happens, ignore this specific error and continue.

See **fcntl()** man page for details on non-blocking mode.

### Return Value

- Number of ready descriptors on success
- **0** if timeout was reached
- **-1** on error (**errno** is set)

The descriptor sets are modified in-place to indicate readiness.

### Example

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

[↑ Back to top](#table-des-matieres)

------------------------------------------

## byte-order: htons(), htonl(), ntohs(), ntohl()

Convert multi-byte integer types from host byte order to network byte order.

### Synopsis

```cpp
#include <netinet/in.h>

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
```

### Description

Just to make you really unhappy, different computers use different byte orderings internally for their
multibyte integers (i.e. any integer that's larger than a char). The upshot of this is that if you send() a
two-byte short int from an Intel box to a Mac (before they became Intel boxes, too, I mean), what one
computer thinks is the number 1, the other will think is the number 256, and vice-versa.

The way to get around this problem is for everyone to put aside their differences and agree that Motorola
and IBM had it right, and Intel did it the weird way, and so we all convert our byte orderings to "big-endian"
before sending them out. Since Intel is a "little-endian" machine, it's far more politically correct to call our
preferred byte ordering "Network Byte Order". So these functions convert from your native byte order to
network byte order and back again.

(This means on Intel these functions swap all the bytes around, and on PowerPC they do nothing because
the bytes are already in Network Byte Order. But you should always use them in your code anyway, since
someone might want to build it on an Intel machine and still have things work properly.)

Note that the types involved are 32-bit (4 byte, probably int) and 16-bit (2 byte, very likely short)
numbers.

There are 64-bit variants on various systems. Check out the htobe64() function and its relatives in
<endian.h> if you have it (which apparently MacOS doesn't). And GCC has byte swapping built-ins
that even go up to 128 bits. Or you can roll your own, but only actually do the swap if you're on a
little-endian machine.

Anyway, the way these functions work is that you first decide if you're converting from host (your ma-
chine's) byte order or from network byte order. If "host", then the first letter of the function you're going to
call is "h". Otherwise it's "n" for "network". The middle of the function name is always "to" because you're
converting from one "to" another, and the penultimate letter shows what you're converting to. The last
letter is the size of the data, "s" for short, or "l" for long. Thus:

| Function | Description |
|---|---|
| htons() | h ost to n etwork s hort |
| htonl() | h ost to n etwork l ong |
| ntohs() | n etwork to h ost s hort |
| ntohl() | n etwork to h ost l ong |

### Return Value

Each function returns the converted value.

[↑ Back to top](#table-des-matieres)

-----------------------------


## man: `epoll`

`epoll` is how you watch hundreds of sockets at once without melting your CPU.
The old way (`select`, `poll`) looped through every fd every time. `epoll` only
tells you about the ones that are actually ready.

---

# The three calls you need

## epoll_create1

```c
int epfd = epoll_create1(0);
```

Creates the epoll object. Think of it as a list you're going to fill with fds
to watch. Returns a fd itself — close it when you're done.

---

## epoll_ctl

```c
epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
```

Adds, changes, or removes an fd from your watch list.

**Second argument:**

- `EPOLL_CTL_ADD` — start watching this fd
- `EPOLL_CTL_MOD` — change what you're watching for
- `EPOLL_CTL_DEL` — stop watching it

**The event struct:**

```c
struct epoll_event ev;
ev.events  = EPOLLIN;   /* tell me when there's data to read */
ev.data.fd = fd;        /* this comes back to you in epoll_wait */
```

Common flags for `events`:

- `EPOLLIN` — ready to read
- `EPOLLOUT` — ready to write
- `EPOLLET` — edge-triggered (see below)

epoll_event struct
 
This is what the kernel gives you and what you fill before calling `epoll_ctl`.
 
```c
typedef union epoll_data {
    void     *ptr;  /* pointer to your own struct with connection info */
    int       fd;   /* simplest — just store the fd directly */
    uint32_t  u32;
    uint64_t  u64;
} epoll_data_t;
 
struct epoll_event {
    uint32_t     events;  /* bitmask: EPOLLIN, EPOLLOUT, EPOLLET... */
    epoll_data_t data;    /* comes back to you unchanged in epoll_wait */
};
```
 
### events bitmask
 
```c
EPOLLIN       /* ready to read */
EPOLLOUT      /* ready to write */
EPOLLERR      /* error on fd (always watched, even if not set) */
EPOLLHUP      /* hangup (client disconnected) */
EPOLLET       /* edge-triggered mode */
EPOLLONESHOT  /* fire once, re-arm manually with EPOLL_CTL_MOD */
```
 
### data union — pick one
 
```c
ev.data.fd  = client_fd;   /* use this for simple cases */
ev.data.ptr = &my_struct;  /* use this when you need more context */
```
 
You only use one member of the union at a time. Most of the time `data.fd` is enough.
 
### Full usage
 
```c
struct epoll_event ev;
 
ev.events  = EPOLLIN | EPOLLET;
ev.data.fd = client_fd;
 
epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
```

---

## epoll_wait

```c
int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
```

Blocks until something happens. Returns how many fds are ready.
The kernel fills your `events` array with only the ready ones — no scanning.

- timeout `-1` = wait forever
- timeout `0`  = check and return immediately
- timeout `N`  = wait N milliseconds

---

### Putting it together

```c
#define MAX_EVENTS 64

int epfd = epoll_create1(0);

struct epoll_event ev;
ev.events  = EPOLLIN;
ev.data.fd = server_fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

struct epoll_event events[MAX_EVENTS];

while (1)
{
    int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
    if (n == -1)
    {
        if (errno == EINTR) continue; /* signal interrupted us, just retry */
        perror("epoll_wait");
        break;
    }
    for (int i = 0; i < n; i++)
    {
        if (events[i].events & EPOLLIN)
            handle_read(events[i].data.fd);
        if (events[i].events & EPOLLOUT)
            handle_write(events[i].data.fd);
    }
}
```

---

## Level-triggered vs edge-triggered

By default epoll is **level-triggered**: as long as there's unread data in the
buffer, `epoll_wait` keeps telling you about it on every call. Safe and simple.

With `EPOLLET` (edge-triggered) it only tells you once — when new data arrives.
If you don't read everything in one shot, you'll never hear about it again.

For webserv, stick with level-triggered. Edge-triggered is faster but you need
non-blocking fds and a full drain loop to not lose data.

---

## Things that will bite you

- Always set `O_NONBLOCK` on your fds before adding them to epoll.
- `epoll_wait` returns `-1` when a signal hits — check `errno == EINTR` and retry.
- `data` in the event struct is a union. Use `data.fd` to store the fd or
  `data.ptr` to point to a struct with more context about the connection.

---

[↑ Back to top](#table-des-matieres)
