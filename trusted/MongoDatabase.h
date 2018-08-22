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
#include <database.h>

#define KEY_SIZE 32

typedef std::vector<std::array<uint8_t, KEY_SIZE>> KeyArray;
class MongoDatabase : public Database {
public:
	MongoDatabase(bool enable_tracing = false);
	virtual ~MongoDatabase();

	bool ping();
	void delete_user(const char* user_name);
	void add_user_to_group(const std::string &group_name, const std::string &user_name);
	void remove_user_from_group(const std::string &group_name, const std::string &user_name);
	bool is_user_part_of_group(const char* user_name, const char* group_name);
	void create_user(const std::string &user_name, const std::string &key);
    void create_group( const std::string &gname, const std::string &uid );
	KeyArray get_keys_of_group(const char* group_name);
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
};

#endif /* TRUSTED_MONGODATABASE_H_ */
