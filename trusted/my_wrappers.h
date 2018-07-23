/*
 * my_wrappers.h
 *
 *  Created on: 19 Jul 2018
 *      Author: sebastien
 */

#ifndef TRUSTED_MY_WRAPPERS_H_
#define TRUSTED_MY_WRAPPERS_H_

#include <sys/utsname.h>
#include "my_enclave_types.h"

#if defined(__cplusplus)
extern "C" {
#endif

// OCALLed
int printf(const char *fmt, ...);
void *native_malloc(size_t size);
void native_free(void *ptr);
int access(const char *path, int amode);
int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
void freeaddrinfo(struct addrinfo *res);
int fcntl(int fildes, int cmd, ...);
int poll(struct pollfd fds[], nfds_t nfds, int timeout);
int getsockopt(int socket, int level, int option_name, void *option_value, socklen_t *option_len);
int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
int accept(int socket, struct sockaddr *address, socklen_t *address_len);
int close(int fildes);
uint16_t ntohs(uint16_t netshort);
int bind(int socket, const struct sockaddr *address, socklen_t address_len);
int shutdown(int socket, int how);
int connect(int socket, const struct sockaddr *address, socklen_t address_len);
int listen(int socket, int backlog);
int socket(int domain, int type, int protocol);
ssize_t recv(int socket, void *buffer, size_t length, int flags);
ssize_t send(int socket, const void *buffer, size_t length, int flags);
ssize_t sendmsg(int socket, const struct msghdr *message, int flags);
struct servent *getservbyname(const char *name, const char *proto);
struct hostent *gethostbyname(const char *name);
int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags);
int getpeername(int socket, struct sockaddr *address, socklen_t *address_len);
int ioctl(int fildes, int request, ... /* arg */);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
ssize_t read(int fildes, void *buf, size_t nbyte);
int open(const char *path, int oflag, ...);
int gethostname(char *name, size_t namelen);
pid_t getpid(void);
int putchar(int c);
int sscanf(const char *str, const char *format, ...);
int getsockname(int socket, struct sockaddr *address, socklen_t *address_len);
long syscall(long number, ...);
int vsscanf(const char *str, const char *format, va_list ap);
int usleep(useconds_t usec);

// Served locally
int uname(struct utsname *name);
int SSL_CTX_set_default_verify_paths(void *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* TRUSTED_MY_WRAPPERS_H_ */
