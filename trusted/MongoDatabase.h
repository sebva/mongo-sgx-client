/*
 * MongoDatabase.h
 *
 *  Created on: 23 Jul 2018
 *      Author: sebastien
 */

#ifndef TRUSTED_MONGODATABASE_H_
#define TRUSTED_MONGODATABASE_H_

#include <string>
#include <mongoc.h>

class MongoDatabase {
public:
	MongoDatabase(bool enable_tracing = false);
	virtual ~MongoDatabase();

	bool ping();

private:
	mongoc_client_t *client;
	mongoc_collection_t *users_collection;
	const char* CONNECTION_URL = "mongodb://sgx-3.maas:27017/?ssl=true&sslAllowInvalidCertificates=true&sslAllowInvalidHostnames=true";
};

#endif /* TRUSTED_MONGODATABASE_H_ */
