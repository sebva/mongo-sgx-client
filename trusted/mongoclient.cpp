#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include <mongoc.h>

#include "mongoclient.h"
#include "mongoclient_t.h"  /* print_string */

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_mongoclient_sample(buf);
}

int ecall_mongoclient_sample()
{
  printf("IN MONGOCLIENT\n");
  return 0;
}

