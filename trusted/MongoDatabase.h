/*
 * MongoDatabase.h
 *
 *  Created on: 23 Jul 2018
 *      Author: sebastien
 */

#ifndef TRUSTED_MONGODATABASE_H_
#define TRUSTED_MONGODATABASE_H_

#include <vector>
#include <array>
#include <mongoc.h>
#include <cstdint>

class MongoDatabase {
public:
	MongoDatabase(bool enable_tracing = false);
	virtual ~MongoDatabase();

	bool ping();
	void delete_user(const char* user_name);
	void add_user_to_group(const char* user_name, const char* group_name);
	void remove_user_from_group(const char* user_name, const char* group_name);
	bool is_user_part_of_group(const char* user_name, const char* group_name);
private:
	mongoc_client_t *client;
	mongoc_collection_t *users_collection;
	const char* CONNECTION_URL = "mongodb://sgx-3.maas:27017/?ssl=true&sslAllowInvalidCertificates=true&sslAllowInvalidHostnames=true";

	inline void throw_potential_error(bson_error_t& error) {
		uint32_t error_code = error.code;
		if (error_code != 0) {
			throw error_code;
		}
	}

public:
	template <uint32_t KEY_SIZE>
	void create_user(const char* user_name, uint8_t* key) {
		bson_t *document = BCON_NEW("name", BCON_UTF8(user_name), "key", BCON_BIN(BSON_SUBTYPE_BINARY, key, KEY_SIZE), "groups", "[", "]");

		bson_t reply;
		bson_error_t error;
		bool retval = mongoc_collection_insert_one(users_collection, document, nullptr, &reply, &error);

		bson_destroy(&reply);
		bson_destroy(document);

		throw_potential_error(error);
	}

	template <uint32_t KEY_SIZE>
	std::vector<std::array<uint8_t, KEY_SIZE>> get_keys_of_group(const char* group_name) {
		bson_t *pipeline = BCON_NEW("pipeline", "[",
				"{", "$match", "{", "groups", BCON_UTF8(group_name), "}", "}",
				"{", "$project", "{", "key", BCON_BOOL(true), "_id", BCON_BOOL(false), "}", "}",
			"]");

		mongoc_cursor_t *cursor = mongoc_collection_aggregate(users_collection, MONGOC_QUERY_NONE, pipeline, nullptr, nullptr);

		bson_destroy(pipeline);

		bson_error_t error;
		if (mongoc_cursor_error(cursor, &error)) {
			mongoc_cursor_destroy(cursor);
			throw error.code;
		}

		bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
		const bson_t *key_document;
		uint32_t key_length;
		const uint8_t *key_binary;
		bson_iter_t iter;
		std::vector<std::array<uint8_t, KEY_SIZE>> list;

		while (mongoc_cursor_next(cursor, &key_document)) {
			bson_iter_init_find(&iter, key_document, "key");

			bson_iter_binary(&iter, &subtype, &key_length, &key_binary);
			std::array<uint8_t, KEY_SIZE> key_array_std;
			memcpy(key_array_std.data(), key_binary, KEY_SIZE);

			list.push_back(key_array_std);
		}

		mongoc_cursor_destroy(cursor);
		return list;
	}
};

#endif /* TRUSTED_MONGODATABASE_H_ */
