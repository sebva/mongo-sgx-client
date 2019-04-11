/*
 * MongoDatabase.h
 *
 *  Created on: 23 Jul 2018
 *      Author: sebastien
 */

#ifndef TRUSTED_MONGODATABASE_H_
#define TRUSTED_MONGODATABASE_H_

#include <mongoc.h>
#include <cstdint>
#include <string>
#include <database.h>

class MongoDatabase : public Database {
public:
    explicit MongoDatabase(const std::string& connection_url, bool enable_tracing = false);

    virtual ~MongoDatabase();

    bool init_collections();

    bool ping();

    void delete_user(const std::string &user_name);

    void delete_all_data();

    void add_user_to_group(const std::string &group_name, const std::string &user_name);

    void remove_user_from_group(const std::string &group_name, const std::string &user_name);

    bool is_user_part_of_group(const std::string &user_name, const std::string &group_name);

    void create_user(const std::string &user_name, const std::string &key);

    void create_group(const std::string &gname, const std::string &uid);

    KeyArray get_keys_of_group(const std::string &group_name);

private:
    mongoc_client_t *client;
    mongoc_collection_t *users_collection;
    mongoc_collection_t *groups_collection;
    const char *DB_NAME = "newtest";
    const char *USERS_COLLECTION_NAME = "users";
    const char *GROUPS_COLLECTION_NAME = "groups";

    inline void throw_potential_error(bson_error_t &error) {
        uint32_t error_code = error.code;
        if (error_code != 0) {
            throw error_code;
        }
    }
};

#endif /* TRUSTED_MONGODATABASE_H_ */
