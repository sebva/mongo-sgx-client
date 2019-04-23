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

#define HMAC_KEY_LENGTH 64
#define HMAC_RESULT_LENGTH 32

typedef std::array<unsigned char, HMAC_RESULT_LENGTH> hashed_t;

class MongoDatabase : public Database {
public:
    explicit MongoDatabase(bool enable_tracing = false);

    virtual ~MongoDatabase();

    bool init(const std::string &connection_url);

    bool init_indexes();

    bool ping();

    void delete_user(const std::string &user_name);

    void delete_all_data();

    void add_user_to_group(const std::string &group_name, const std::string &user_name);

    void remove_user_from_group(const std::string &group_name, const std::string &user_name);

    bool is_user_part_of_group(const std::string &group_name, const std::string &user_name);

    void create_user(const std::string &user_name, const std::string &key);

    void create_group(const std::string &group_name, const std::string &user_name);

    KeyArray get_keys_of_group(const std::string &group_name);

private:
    mongoc_client_t *client;
    mongoc_collection_t *users_collection;
    mongoc_collection_t *groups_collection;
    const char *DB_NAME = "newtest";
    const char *USERS_COLLECTION_NAME = "users";
    const char *GROUPS_COLLECTION_NAME = "groups";
    const unsigned char *hmac_key = reinterpret_cast<const unsigned char *>("\x3c\x4f\xd3\x55\x3b\x89\xa1\xf8\xfa\xcb\x3b\xb7\x68\x2c\x8c\x4f\x0a\x69\x03\xde\x7b\x45\x8c\x6a\xd8\x7b\x6b\xe1\x3d\x1b\x96\x67\xe0\xb6\x47\xa7\xe4\x59\x5d\x56\xc8\xf1\x3a\x34\x02\x79\xb9\xeb\x61\xa0\x13\x87\xc2\x34\x88\x05\xc1\xe4\x43\x36\x26\xaf\xb0\x52");
    const std::string aes_key = "\x11\x7e\x15\xeb\x33\x18\xde\x37\x55\xd2\x21\x3a\x79\x3d\xaf\x7a\x81\x18\xc9\x11\x5f\x35\x1d\x06\x67\x6f\xaa\x6c\x72\xc8\xea\xfd";

    inline void throw_potential_error(bson_error_t &error) {
        uint32_t error_code = error.code;
        if (error_code != 0) {
            throw error;
        }
    }

    void add_user_to_group(const std::string &group_name, const std::string &user_name,
                           const std::string &user_key_reencrypted);

    const std::string reencrypt_user_key(const std::string &user_name);

    void remove_user_from_all_groups(const std::string &user_name);

    const hashed_t hash_name(const std::string &name, const std::string *optional_salt = nullptr);

    const std::string encrypt_data(const std::string &data);

    const std::string decrypt_data(const std::string &data);

    const hashed_t compute_user_signature(const hashed_t &hashed_user_name, const std::string &encrypted_user_key);

    const hashed_t compute_hmac_2(const uint8_t *element_1, uint32_t element_1_length,
                                  const uint8_t *element_2, uint32_t element_2_length);

    // Alias C-style compute_user_signature to compute_hmac_2 https://stackoverflow.com/a/9864472/1045559
    template<typename... Args>
    auto compute_user_signature(Args &&... args) -> decltype(compute_hmac_2(std::forward<Args>(args)...)) {
        return compute_hmac_2(std::forward<Args>(args)...);
    }

    hashed_t compute_group_signature(const bson_t *group_document,
                                     const hashed_t *additional_user_name = nullptr,
                                     const std::string *additional_user_key = nullptr,
                                     bool additional_is_add = true);

    bool validate_group_signature(const bson_t *group_document);
};

#endif /* TRUSTED_MONGODATABASE_H_ */
