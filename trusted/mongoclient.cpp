#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include <mongoc.h>

#include "MongoDatabase.h"
#include "my_wrappers.h"

#include "mongoclient.h"
#include "mongoclient_t.h"  /* print_string */

#include <vector>
#include <array>


void printMemberships(MongoDatabase &database);

int ecall_mongoclient_sample() {
    printf("IN MONGOCLIENT\n");

    auto mongo_url = "mongodb://sgx-3.maas:27017/?ssl=true&sslAllowInvalidCertificates=true&sslAllowInvalidHostnames=true";
    MongoDatabase database(false);
    database.init(mongo_url);

    // printf("PING %s\n", database.ping() ? "success" : "fail");

    printf("Deleting everything\n");
    database.delete_all_data();

    printf("Init collection\n");
    bool success = database.init_indexes();
    if (!success) {
        printf("Error when initializing the indexes\n");
    }

    printf("Generating key\n");
    const char key_raw[KEY_SIZE] = {'h', 'u', 'n', 't', 'e', 'r', '2', '\0'};
    std::string key(key_raw, KEY_SIZE);

    try {
        printf("Creating user1\n");
        database.create_user("user1", key);
        printf("Creating user2\n");
        database.create_user("user2", key);
    } catch (bson_error_t &error) {
        printf("ERROR: exception thrown when adding users. Did a previous run fail?. Error %ld: %s\n",
               error.code, error.message);
    }

    printMemberships(database);

    printf("Creating group1 with user1 as member\n");
    database.create_group("group1", "user1");

    printMemberships(database);

    printf("Creating group1 with user1 as member again\n");
    try {
        database.create_group("group1", "user1");
    } catch (bson_error_t &error) {
        printf("OK, exception thrown when creating group again. Error %ld: %s\n", error.code, error.message);
    }
    printMemberships(database);

    printf("Creating group2 with user2 as member\n");
    database.create_group("group2", "user2");
    printMemberships(database);

    printf("Adding user1 to group2\n");
    database.add_user_to_group("group2", "user1");

    printMemberships(database);

    printf("Adding user1 to group1 again\n");
    database.add_user_to_group("group1", "user1");

    printMemberships(database);

    try {
        printf("Creating user user1 again\n");
        database.create_user("user1", key);
    } catch (bson_error_t &error) {
        printf("OK, exception thrown when adding user1 again. Error %ld: %s\n", error.code, error.message);
    }

    printMemberships(database);

    printf("All keys of group1\n");
    KeyArray list = database.get_keys_of_group("group1");
    int i = 1;
    for (auto it = list.begin(); it != list.end(); it++, i++) {
        printf("Key no %d: %s\n", i, (*it).data());
    }

    printf("Removing user1 from group2\n");
    database.remove_user_from_group("group2", "user1");
    printMemberships(database);

    printf("Adding user1 to group2 again\n");
    database.add_user_to_group("group2", "user1");
    printMemberships(database);

    printf("Deleting user1\n");
    database.delete_user("user1");
    printMemberships(database);

    return 0;
}

void printMemberships(MongoDatabase &database) {
    printf("Is user1 part of group1: %d\n", database.is_user_part_of_group("group1", "user1"));
    printf("Is user2 part of group1: %d\n", database.is_user_part_of_group("group1", "user2"));
    printf("Is user1 part of group2: %d\n", database.is_user_part_of_group("group2", "user1"));
    printf("Is user2 part of group2: %d\n", database.is_user_part_of_group("group2", "user2"));
}
