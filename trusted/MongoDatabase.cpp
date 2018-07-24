/*
 * MongoDatabase.cpp
 *
 *  Created on: 23 Jul 2018
 *      Author: sebastien
 */

#include "MongoDatabase.h"

#include "ssl_wrappers.h"


MongoDatabase::MongoDatabase(bool enable_tracing) {
	// Required to initialize libmongoc's internals
	mongoc_init();

	if (!enable_tracing) {
		mongoc_log_trace_disable();
	}

	// Safely create a MongoDB URI object from the given string
	bson_error_t error;
	mongoc_uri_t* uri = mongoc_uri_new_with_error(CONNECTION_URL, &error);
	if (!uri) {
		printf("failed to parse URI: %s\n", error.message);
		return;
	}

	// Create a new client instance
	client = mongoc_client_new_from_uri(uri);
	if (!client) {
		printf("Client failure\n");
		return;
	}
	printf("Client OK\n");

	mongoc_uri_destroy(uri);

	/*
	 * Register the application name so we can track it in the profile logs
	 * on the server. This can also be done from the URI (see other examples).
	 */
	mongoc_client_set_appname(client, "sgx-anonymbe");

	/*
	 * Get a handle on the database "db_name" and collection "coll_name"
	 */
	users_collection = mongoc_client_get_collection(client, "test", "users");
}

MongoDatabase::~MongoDatabase() {
	// Release our handles and clean up libmongoc
	mongoc_collection_destroy(users_collection);

	// Freeing the memory segfaults for some reason
	// mongoc_client_destroy(client);
	mongoc_cleanup();
}

bool MongoDatabase::ping() {
	bson_t reply;
	bson_error_t error;
	bson_t *command = BCON_NEW("ping", BCON_INT32 (1));

	bool retval = mongoc_client_command_simple(client, "admin", command, nullptr, &reply, &error);

	bson_destroy(&reply);
	bson_destroy(command);

	throw_potential_error(error);
	return retval;
}

void MongoDatabase::delete_user(const char* user_name) {
	bson_t *selector = BCON_NEW("name", BCON_UTF8(user_name));

	bson_t reply;
	bson_error_t error;
	bool retval = mongoc_collection_delete_one(users_collection, selector, nullptr, &reply, &error);

	bson_destroy(&reply);
	bson_destroy(selector);

	throw_potential_error(error);
}

void MongoDatabase::add_user_to_group(const char* user_name, const char* group_name) {
	bson_t *selector = BCON_NEW("name", BCON_UTF8(user_name));
	bson_t *update = BCON_NEW("$addToSet", "{", "groups", BCON_UTF8(group_name), "}");

	bson_t reply;
	bson_error_t error;
	bool retval = mongoc_collection_update_one(users_collection, selector, update, nullptr, &reply, &error);

	bson_destroy(&reply);
	bson_destroy(selector);
	bson_destroy(update);

	throw_potential_error(error);
}

void MongoDatabase::remove_user_from_group(const char* user_name, const char* group_name) {
	bson_t *selector = BCON_NEW("name", BCON_UTF8(user_name));
	bson_t *update = BCON_NEW("$pull", "{", "groups", BCON_UTF8(group_name), "}");

	bson_t reply;
	bson_error_t error;
	bool retval = mongoc_collection_update_one(users_collection, selector, update, nullptr, &reply, &error);

	bson_destroy(&reply);
	bson_destroy(selector);
	bson_destroy(update);

	throw_potential_error(error);
}

bool MongoDatabase::is_user_part_of_group(const char* user_name, const char* group_name) {
	bson_t *query = BCON_NEW("name", BCON_UTF8(user_name), "groups", BCON_UTF8(group_name));
	bson_t *opts = BCON_NEW("limit", BCON_INT32(1), "projection", "{", "_id", BCON_BOOL(true), "}");

	mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(users_collection, query, opts, nullptr);
	bson_destroy(query);
	bson_destroy(opts);

	const bson_t *ignored;
	bool document_exists = mongoc_cursor_next(cursor, &ignored);

	bson_error_t error;
	if (mongoc_cursor_error(cursor, &error)) {
		mongoc_cursor_destroy(cursor);
		throw error.code;
	}

	mongoc_cursor_destroy(cursor);

	return document_exists;
}
