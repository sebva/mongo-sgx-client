#include "my_wrappers.h"

#include <string.h>
#include "mongoclient_t.h"

void *native_malloc(size_t size) {
	void* buffer;

	int sgx_retval = ocall_malloc(&buffer, size);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in malloc OCALL");
		sgx_exit();
	}

	if (!sgx_is_outside_enclave(buffer, size)) {
		printf("Rebinding attack detected! ABORT.");
		sgx_exit();
	}

	return buffer;
}

void native_free(void *ptr) {
	if (!sgx_is_outside_enclave(ptr, 1)) {
		printf("Trying to native free trusted memory.");
		sgx_exit();
	}
	int sgx_retval = ocall_free(ptr);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in free OCALL");
		sgx_exit();
	}
}

int access(const char *path, int amode) {
	int retval;

	int sgx_retval = ocall_access(&retval, path, amode);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in access OCALL");
		sgx_exit();
	}

	return retval;
}

int getaddrinfo(const char *node, const char *service,
		const struct addrinfo *hints, struct addrinfo **res) {
	int retval;

	int sgx_retval = ocall_getaddrinfo(&retval, node, service, hints, res);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in getaddrinfo OCALL");
		sgx_exit();
	}

	if (!sgx_is_outside_enclave(*res, sizeof(struct addrinfo))) {
		printf("Rebinding attack detected! ABORT.");
		sgx_exit();
	}

	return retval;
}

void freeaddrinfo(struct addrinfo *res) {
	int sgx_retval = ocall_freeaddrinfo(res);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in access OCALL");
		sgx_exit();
	}
}

int putchar(int c) {
	char string[] = { (char) c, '\0' };
	int ret;
	ocall_print_string(&ret, string);
	return ret;
}

int fcntl(int fildes, int cmd, ...) {
	va_list cmd_va;
	va_start(cmd_va, cmd);

	int cmd1, cmd2;

	for (int i = 0; i < cmd; i++) {
		int val = va_arg(cmd_va, int);
		if (i == 0) {
			cmd1 = val;
		} else if (i == 1) {
			cmd2 = val;
		}
	}
	va_end(cmd_va);

	int retval;
	int sgx_retval = ocall_fcntl(&retval, fildes, cmd1, cmd2);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in fcntl OCALL");
		sgx_exit();
	}
	return retval;
}

int poll(struct pollfd fds[], nfds_t nfds, int timeout) {
	int retval;

	int sgx_retval = ocall_poll(&retval, fds, nfds, timeout);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in poll OCALL");
		sgx_exit();
	}

	return retval;
}

int getsockopt(int socket, int level, int option_name, void *restrict option_value, socklen_t *restrict option_len) {
	int retval;

	void* native_option_value = native_malloc(*option_len);
	socklen_t* native_option_len = native_malloc(sizeof(socklen_t));

	memcpy(native_option_value, option_value, *option_len);
	*native_option_len = *option_len;

	int sgx_retval = ocall_getsockopt(&retval, socket, level, option_name, native_option_value, native_option_len);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in getsockopt OCALL");
		sgx_exit();
	}

	size_t final_len = *option_len;
	memcpy(option_value, native_option_value, final_len);
	*option_len = *native_option_len;

	native_free(native_option_value);
	native_free(native_option_len);

	return retval;
}

int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len) {
	int retval;

	int sgx_retval = ocall_setsockopt(&retval, socket, level, option_name, option_value, option_len);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in setsockopt OCALL");
		sgx_exit();
	}

	return retval;
}

int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len) {
	int retval;

	int sgx_retval = ocall_accept(&retval, socket, address, address_len);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in accept OCALL");
		sgx_exit();
	}

	return retval;
}

int close(int fildes) {
	int retval;

	int sgx_retval = ocall_close(&retval, fildes);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in close OCALL");
		sgx_exit();
	}

	return retval;
}

uint16_t ntohs(uint16_t netshort) {
	uint16_t retval;

	int sgx_retval = ocall_ntohs(&retval, netshort);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in ntohs OCALL");
		sgx_exit();
	}

	return retval;
}

int bind(int socket, const struct sockaddr *address, socklen_t address_len) {
	int retval;

	int sgx_retval = ocall_bind(&retval, socket, address, address_len);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in bind OCALL");
		sgx_exit();
	}

	return retval;
}

int shutdown(int socket, int how) {
	int retval;

	int sgx_retval = ocall_shutdown(&retval, socket, how);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in shutdown OCALL");
		sgx_exit();
	}

	return retval;
}

int connect(int socket, const struct sockaddr *address, socklen_t address_len) {
	int retval;

	int sgx_retval = ocall_connect(&retval, socket, address, address_len);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in connect OCALL");
		sgx_exit();
	}

	return retval;
}

int listen(int socket, int backlog) {
	int retval;

	int sgx_retval = ocall_listen(&retval, socket, backlog);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in listen OCALL");
		sgx_exit();
	}

	return retval;
}

int socket(int domain, int type, int protocol) {
	int retval;

	int sgx_retval = ocall_socket(&retval, domain, type, protocol);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in socket OCALL");
		sgx_exit();
	}

	return retval;
}

ssize_t recv(int socket, void *buffer, size_t length, int flags) {
	ssize_t retval;

	int sgx_retval = ocall_recv(&retval, socket, buffer, length, flags);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in recv OCALL");
		sgx_exit();
	}

	return retval;
}

ssize_t send(int socket, const void *buffer, size_t length, int flags) {
	ssize_t retval;

	int sgx_retval = ocall_send(&retval, socket, buffer, length, flags);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in send OCALL");
		sgx_exit();
	}

	return retval;
}

ssize_t sendmsg(int socket, const struct msghdr *message, int flags) {
	ssize_t retval;

	int sgx_retval = ocall_sendmsg(&retval, socket, message, flags);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in sendmsg OCALL");
		sgx_exit();
	}

	return retval;
}

struct servent *getservbyname(const char *name, const char *proto) {

}

struct hostent *gethostbyname(const char *name) {
	struct hostent *retval;

	int sgx_retval = ocall_gethostbyname(&retval, socket, name);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in gethostbyname OCALL");
		sgx_exit();
	}

	return retval;
}

int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags) {

}

int getpeername(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len) {

}

int ioctl(int fildes, int request, ... /* arg */) {

}

ssize_t read(int fildes, void *buf, size_t nbyte) {
	ssize_t retval;

	int sgx_retval = ocall_read(&retval, fildes, buf, nbyte);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in read OCALL");
		sgx_exit();
	}

	return retval;
}

int open(const char *path, int oflag, ...) {

}

int uname(struct utsname *name) {
	strncpy(name->machine, "SGX", 4);
	strncpy(name->nodename, "SGX", 4);
	strncpy(name->release, "SGX", 4);
	strncpy(name->sysname, "SGX", 4);
	strncpy(name->version, "SGX", 4);
	return 0;
}

pid_t getpid(void) {
	return 42;
}

int gethostname(char *name, size_t namelen) {
	strncpy(name, "SGX", 4);
	return 0;
}

/* Return the value of CH as a hexademical digit, or -1 if it is a
 different type of character.  */
static int hex_digit_value(char ch) {
	if ('0' <= ch && ch <= '9')
		return ch - '0';
	if ('a' <= ch && ch <= 'f')
		return ch - 'a' + 10;
	if ('A' <= ch && ch <= 'F')
		return ch - 'A' + 10;
	return -1;
}

static int inet_pton4(const char *src, const char *end, unsigned char *dst) {
	int saw_digit, octets, ch;
	unsigned char tmp[NS_INADDRSZ], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while (src < end) {
		ch = *src++;
		if (ch >= '0' && ch <= '9') {
			unsigned int new = *tp * 10 + (ch - '0');

			if (saw_digit && *tp == 0)
				return 0;
			if (new > 255)
				return 0;
			*tp = new;
			if (!saw_digit) {
				if (++octets > 4)
					return 0;
				saw_digit = 1;
			}
		} else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return 0;
			*++tp = 0;
			saw_digit = 0;
		} else
			return 0;
	}
	if (octets < 4)
		return 0;
	memcpy(dst, tmp, NS_INADDRSZ);
	return 1;
}

/* Convert presentation-level IPv6 address to network order binary
 form.  Return 1 if SRC is a valid [RFC1884 2.2] address, else 0.
 This function does not touch DST unless it's returning 1.
 Author: Paul Vixie, 1996.  Inspired by Mark Andrews.  */
static int inet_pton6(const char *src, const char *src_endp, unsigned char *dst) {
	unsigned char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
	const char *curtok;
	int ch;
	size_t xdigits_seen; /* Number of hex digits since colon.  */
	unsigned int val;

	tp = memset(tmp, '\0', NS_IN6ADDRSZ);
	endp = tp + NS_IN6ADDRSZ;
	colonp = NULL;

	/* Leading :: requires some special handling.  */
	if (src == src_endp)
		return 0;
	if (*src == ':') {
		++src;
		if (src == src_endp || *src != ':')
			return 0;
	}

	curtok = src;
	xdigits_seen = 0;
	val = 0;
	while (src < src_endp) {
		ch = *src++;
		int digit = hex_digit_value(ch);
		if (digit >= 0) {
			if (xdigits_seen == 4)
				return 0;
			val <<= 4;
			val |= digit;
			if (val > 0xffff)
				return 0;
			++xdigits_seen;
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (xdigits_seen == 0) {
				if (colonp)
					return 0;
				colonp = tp;
				continue;
			} else if (src == src_endp)
				return 0;
			if (tp + NS_INT16SZ > endp)
				return 0;
			*tp++ = (unsigned char) (val >> 8) & 0xff;
			*tp++ = (unsigned char) val & 0xff;
			xdigits_seen = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + NS_INADDRSZ) <= endp)
				&& inet_pton4(curtok, src_endp, tp) > 0) {
			tp += NS_INADDRSZ;
			xdigits_seen = 0;
			break; /* '\0' was seen by inet_pton4.  */
		}
		return 0;
	}
	if (xdigits_seen > 0) {
		if (tp + NS_INT16SZ > endp)
			return 0;
		*tp++ = (unsigned char) (val >> 8) & 0xff;
		*tp++ = (unsigned char) val & 0xff;
	}
	if (colonp != NULL) {
		/* Replace :: with zeros.  */
		if (tp == endp)
			/* :: would expand to a zero-width field.  */
			return 0;
		size_t n = tp - colonp;
		memmove(endp - n, colonp, n);
		memset(colonp, 0, endp - n - colonp);
		tp = endp;
	}
	if (tp != endp)
		return 0;
	memcpy(dst, tmp, NS_IN6ADDRSZ);
	return 1;
}

int inet_pton(int af, const char *src, void *dst) {
	switch (af) {
	case AF_INET:
		return inet_pton4(src, src + strlen(src), dst);
	case AF_INET6:
		return inet_pton6(src, src + strlen(src), dst);
	default:
		return -1;
	}
}
