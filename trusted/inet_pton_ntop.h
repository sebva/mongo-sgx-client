/*
 * inet_pton_ntop.h
 *
 *  Created on: 19 Jul 2018
 *      Author: sebastien
 */

#ifndef TRUSTED_INET_PTON_NTOP_H_
#define TRUSTED_INET_PTON_NTOP_H_

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop (int af, const void *src, char *dst, socklen_t size);

#ifdef __cplusplus
}
#endif

#endif /* TRUSTED_INET_PTON_NTOP_H_ */
