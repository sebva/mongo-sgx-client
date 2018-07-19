#include "my_wrappers_impl.h"

#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stropts.h>
#include <cstdio>

void* ocall_malloc(size_t size) {
	return malloc(size);
}

void ocall_free(void *ptr) {
	free(ptr);
}

int ocall_access(const char *path, int amode) {
	return access(path, amode);
}

int ocall_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
	return getaddrinfo(node, service, hints, res);
}

void ocall_freeaddrinfo(struct addrinfo *res) {
	return freeaddrinfo(res);
}

int ocall_fcntl(int fildes, int cmd, int arg) {
	// WARNING: only works with the cmd that mongoc uses!
	return fcntl(fildes, cmd, arg);
}

int ocall_poll(struct pollfd fds[], nfds_t nfds, int timeout) {
	return poll(fds, nfds, timeout);
}

int ocall_getsockopt(int socket, int level, int option_name, void *option_value, socklen_t *option_len) {
	return getsockopt(socket, level, option_name, option_value, option_len);
}

int ocall_setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len) {
	return setsockopt(socket, level, option_name, option_value, option_len);
}

int ocall_accept(int socket, struct sockaddr *address, socklen_t *address_len) {
	return accept(socket, address, address_len);
}

int ocall_close(int fildes) {
	return close(fildes);
}

uint16_t ocall_ntohs(uint16_t netshort) {
	return ntohs(netshort);
}

int ocall_bind(int socket, const struct sockaddr *address, socklen_t address_len) {
	return bind(socket, address, address_len);
}

int ocall_shutdown(int socket, int how) {
	return shutdown(socket, how);
}

int ocall_connect(int socket, const struct sockaddr *address, socklen_t address_len) {
	return connect(socket, address, address_len);
}

int ocall_listen(int socket, int backlog) {
	return listen(socket, backlog);
}

int ocall_socket(int domain, int type, int protocol) {
	return socket(domain, type, protocol);
}

ssize_t ocall_recv(int socket, void *buffer, size_t length, int flags) {
	return recv(socket, buffer, length, flags);
}

ssize_t ocall_send(int socket, const void *buffer, size_t length, int flags) {
	return send(socket, buffer, length, flags);
}

ssize_t ocall_sendmsg(int socket, const struct msghdr *message, size_t message_len, int flags) {
	return sendmsg(socket, message, flags);
}

struct servent *ocall_getservbyname(const char *name, const char *proto) {
	return getservbyname(name, proto);
}

struct hostent *ocall_gethostbyname(const char *name) {
	return gethostbyname(name);
}

int ocall_getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags) {
	return getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);
}

int ocall_getpeername(int socket, struct sockaddr *address, socklen_t *address_len) {
	return getpeername(socket, address, address_len);
}

int ocall_ioctl(int fildes, int request, ... /* arg */) {
	return ioctl(fildes, request);
}

ssize_t ocall_read(int fildes, void *buf, size_t nbyte) {
	return read(fildes, buf, nbyte);
}

int ocall_open(const char *path, int oflag, ...) {
	return open(path, oflag);
}

int ocall_gethostname(char *name, size_t namelen) {
	return gethostname(name, namelen);
}

pid_t ocall_getpid(void) {
	return getpid();
}

int ocall_usleep(useconds_t usec) {
	return usleep(usec);
}
