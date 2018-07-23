#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include <mongoc.h>

#include "MongoDatabase.h"
#include "my_wrappers.h"

#include "mongoclient.h"
#include "mongoclient_t.h"  /* print_string */

int ecall_mongoclient_sample() {
	printf("IN MONGOCLIENT\n");

	MongoDatabase database;

	printf("PING %s\n", database.ping() ? "true" : "false");

	return 0;
}

