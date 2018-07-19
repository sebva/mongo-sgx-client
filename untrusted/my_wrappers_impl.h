#ifndef _OCALL_IMPLEMENTS_H_
#define _OCALL_IMPLEMENTS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <cstdint>

#if defined(__cplusplus)
extern "C" {
#endif

	void* ocall_malloc(size_t size);
	void ocall_free(void *ptr);
	int ocall_access(const char *path, int amode);
	int ocall_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
	void ocall_freeaddrinfo(struct addrinfo *res);
	int ocall_fcntl(int fildes, int cmd1, int cmd2);
	int ocall_poll(struct pollfd fds[], nfds_t nfds, int timeout);
	int ocall_getsockopt(int socket, int level, int option_name, void *option_value, socklen_t *option_len);
	int ocall_setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
	int ocall_accept(int socket, struct sockaddr *address, socklen_t *address_len);
	int ocall_close(int fildes);
	uint16_t ocall_ntohs(uint16_t netshort);
	int ocall_bind(int socket, const struct sockaddr *address, socklen_t address_len);
	int ocall_shutdown(int socket, int how);
	int ocall_connect(int socket, const struct sockaddr *address, socklen_t address_len);
	int ocall_listen(int socket, int backlog);
	int ocall_socket(int domain, int type, int protocol);
	ssize_t ocall_recv(int socket, void *buffer, size_t length, int flags);
	ssize_t ocall_send(int socket, const void *buffer, size_t length, int flags);
	ssize_t ocall_sendmsg(int socket, const struct msghdr *message, size_t message_len, int flags);
	struct servent *ocall_getservbyname(const char *name, const char *proto);
	struct hostent *ocall_gethostbyname(const char *name);
	int ocall_getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags);
	int ocall_getpeername(int socket, struct sockaddr *address, socklen_t *address_len);
	int ocall_ioctl(int fildes, int request, ... /* arg */);
	ssize_t ocall_read(int fildes, void *buf, size_t nbyte);
	int ocall_open(const char *path, int oflag, ...);
	int ocall_gethostname(char *name, size_t namelen);
	pid_t ocall_getpid(void);

#if defined(__cplusplus)
}
#endif

#endif /* !_OCALL_IMPLEMENTS_H_ */
