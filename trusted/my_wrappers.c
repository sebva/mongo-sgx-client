#include "my_wrappers.h"

#include <string.h>
#include "mongoclient_t.h"

void *native_malloc(size_t size) {
	void* buffer;

	int sgx_retval = ocall_malloc(&buffer, size);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in malloc OCALL\n");
		sgx_exit();
	}

	if (!sgx_is_outside_enclave(buffer, size)) {
		printf("Rebinding attack detected! ABORT.\n");
		sgx_exit();
	}

	return buffer;
}

void native_free(void *ptr) {
	if (!sgx_is_outside_enclave(ptr, 1)) {
		printf("Trying to native free trusted memory.\n");
		sgx_exit();
	}
	int sgx_retval = ocall_free(ptr);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in free OCALL\n");
		sgx_exit();
	}
}

int access(const char *path, int amode) {
	int retval;

	int sgx_retval = ocall_access(&retval, path, amode);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in access OCALL\n");
		sgx_exit();
	}

	return retval;
}

int getaddrinfo(const char *node, const char *service,
		const struct addrinfo *hints, struct addrinfo **res) {
	int retval;

	int sgx_retval = ocall_getaddrinfo(&retval, node, service, hints, res);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in getaddrinfo OCALL\n");
		sgx_exit();
	}

	if (!sgx_is_outside_enclave(*res, sizeof(struct addrinfo))) {
		printf("Rebinding attack detected! ABORT.\n");
		sgx_exit();
	}

	return retval;
}

void freeaddrinfo(struct addrinfo *res) {
	int sgx_retval = ocall_freeaddrinfo(res);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in access OCALL\n");
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
	va_list ap;
	va_start(ap, cmd);
	int arg = va_arg(ap, int);
	va_end(ap);

	int retval;
	int sgx_retval = ocall_fcntl(&retval, fildes, cmd, arg);

	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in fcntl OCALL\n");
		sgx_exit();
	}
	return retval;
}

int poll(struct pollfd fds[], nfds_t nfds, int timeout) {
	int retval;

	int sgx_retval = ocall_poll(&retval, fds, nfds, timeout);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in poll OCALL\n");
		sgx_exit();
	}

	return retval;
}

int getsockopt(int socket, int level, int option_name,
		void *restrict option_value, socklen_t *restrict option_len) {
	int retval;

	void* native_option_value = native_malloc(*option_len);
	socklen_t* native_option_len = native_malloc(sizeof(socklen_t));

	memcpy(native_option_value, option_value, *option_len);
	*native_option_len = *option_len;

	int sgx_retval = ocall_getsockopt(&retval, socket, level, option_name,
			native_option_value, native_option_len);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in getsockopt OCALL\n");
		sgx_exit();
	}

	size_t final_len = *option_len;
	memcpy(option_value, native_option_value, final_len);
	*option_len = *native_option_len;

	native_free(native_option_value);
	native_free(native_option_len);

	return retval;
}

int setsockopt(int socket, int level, int option_name, const void *option_value,
		socklen_t option_len) {
	int retval;

	int sgx_retval = ocall_setsockopt(&retval, socket, level, option_name,
			option_value, option_len);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in setsockopt OCALL\n");
		sgx_exit();
	}

	return retval;
}

int accept(int socket, struct sockaddr *restrict address,
		socklen_t *restrict address_len) {
	int retval;

	int sgx_retval = ocall_accept(&retval, socket, address, address_len);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in accept OCALL\n");
		sgx_exit();
	}

	if (!sgx_is_outside_enclave(address, sizeof(struct addrinfo))) {
		printf("Rebinding attack detected! ABORT.\n");
		sgx_exit();
	}

	return retval;
}

int close(int fildes) {
	int retval;

	int sgx_retval = ocall_close(&retval, fildes);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in close OCALL\n");
		sgx_exit();
	}

	return retval;
}

uint16_t ntohs(uint16_t netshort) {
	uint16_t retval;

	int sgx_retval = ocall_ntohs(&retval, netshort);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in ntohs OCALL\n");
		sgx_exit();
	}

	return retval;
}

int bind(int socket, const struct sockaddr *address, socklen_t address_len) {
	int retval;

	int sgx_retval = ocall_bind(&retval, socket, address, address_len);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in bind OCALL\n");
		sgx_exit();
	}

	return retval;
}

int shutdown(int socket, int how) {
	int retval;

	int sgx_retval = ocall_shutdown(&retval, socket, how);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in shutdown OCALL\n");
		sgx_exit();
	}

	return retval;
}

int connect(int socket, const struct sockaddr *address, socklen_t address_len) {
	int retval;

	int sgx_retval = ocall_connect(&retval, socket, address, address_len);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in connect OCALL\n");
		sgx_exit(sgx_retval);
	}

	return retval;
}

int listen(int socket, int backlog) {
	int retval;

	int sgx_retval = ocall_listen(&retval, socket, backlog);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in listen OCALL\n");
		sgx_exit();
	}

	return retval;
}

int socket(int domain, int type, int protocol) {
	int retval;

	int sgx_retval = ocall_socket(&retval, domain, type, protocol);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in socket OCALL\n");
		sgx_exit();
	}

	return retval;
}

ssize_t recv(int socket, void *buffer, size_t length, int flags) {
	ssize_t retval;

	int sgx_retval = ocall_recv(&retval, socket, buffer, length, flags);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in recv OCALL\n");
		sgx_exit();
	}

	return retval;
}

ssize_t send(int socket, const void *buffer, size_t length, int flags) {
	ssize_t retval;

	int sgx_retval = ocall_send(&retval, socket, buffer, length, flags);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in send OCALL\n");
		sgx_exit();
	}

	return retval;
}

ssize_t sendmsg(int socket, const struct msghdr *message, int flags) {
	ssize_t retval;

	// Array where we keep all pointer we allocated for easy freeing at the end
	size_t nb_mallocs = 4 + (message->msg_iovlen * 2);
	void **native_pointers_to_free = malloc(nb_mallocs);
	int index = 0;

	struct msghdr *message_native = native_malloc(sizeof(struct msghdr));
	native_pointers_to_free[index++] = message_native;

	// Copy trivial values
	message_native->msg_controllen = message->msg_controllen;
	message_native->msg_flags = message->msg_flags;
	message_native->msg_iovlen = message->msg_iovlen;
	message_native->msg_namelen = message->msg_namelen;

	// Now onto slightly more difficult values
	void* msg_control_native = native_malloc(message->msg_controllen);
	native_pointers_to_free[index++] = msg_control_native;
	memcpy(msg_control_native, message->msg_control, message->msg_controllen);
	message_native->msg_control = msg_control_native;

	void* msg_name_native = native_malloc(message->msg_namelen);
	native_pointers_to_free[index++] =
	memcpy(msg_name_native, message->msg_name, message->msg_namelen);
	message_native->msg_name = msg_name_native;

	// And finally, the complex value
	struct iovec *iov_array_native = native_malloc(message->msg_iovlen * sizeof(struct iovec));
	native_pointers_to_free[index++] = iov_array_native;
	message_native->msg_iov = iov_array_native;

	for (int i = 0; i < message->msg_iovlen; i++) {
		struct iovec *iov_native = native_malloc(sizeof(struct iovec));
		native_pointers_to_free[index++] = iov_native;

		size_t len = (message->msg_iov[i]).iov_len;
		iov_native->iov_len = len;
		void *iov_base_native = native_malloc(len);
		native_pointers_to_free[index++] = iov_base_native;
		memcpy(iov_base_native, (message->msg_iov[i]).iov_base, len);
		iov_native->iov_base = iov_base_native;

		iov_array_native[i] = *iov_native;
	}

	// Be sure that nb_mallocs is correct
	if (index != nb_mallocs) {
		printf("ERROR in computation of nb_mallocs in sendmsg\n");
		sgx_exit(1);
		return 0;
	}

	int sgx_retval = ocall_sendmsg(&retval, socket, message_native, flags);

	// Free all that memory we allocated earlier
	for (int i = 0; i < nb_mallocs; i++) {
		native_free(native_pointers_to_free[i]);
	}


	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in sendmsg OCALL\n");
		sgx_exit();
	}

	return retval;
}

struct servent *getservbyname(const char *name, const char *proto) {
	printf("TODO getservbyname\n");
}

struct hostent *gethostbyname(const char *name) {
	struct hostent *retval;

	int sgx_retval = ocall_gethostbyname(&retval, name);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in gethostbyname OCALL\n");
		sgx_exit();
	}

	return retval;
}

int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host,
		socklen_t hostlen, char *serv, socklen_t servlen, int flags) {
	printf("TODO getnameinfo\n");
}

int getpeername(int socket, struct sockaddr *restrict address,
		socklen_t *restrict address_len) {
	printf("TODO getpeername\n");
}

int ioctl(int fildes, int request, ... /* arg */) {
	printf("TODO ioctl\n");
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
	ssize_t retval;

	int sgx_retval = ocall_read(&retval, fildes, buf, nbyte);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in read OCALL\n");
		sgx_exit();
	}

	return retval;
}

int usleep(useconds_t usec) {
	int retval;

	int sgx_retval = ocall_usleep(&retval, usec);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in usleep OCALL\n");
		sgx_exit();
	}

	return retval;
}

int open(const char *path, int oflag, ...) {
	printf("TODO open\n");
}

int uname(struct utsname *name) {
	strncpy(name->machine, "SGX", 4);
	strncpy(name->nodename, "SGX", 4);
	strncpy(name->release, "SGX", 4);
	strncpy(name->sysname, "SGX", 4);
	strncpy(name->version, "SGX", 4);
	return 0;
}

int sscanf(const char *str, const char *format, ...) {
	va_list format_va;
	va_start(format_va, format);

	int retval = vsscanf(str, format, format_va);
	va_end(format_va);
}

pid_t getpid(void) {
	pid_t retval;

	int sgx_retval = ocall_getpid(&retval);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in getpid OCALL\n");
		sgx_exit();
	}

	return retval;
}

int getsockname(int socket, struct sockaddr *restrict address,
		socklen_t *restrict address_len) {
	printf("TODO getsockname\n");
}

int gethostname(char *name, size_t namelen) {
	int retval;

	int sgx_retval = ocall_gethostname(&retval, name, namelen);
	if (sgx_retval != SGX_SUCCESS) {
		printf("Error in gethostname OCALL\n");
		sgx_exit();
	}

	return retval;
}

long syscall(long number, ...) {
	return 42L;
}

int vsscanf(const char *str, const char *format, va_list args) {
	int val_cnt = 0;
	for (; *format != '\0'; format++) {
		if (*format == '%' && format[1] == 'd') {
			int positive;
			int value;
			int *valp;

			if (*str == '-') {
				positive = 0;
				str++;
			} else
				positive = 1;
			if (!isdigit(*str))
				break;
			value = 0;
			do {
				value = (value * 10) - (*str - '0');
				str++;
			} while (isdigit(*str));
			if (positive)
				value = -value;
			valp = va_arg(args, int *);
			val_cnt++;
			*valp = value;
			format++;
		} else if (*format == '%' && format[1] == 'c') {
			char value;
			char *valp;

			if (!isalpha(*str))
				break;
			value = *str;
			str++;
			valp = va_arg(args, char *);
			val_cnt++;
			*valp = value;
			format++;
		} else if (*format == *str) {
			str++;
		} else {
			break;
		}
	}
	return val_cnt;
}

int SSL_CTX_set_default_verify_paths(void *ctx) {
	return 1;
}
