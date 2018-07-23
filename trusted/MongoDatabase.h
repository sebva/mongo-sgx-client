/*
 * MongoDatabase.h
 *
 *  Created on: 23 Jul 2018
 *      Author: sebastien
 */

#ifndef TRUSTED_MONGODATABASE_H_
#define TRUSTED_MONGODATABASE_H_

#include <set>
#include <mongoc.h>

class MongoDatabase {
public:
	MongoDatabase(bool enable_tracing = false);
	virtual ~MongoDatabase();

	bool ping();
	bool create_user(const char* user_name);
	bool add_user_to_group(const char* user_name, const char* group_name);
	bool is_user_part_of_group(const char* user_name, const char* group_name);
	std::set<const char*> get_keys_of_group(const char* group_name);

private:
	mongoc_client_t *client;
	mongoc_collection_t *users_collection;
	const char* CONNECTION_URL = "mongodb://sgx-3.maas:27017/?ssl=true&sslAllowInvalidCertificates=true&sslAllowInvalidHostnames=true";
};

#endif /* TRUSTED_MONGODATABASE_H_ */
