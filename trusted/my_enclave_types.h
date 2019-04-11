/*
 * my_enclave_types.h
 *
 *  Created on: 19 Jul 2018
 *      Author: sebastien
 */

#ifndef TRUSTED_MY_ENCLAVE_TYPES_H_
#define TRUSTED_MY_ENCLAVE_TYPES_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <stdarg.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>


#ifndef MONGOCLIENT_U_H__

typedef int32_t __pid_t;

#ifndef __pid_t_defined
typedef __pid_t pid_t;
# define __pid_t_defined
#endif

#endif /* MONGOCLIENT_U_H__ */

# ifndef __useconds_t_defined
typedef unsigned int __useconds_t;
typedef __useconds_t useconds_t;
#  define __useconds_t_defined
# endif

#endif /* TRUSTED_MY_ENCLAVE_TYPES_H_ */
