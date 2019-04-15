/*
 * MongoDatabase.cpp
 *
 *  Created on: 23 Jul 2018
 *      Author: sebastien
 */

#include "MongoDatabase.h"

#include "ssl_wrappers.h"

#include <sgx_tcrypto.h>
#include <sgx_cryptoall.h>
#include <array>

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

    auto hashed_user_name = hash_name(user_name);
    bson_t *selector = BCON_NEW("name",
                                BCON_BIN(BSON_SUBTYPE_BINARY, hashed_user_name.data(), hashed_user_name.size()));

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

    // Re-encrypt the user's key
    auto user_key_reencrypted = encrypt_data(decrypt_data(std::string((const char *) user_key, user_key_length)));
    auto hashed_group_name = hash_name(group_name);
    auto hashed_user_name = hash_name(user_name, &group_name);

    bson_t *selector = BCON_NEW("name",
                                BCON_BIN(BSON_SUBTYPE_BINARY, hashed_group_name.data(), hashed_group_name.size()));
    bson_t *update = BCON_NEW("$addToSet", "{", "users", "{",
                              "name", BCON_BIN(BSON_SUBTYPE_BINARY, hashed_user_name.data(), hashed_user_name.size()),
                              "key", BCON_BIN(BSON_SUBTYPE_BINARY,
                                              (const uint8_t *) user_key_reencrypted.data(),
                                              user_key_reencrypted.size()),
                              "}", "}");

    bson_error_t error;
    mongoc_collection_update_one(groups_collection, selector, update, nullptr, nullptr, &error);

    bson_destroy(selector);
    bson_destroy(update);

    throw_potential_error(error);
}

void MongoDatabase::remove_user_from_group(const std::string &group_name, const std::string &user_name) {
    auto hashed_group_name = hash_name(group_name);
    auto hashed_user_name = hash_name(user_name, &group_name);

    bson_t *selector = BCON_NEW("name",
                                BCON_BIN(BSON_SUBTYPE_BINARY, hashed_group_name.data(), hashed_group_name.size()));
    bson_t *update = BCON_NEW("$pull", "{", "users", "{",
                              "name", BCON_BIN(BSON_SUBTYPE_BINARY, hashed_user_name.data(), hashed_user_name.size()),
                              "}", "}");

    bson_error_t error;
    mongoc_collection_update_one(groups_collection, selector, update, nullptr, nullptr, &error);

    bson_destroy(selector);
    bson_destroy(update);

    throw_potential_error(error);
}

void MongoDatabase::remove_user_from_all_groups(const std::string &user_name) {
    bson_t *null_selector = BCON_NEW(nullptr);
    mongoc_read_prefs_t *read_prefs = mongoc_read_prefs_new(MONGOC_READ_PRIMARY_PREFERRED);

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(groups_collection, null_selector, nullptr, read_prefs);
    mongoc_read_prefs_destroy(read_prefs);
    bson_destroy(null_selector);

    mongoc_bulk_operation_t *bulk_operation = mongoc_collection_create_bulk_operation_with_opts(groups_collection,
                                                                                                nullptr);
    const bson_t *group_document;
    while (mongoc_cursor_next(cursor, &group_document)) {
        bson_iter_t iter;
        uint32_t encrypted_group_length;
        const uint8_t *encrypted_group_name;

        bson_iter_init_find(&iter, group_document, "encname");
        bson_iter_binary(&iter, nullptr, &encrypted_group_length, &encrypted_group_name);
        auto group_name = decrypt_data(std::string((const char *) encrypted_group_name, encrypted_group_length));
        printf("Iterating over %s\n", group_name.c_str());
        auto hashed_user_name = hash_name(user_name, &group_name);

        bson_t *update = BCON_NEW("$pull", "{", "users", "{",
                                  "name",
                                  BCON_BIN(BSON_SUBTYPE_BINARY, hashed_user_name.data(), hashed_user_name.size()),
                                  "}", "}");

        mongoc_bulk_operation_update_one(bulk_operation, group_document, update, false);

        bson_destroy(update);
    }

    bson_error_t error;
    mongoc_bulk_operation_execute(bulk_operation, nullptr, &error);
    throw_potential_error(error);
}

bool MongoDatabase::is_user_part_of_group(const std::string &group_name, const std::string &user_name) {
    auto hashed_group_name = hash_name(group_name);
    auto hashed_user_name = hash_name(user_name, &group_name);

    bson_t *query = BCON_NEW("name", BCON_BIN(BSON_SUBTYPE_BINARY, hashed_group_name.data(), hashed_group_name.size()),
                             "users.name",
                             BCON_BIN(BSON_SUBTYPE_BINARY, hashed_user_name.data(), hashed_user_name.size()));
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
    auto hashed_group_name = hash_name(group_name);
    auto encrypted_group_name = encrypt_data(group_name);

    bson_t *document = BCON_NEW(
            "name", BCON_BIN(BSON_SUBTYPE_BINARY, hashed_group_name.data(), hashed_group_name.size()),
            "encname",
            BCON_BIN(BSON_SUBTYPE_BINARY, (uint8_t *) encrypted_group_name.data(), encrypted_group_name.size()),
            "users", "[", "]");

    bson_error_t error;
    mongoc_collection_insert_one(groups_collection, document, nullptr, nullptr, &error);

    bson_destroy(document);

    throw_potential_error(error);

    add_user_to_group(group_name, user_name);
}

void MongoDatabase::create_user(const std::string &user_name, const std::string &key) {
    auto hashed_user_name = hash_name(user_name);
    auto encrypted_key = encrypt_data(key);

    bson_t *document = BCON_NEW(
            "name", BCON_BIN(BSON_SUBTYPE_BINARY, hashed_user_name.data(), hashed_user_name.size()),
            "key", BCON_BIN(BSON_SUBTYPE_BINARY, (const uint8_t *) encrypted_key.data(), encrypted_key.size()));

    bson_error_t error;
    mongoc_collection_insert_one(users_collection, document, nullptr, nullptr, &error);

    bson_destroy(document);

    throw_potential_error(error);
}

KeyArray MongoDatabase::get_keys_of_group(const std::string &group_name) {
    auto hashed_group_name = hash_name(group_name);

    bson_t *pipeline = BCON_NEW(
            "pipeline", "[",
            "{", "$match", "{",
            "name", BCON_BIN(BSON_SUBTYPE_BINARY, hashed_group_name.data(), hashed_group_name.size()),
            "}", "}",
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
        auto decrypted_key = decrypt_data(std::string((const char *) key_binary, key_length));
        std::array<uint8_t, KEY_SIZE> key_array_std{};

        if (decrypted_key.size() != KEY_SIZE) {
            throw 555u;
        }

        memcpy(key_array_std.data(), decrypted_key.data(), decrypted_key.size());

        list.push_back(key_array_std);
    }

    mongoc_cursor_destroy(cursor);
    return list;
}

const bson_t *MongoDatabase::retrieve_user_document(const std::string &user_name) {
    auto hashed_user_name = hash_name(user_name);

    bson_t *query = BCON_NEW("name", BCON_BIN(BSON_SUBTYPE_BINARY, hashed_user_name.data(), hashed_user_name.size()));

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

const hashed_t MongoDatabase::hash_user(const hashed_t &hashed_name, const std::string &encrypted_key) {
    sgx_hmac_state_handle_t context;
    sgx_hmac256_init(hmac_key, HMAC_KEY_LENGTH, &context);
    sgx_hmac256_update((const uint8_t *) hashed_name.data(), hashed_name.size(), context);
    sgx_hmac256_update((const uint8_t *) encrypted_key.data(), encrypted_key.size(), context);
    std::array<unsigned char, HMAC_RESULT_LENGTH> result{};
    sgx_hmac256_final(result.data(), HMAC_RESULT_LENGTH, context);
    sgx_hmac256_close(context);
    return result;
}

const hashed_t MongoDatabase::hash_name(const std::string &name, const std::string *optional_salt) {
    sgx_hmac_state_handle_t context;
    sgx_hmac256_init(hmac_key, HMAC_KEY_LENGTH, &context);
    sgx_hmac256_update((const uint8_t *) name.data(), name.size(), context);
    if (optional_salt != nullptr) {
        sgx_hmac256_update((const uint8_t *) optional_salt->data(), optional_salt->length(), context);
    }
    std::array<unsigned char, HMAC_RESULT_LENGTH> result{};
    sgx_hmac256_final(result.data(), HMAC_RESULT_LENGTH, context);
    sgx_hmac256_close(context);
    return result;
}

const std::string MongoDatabase::encrypt_data(const std::string &data) {
    return Crypto::encrypt_aesgcm(aes_key, data);
}

const std::string MongoDatabase::decrypt_data(const std::string &data) {
    auto result = Crypto::decrypt_aesgcm(aes_key, data);
    if (result.first) {
        return result.second;
    } else {
        throw 44u;
    }
}
