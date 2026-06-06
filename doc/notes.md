# Notes - Webserv (42)

## Table des matieres
- [I/O non-bloquante](#io-non-bloquante)
- [poll](#poll)
- [select](#select)
- [epoll](#epoll)
- [HTTP protocol](#http-protocol)
- [Sockets en C/C++](#sockets-en-cc)
- [CGI](#cgi)
- [Parsing de config type NGINX](#parsing-de-config-type-nginx)
- [Architecture generale d'un serveur](#architecture-generale-dun-serveur)

---
## Overview

### Web Request Flow
1. Entering the URL
1. DNS Resolution
1. Establishing the Connection (TCP, 3 way handshake, SYN/ACK)
1. Sending the HTTP Request
1. **Server Receives the Request**(Apache or Nginx direct the request)
1. **Backend Processing (optional)**
1. **Generating the Response**
1. **Sending the HTTP Response**
1. Receiving the Response
1. HTML Parsing
1. Rendering Engine
1. GPU Processing and Display

I think we only have to manage from 5 (or 3) to 8 but we have to verify if there is some check that we have to do.

But we have to do the job of Apache or Nginx (web server software).

## IP addresses


### Key Notions
- Thank's to netpractice we master ip adresses. But to remind, there is the ipv4 standard (ex: 192.0.2.111) and ipv6 standard (ex: 2001:0db8:c9d2:aee5:73e3:934a:a5ae:9551). Some are blocked (like ::1 in ipv6 for the loopback). Doing subnet can be convenient, so we put a network portion and a host portion.

- Some computers are in little endian and other in big endian. For exemple 3a4f can be store ase 3a followed by 4f (little endian) or 4f followed by 3a (big endian). Big endian is also called Network Byte Order because that’s the order us network types like.

- There are two types of numbers that you can convert: short (two bytes) and long (four bytes). These functions work for the unsigned variations as well. Say you want to convert a short from Host Byte Order to Network Byte Order. Start with “h” for “host”, follow it with “to”, then “n” for “network”, and “s” for “short”: h-to-n-s, or htons() (read: “Host to Network Short”). Here are the usefull functions `htons`, `htonl`, `ntohs`, `ntohl`.
- Full man page: [htons(), htonl(), ntohs(), ntohl()](man/byte-order.md)

- Maybe not relevant here but there is sometimes privates network, all the ip adresses are behind another one given to the public and the firewall will convert.

### Points a maitriser pour webserv
- je crois que les problemes d'ipv4 et ipv6 sont gérés par `getaddrinfo`. Si ce n'est pas le cas voir https://beej.us/guide/bgnet/ chap3.
- We will have to be careful to use the convert functions when we send or receive.

### Pitfalls frequents
- 

### Ressources utiles
- https://beej.us/guide/bgnet/

## poll, select, epoll and non-blocking probleme
### non-blocking

- Most system call are blocking (**Synchronous Calls**), they do they have to do and when it's over they return, like our usual functions.

- Here we want to be able to listen to many connection, and if one has a really big paquet to send we don't want to block the other ones (not sure but I think we want to do like a loop through connections and we take each time a little bit from each connection). So we need **Non-Blocking Cals Asynchronous**, it does something in background and call an event when the task is completed. It's not a multithreading !


### select() system call
- Select take files descriptors and tell us when one of them is ready for reading.
- Full man page: [select()](man/select.md)

### poll() system call
- Same than select, but they manage fd differently and it has a more flexible limit. (It just seems to be better but we should read more about that).
- Full man page: [poll()](man/poll.md)

### epoll system call
- Linux event API similar to poll, usually better for many simultaneous connections. (Even better maybe ? But linux only...)
- Full man page: [epoll](man/epoll.md)


### Ressources utiles
- select: https://www.youtube.com/watch?v=Y6pFtgRdUts
- poll: 
---

## Sockets in C/C++
### Key Notions
*Everything in Unix is a file*

- When Unix programs do any sort of I/O, they do it by reading or writing to a file descriptor. A file descriptor is simply an integer associated with an open file. But (and here’s the catch), that file can be a network connection, a FIFO, a pipe, a terminal, a real on-the-disk file, or just about anything else. 
- In order to get this file descriptor for network connection, we make a call to the `socket()` system routine. It returns the socket descriptor, and you communicate through it using the specialized `send()` and `recv()` socket calls. They offer much greater control over the data transmission than read() and write().
- Full man page: [socket()](man/socket.md), send, recv


### Attention points
- 

### Ressources utiles
- 

---
## HTTP Protocol

### Key Notions


---

## CGI (Common Gateway Interface)

### Key Notions
- CGI is one method by which a web server can obtain data from (or send data to) databases, documents, and other programs, and present that data to viewers via the web. More simply, a CGI is a program intended to be run on the web. A CGI program can be written in any programming language, it could be a script or a binary file.
- The more common used are PHP, Bash, Perl, Ruby, Python, C or C++. Python or PHP seems to be the easiest ones, but it's up to us want what we want to learn.
- Maybe we don't have to write the script but be able to support a .php CGI. We need to go deeper in that.

### Points a maitriser pour webserv
- 

### Pitfalls frequents
- 

### Ressources
- Web Request Flow https://www.youtube.com/watch?v=hWyBeEF3CqQ

---

## Parsing de config type NGINX
### Objectif

### Notions cles
- Blocs (`http`, `server`, `location`)
- Directives et heritage
- Validation syntaxique et semantique

### Points a maitriser pour webserv
- 

### Pitfalls frequents
- 

### Ressources utiles
- 

---

## Architecture generale d'un serveur
### Objectif

### Composants principaux
- Boucle d'evenements
- Gestion des connexions
- Parser HTTP
- Routage vers ressources/fichiers/CGI
- Construction de la reponse
- Gestion des erreurs
- Logging

### Points a maitriser pour webserv
- 

### Pitfalls frequents
- 

### Ressources utiles
- 

---

## Checklist de progression
- [ ] I/O non-bloquante
- [ ] poll
- [ ] select
- [ ] epoll
- [ ] HTTP protocol
- [ ] Sockets en C/C++
- [ ] CGI
- [ ] Parsing de config type NGINX
- [ ] Architecture generale d'un serveur
