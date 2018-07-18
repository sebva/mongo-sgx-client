#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include <mongoc.h>

#include "mongoclient.h"
#include "mongoclient_t.h"  /* print_string */


/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...) {
	char buf[BUFSIZ] = { '\0' };
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, BUFSIZ, fmt, ap);
	va_end(ap);
	ocall_mongoclient_sample(buf);
}

int ecall_mongoclient_sample() {
	printf("IN MONGOCLIENT\n");

	const char *uri_string = "mongodb://sgx-3.maas:27017/?ssl=true&sslAllowInvalidCertificates=true&sslAllowInvalidHostnames=true";
	mongoc_uri_t *uri;
	mongoc_client_t *client;
	mongoc_database_t *database;
	mongoc_collection_t *collection;
	bson_t *command, reply, *insert;
	bson_error_t error;
	char *str;
	bool retval;

	/*
	 * Required to initialize libmongoc's internals
	 */
	mongoc_init();

	/*
	 * Safely create a MongoDB URI object from the given string
	 */
	uri = mongoc_uri_new_with_error(uri_string, &error);
	if (!uri) {
		printf("failed to parse URI: %s\n"
				"error message:       %s\n", uri_string, error.message);
		return EXIT_FAILURE;
	}

	/*
	 * Create a new client instance
	 */
	client = mongoc_client_new_from_uri(uri);
	if (!client) {
		return EXIT_FAILURE;
	}

	/*
	 * Register the application name so we can track it in the profile logs
	 * on the server. This can also be done from the URI (see other examples).
	 */
	mongoc_client_set_appname(client, "connect-example");

	/*
	 * Get a handle on the database "db_name" and collection "coll_name"
	 */
	database = mongoc_client_get_database(client, "db_name");
	collection = mongoc_client_get_collection(client, "db_name", "coll_name");

	/*
	 * Do work. This example pings the database, prints the result as JSON and
	 * performs an insert
	 */
	command = BCON_NEW("ping", BCON_INT32 (1));

	retval = mongoc_client_command_simple(client, "admin", command, NULL,
			&reply, &error);

	if (!retval) {
		printf("%s\n", error.message);
		return EXIT_FAILURE;
	}

	str = bson_as_json(&reply, NULL);
	printf("%s\n", str);

	insert = BCON_NEW("hello", BCON_UTF8 ("world"));

	if (!mongoc_collection_insert_one(collection, insert, NULL, NULL, &error)) {
		printf("%s\n", error.message);
	}

	bson_destroy(insert);
	bson_destroy(&reply);
	bson_destroy(command);
	bson_free(str);

	/*
	 * Release our handles and clean up libmongoc
	 */
	mongoc_collection_destroy(collection);
	mongoc_database_destroy(database);
	mongoc_uri_destroy(uri);
	mongoc_client_destroy(client);
	mongoc_cleanup();

	return 0;
}

