/*
 * MongoDatabase.cpp
 *
 *  Created on: 23 Jul 2018
 *      Author: sebastien
 */

#include "MongoDatabase.h"

#include "ssl_wrappers.h"

MongoDatabase::MongoDatabase(const std::string &connection_url, bool enable_tracing) {
    // Required to initialize libmongoc's internals
    mongoc_init();

    if (!enable_tracing) {
        mongoc_log_trace_disable();
    }

    // Safely create a MongoDB URI object from the given string
    bson_error_t error;
    mongoc_uri_t *uri = mongoc_uri_new_with_error(connection_url.c_str(), &error);
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
    users_collection = mongoc_client_get_collection(client, DB_NAME, USERS_COLLECTION_NAME);
    groups_collection = mongoc_client_get_collection(client, DB_NAME, GROUPS_COLLECTION_NAME);
}

MongoDatabase::~MongoDatabase() {
    // Release our handles and clean up libmongoc
    mongoc_collection_destroy(users_collection);
    mongoc_collection_destroy(groups_collection);

    // Freeing the memory segfaults for some reason
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

bool MongoDatabase::init_collections() {
    bson_t reply;
    bson_error_t error;

    mongoc_database_t *database = mongoc_client_get_database(client, DB_NAME);

    bson_t *command = BCON_NEW(
            "createIndexes", BCON_UTF8(USERS_COLLECTION_NAME), "indexes", "[",
            "{", "key", "{", "name", BCON_INT32(1), "}", "name", "user_name_unique", "unique", BCON_BOOL(true), "}",
            "]");

    bool retval1 = mongoc_database_write_command_with_opts(database, command, nullptr, &reply, &error);

    const char *reply_str = bson_as_json(&reply, nullptr);
    printf("%s\n", reply_str);

    bson_destroy(&reply);
    bson_destroy(command);

    throw_potential_error(error);


    command = BCON_NEW(
            "createIndexes", BCON_UTF8(GROUPS_COLLECTION_NAME), "indexes", "[",
            "{", "key", "{", "name", BCON_INT32(1), "}", "name", "group_name_unique", "unique", BCON_BOOL(true), "}",
            "{", "key", "{", "name", BCON_INT32(1), "users.name", BCON_INT32(1), "}", "name", "groups_users", "}",
            "]");

    bool retval2 = mongoc_database_write_command_with_opts(database, command, nullptr, &reply, &error);

    reply_str = bson_as_json(&reply, nullptr);
    printf("%s\n", reply_str);

    bson_destroy(&reply);
    bson_destroy(command);

    throw_potential_error(error);

    return retval1 && retval2;
}

bool MongoDatabase::ping() {
    bson_error_t error;
    bson_t *command = BCON_NEW("ping", BCON_INT32(1));

    bool retval = mongoc_client_command_simple(client, "admin", command, nullptr, nullptr, &error);

    bson_destroy(command);

    throw_potential_error(error);
    return retval;
}

void MongoDatabase::delete_user(const std::string &user_name) {
    remove_user_from_all_groups(user_name);
    bson_t *selector = BCON_NEW("name", BCON_UTF8(user_name.c_str()));

    bson_error_t error;
    mongoc_collection_delete_one(users_collection, selector, nullptr, nullptr, &error);

    bson_destroy(selector);

    throw_potential_error(error);
}

void MongoDatabase::delete_all_data() {
    bson_t *selector = BCON_NEW(nullptr);
    bson_error_t error;

    mongoc_collection_delete_many(users_collection, selector, nullptr, nullptr, &error);
    throw_potential_error(error);

    mongoc_collection_delete_many(groups_collection, selector, nullptr, nullptr, &error);
    throw_potential_error(error);

    bson_destroy(selector);
}

void MongoDatabase::add_user_to_group(const std::string &group_name, const std::string &user_name) {
    const bson_t *user_document = retrieve_user_document(user_name);
    bson_iter_t iter;
    const uint8_t *user_key;
    uint32_t user_key_length;
    bson_subtype_t user_key_subtype;
    if (bson_iter_init_find(&iter, user_document, "key")) {
        bson_iter_binary(&iter, &user_key_subtype, &user_key_length, &user_key);
    } else {
        throw (uint32_t) 42u;
    }
    bson_destroy(const_cast<bson_t *>(user_document));

    bson_t *selector = BCON_NEW("name", BCON_UTF8(group_name.c_str()));
    bson_t *update = BCON_NEW("$addToSet", "{", "users",
                              "{", "name", BCON_UTF8(user_name.c_str()),
                              "key", BCON_BIN(user_key_subtype, user_key, user_key_length),
                              "}", "}");

    bson_error_t error;
    mongoc_collection_update_one(groups_collection, selector, update, nullptr, nullptr, &error);

    bson_destroy(selector);
    bson_destroy(update);

    throw_potential_error(error);
}

void MongoDatabase::remove_user_from_group(const std::string &group_name, const std::string &user_name) {
    bson_t *selector = BCON_NEW("name", BCON_UTF8(group_name.c_str()));
    bson_t *update = BCON_NEW("$pull", "{", "users", "{", "name", BCON_UTF8(user_name.c_str()), "}", "}");

    bson_error_t error;
    mongoc_collection_update_one(groups_collection, selector, update, nullptr, nullptr, &error);

    bson_destroy(selector);
    bson_destroy(update);

    throw_potential_error(error);
}

void MongoDatabase::remove_user_from_all_groups(const std::string &user_name) {
    bson_t *selector = BCON_NEW(nullptr);
    bson_t *update = BCON_NEW("$pull", "{", "users", "{", "name", BCON_UTF8(user_name.c_str()), "}", "}");

    bson_error_t error;
    mongoc_collection_update_many(groups_collection, selector, update, nullptr, nullptr, &error);

    bson_destroy(selector);
    bson_destroy(update);

    throw_potential_error(error);
}

bool MongoDatabase::is_user_part_of_group(const std::string &group_name, const std::string &user_name) {
    bson_t *query = BCON_NEW("name", BCON_UTF8(group_name.c_str()), "users.name", BCON_UTF8(user_name.c_str()));
    bson_t *opts = BCON_NEW("limit", BCON_INT32(1), "projection", "{", "_id", BCON_BOOL(true), "}");

    mongoc_read_prefs_t *read_prefs = mongoc_read_prefs_new(MONGOC_READ_PRIMARY_PREFERRED);

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(groups_collection, query, opts, read_prefs);
    bson_destroy(query);
    bson_destroy(opts);
    mongoc_read_prefs_destroy(read_prefs);

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

void MongoDatabase::create_group(const std::string &group_name, const std::string &user_name) {
    const bson_t *user_document = retrieve_user_document(user_name);
    bson_iter_t iter;
    const uint8_t *user_key;
    uint32_t user_key_length;
    bson_subtype_t user_key_subtype;
    if (bson_iter_init_find(&iter, user_document, "key")) {
        bson_iter_binary(&iter, &user_key_subtype, &user_key_length, &user_key);
    } else {
        throw (uint32_t) 42u;
    }
    bson_destroy(const_cast<bson_t *>(user_document));

    bson_t *document = BCON_NEW(
            "name", BCON_UTF8(group_name.c_str()),
            "users", "[",
            "{",
            "name", BCON_UTF8(user_name.c_str()),
            "key", BCON_BIN(user_key_subtype, user_key, user_key_length),
            "}",
            "]");

    bson_error_t error;
    mongoc_collection_insert_one(groups_collection, document, nullptr, nullptr, &error);

    bson_destroy(document);

    throw_potential_error(error);
}

void MongoDatabase::create_user(const std::string &user_name, const std::string &key) {
    bson_t *document = BCON_NEW("name", BCON_UTF8(user_name.c_str()),
                                "key", BCON_BIN(BSON_SUBTYPE_BINARY, (const uint8_t *) key.c_str(), KEY_SIZE));

    bson_error_t error;
    mongoc_collection_insert_one(users_collection, document, nullptr, nullptr, &error);

    bson_destroy(document);

    throw_potential_error(error);
}

KeyArray MongoDatabase::get_keys_of_group(const std::string &group_name) {
    bson_t *pipeline = BCON_NEW(
            "pipeline", "[",
            "{", "$match", "{", "name", BCON_UTF8(group_name.c_str()), "}", "}",
            "{", "$unwind", "{", "path", "$users", "}", "}",
            "{", "$replaceRoot", "{", "newRoot", "$users", "}", "}",
            "{", "$project", "{", "key", BCON_BOOL(true), "_id", BCON_BOOL(false), "}", "}",
            "]");

    mongoc_read_prefs_t *read_prefs = mongoc_read_prefs_new(MONGOC_READ_SECONDARY_PREFERRED);

    mongoc_cursor_t *cursor = mongoc_collection_aggregate(groups_collection, MONGOC_QUERY_NONE, pipeline, nullptr,
                                                          read_prefs);

    bson_destroy(pipeline);
    mongoc_read_prefs_destroy(read_prefs);

    bson_error_t error;
    if (mongoc_cursor_error(cursor, &error)) {
        mongoc_cursor_destroy(cursor);
        throw error.code;
    }

    bson_subtype_t subtype;
    const bson_t *key_document;
    uint32_t key_length;
    const uint8_t *key_binary;
    bson_iter_t iter;
    std::vector<std::array<uint8_t, KEY_SIZE>> list;

    while (mongoc_cursor_next(cursor, &key_document)) {
        bson_iter_init_find(&iter, key_document, "key");

        bson_iter_binary(&iter, &subtype, &key_length, &key_binary);
        std::array<uint8_t, KEY_SIZE> key_array_std{};
        memcpy(key_array_std.data(), key_binary, KEY_SIZE);

        list.push_back(key_array_std);
    }

    mongoc_cursor_destroy(cursor);
    return list;
}

const bson_t *MongoDatabase::retrieve_user_document(const std::string &user_name) {
    bson_t *query = BCON_NEW("name", BCON_UTF8(user_name.c_str()));

    mongoc_read_prefs_t *read_prefs = mongoc_read_prefs_new(MONGOC_READ_PRIMARY_PREFERRED);

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(users_collection, query, nullptr, read_prefs);
    bson_destroy(query);
    mongoc_read_prefs_destroy(read_prefs);

    const bson_t *document;
    bool document_exists = mongoc_cursor_next(cursor, &document);
    if (!document_exists) {
        printf("No such document");
        throw 42u;
    }

    bson_error_t error;
    if (mongoc_cursor_error(cursor, &error)) {
        mongoc_cursor_destroy(cursor);
        throw error.code;
    }

    mongoc_cursor_destroy(cursor);

    return document;
}
