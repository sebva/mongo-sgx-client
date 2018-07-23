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

	printf("PING %s\n", database.ping() ? "success" : "fail");

	bool status = database.create_user("toto");
	printf("Creating user toto: %s\n", status ? "success" : "fail");

	status = database.add_user_to_group("toto", "group2");
	printf("Add toto to group2: %s\n", status ? "success" : "fail");

	return 0;
}

