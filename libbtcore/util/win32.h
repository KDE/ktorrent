/*
Copyright (c) 2006 by Dan Kennedy.
Copyright (c) 2006 by Juliusz Chroboczek.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/* 
 * Polipo was originally designed to run on Unix-like systems. This
 * header file (and it's accompanying implementation file mingw.c) contain
 * code that allows polipo to run on Microsoft Windows too. 
 *
 * The target MS windows compiler is Mingw (MINimal Gnu for Windows). The
 * code in this file probably get's us pretty close to MSVC also, but
 * this has not been tested. To build polipo for Mingw, define the MINGW
 * symbol. For Unix or Unix-like systems, leave it undefined.
 */
#ifndef MINGW_H
#define MINGW_H
/* Unfortunately, there's no hiding it. */
// #define HAVE_WINSOCK 1

/* At time of writing, a fair bit of stuff doesn't work under Mingw.
 * Hopefully they will be fixed later (especially the disk-cache).
 */

#include <io.h>
#include <wchar.h>
#include <btcore_export.h>
/* Pull in winsock.h for (almost) berkeley sockets. */
#ifdef FD_SETSIZE
#undef FD_SETSIZE
#endif
#define FD_SETSIZE 1000
#include <winsock2.h>
// #define ENOTCONN        WSAENOTCONN
// #define EWOULDBLOCK     WSAEWOULDBLOCK
// #define ENOBUFS         WSAENOBUFS
// #define ECONNRESET      WSAECONNRESET
// #define ESHUTDOWN       WSAESHUTDOWN
// #define EAFNOSUPPORT    WSAEAFNOSUPPORT
// #define EPROTONOSUPPORT WSAEPROTONOSUPPORT
// #define EINPROGRESS     WSAEINPROGRESS
// #define EISCONN         WSAEISCONN

#if ( ( defined(_WIN32_WINNT) && (_WIN32_WINNT < 0x0600) ) || ( defined(_MSC_VER) && (_MSC_VER < 1500 ) ) ) // If VC++ 8.0 or older OR Windows older than Vista

/* winsock doesn't feature poll(), so there is a version implemented
 * in terms of select() in win32.cpp. The following definitions
 * are copied from linux man pages. A poll() macro is defined to
 * call the version in win32.cpp.
 * pollfd is defined in Windows SDK 6.0A and newer if using 
 * MSVC2008, in what seems to be a blatant bug (MSVC2008 reports
 * Windows XP as 0x0600 instead of 0x501)
 */
#define POLLIN      0x0001    /* There is data to read */
#define POLLPRI     0x0002    /* There is urgent data to read */
#define POLLOUT     0x0004    /* Writing now will not block */
#define POLLERR     0x0008    /* Error condition */
#define POLLHUP     0x0010    /* Hung up */
#define POLLNVAL    0x0020    /* Invalid request: fd not open */

struct BTCORE_EXPORT pollfd {
    SOCKET fd;        /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
};
// #define poll(x, y, z)        mingw_poll(x, y, z)
#endif

#ifndef NAME_MAX
#define NAME_MAX            255
#endif
/* These wrappers do nothing special except set the global errno variable if
* an error occurs (winsock doesn't do this by default). They set errno
* to unix-like values (i.e. WSAEWOULDBLOCK is mapped to EAGAIN), so code
* outside of this file "shouldn't" have to worry about winsock specific error
* handling.
*/
// #define socket(x, y, z)      mingw_socket(x, y, z)
// #define connect(x, y, z)     mingw_connect(x, y, z)
// #define accept(x, y, z)      mingw_accept(x, y, z)
// #define shutdown(x, y)       mingw_shutdown(x, y)
// #define getpeername(x, y, z) mingw_getpeername(x, y, z)

/* Wrapper macros to call misc. functions mingw is missing */
// #define sleep(x)             mingw_sleep(x)
// #define inet_aton(x, y)      mingw_inet_aton(x, y)
// #define gettimeofday(x, y)   mingw_gettimeofday(x, y)
// #define stat(x, y)           mingw_stat(x, y)
// 
// #define mkdir(x, y) mkdir(x)

/* Winsock uses int instead of the usual socklen_t */
// typedef int socklen_t;

/* Function prototypes for functions in mingw.c */
// unsigned int mingw_sleep(unsigned int);
// int     mingw_inet_aton(const char *, struct in_addr *);
// int     mingw_gettimeofday(struct timeval *, char *);
BTCORE_EXPORT int     mingw_poll(struct pollfd *, unsigned int, int);
// SOCKET  mingw_socket(int, int, int);
// int     mingw_connect(SOCKET, struct sockaddr*, socklen_t);
// SOCKET  mingw_accept(SOCKET, struct sockaddr*, socklen_t *);
// int     mingw_shutdown(SOCKET, int);
// int     mingw_getpeername(SOCKET, struct sockaddr*, socklen_t *);

/* Three socket specific macros */
// #define READ(x, y, z)  mingw_read_socket(x, y, z)
// #define WRITE(x, y, z) mingw_write_socket(x, y, z)
// #define CLOSE(x)       mingw_close_socket(x)
// 
// int mingw_read_socket(SOCKET, void *, int);
// int mingw_write_socket(SOCKET, void *, int);
// int mingw_close_socket(SOCKET);
// 
// int mingw_setnonblocking(SOCKET, int);
// int mingw_stat(const char*, struct stat*);
#define strerror(e) mingw_strerror(e)

BTCORE_EXPORT char *mingw_strerror(int error);

#if 0
#ifdef POLLRDNORM
#undef POLLRDNORM
#undef POLLRDBAND
#undef POLLIN
#undef POLLPRI
#undef POLLWRNORM
#undef POLLOUT
#undef POLLWRBAND
#undef POLLERR
#undef POLLHUP
#undef POLLNVAL
struct _pollfd {
    SOCKET fd;
    short  events;
    short  revents;
};
#define pollfd _pollfd
#else
struct pollfd {
    SOCKET fd;
    short  events;
    short  revents;
};
#endif

typedef unsigned int nfds_t;

#define POLLIN     (FD_READ | FD_ACCEPT | FD_CLOSE)
#define POLLPRI    (FD_OOB)
#define POLLOUT    (FD_WRITE | FD_CONNECT | FD_CLOSE)
#define POLLRDHUP  (FD_CLOSE)
#define POLLHUP    (FD_CLOSE)
#define POLLRDNORM (POLLIN)
#define POLLRDBAND (POLLIN | POLLPRI)
#define POLLWRNORM (POLLOUT)
#define POLLWRBAND (POLLOUT | POLLPRI)
// POLLERR, POLLNVAL not defined

BTCORE_EXPORT int poll(struct pollfd *fds, nfds_t nfds, int timeout);

#define mingw_poll(a, b, c) poll(a, b, c)
#endif
#endif
