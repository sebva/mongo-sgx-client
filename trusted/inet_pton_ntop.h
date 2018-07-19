/*
 * inet_pton_ntop.h
 *
 *  Created on: 19 Jul 2018
 *      Author: sebastien
 */

#ifndef TRUSTED_INET_PTON_NTOP_H_
#define TRUSTED_INET_PTON_NTOP_H_

#include <sys/socket.h>

int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop (int af, const void *src, char *dst, socklen_t size);


#endif /* TRUSTED_INET_PTON_NTOP_H_ */
