#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include <mongoc.h>

#include "MongoDatabase.h"
#include "my_wrappers.h"

#include "mongoclient.h"
#include "mongoclient_t.h"  /* print_string */

#include <vector>
#include <array>


int ecall_mongoclient_sample() {
    printf("IN MONGOCLIENT\n");

    auto mongo_url = "mongodb://sgx-3.maas:27017/?ssl=true&sslAllowInvalidCertificates=true&sslAllowInvalidHostnames=true";
    MongoDatabase database(mongo_url, false);

    printf("PING %s\n", database.ping() ? "success" : "fail");

    printf("Init collection");
    bool success = database.init_collections();
    if (!success) {
        printf("Error when initializing the indexes\n");
    }

    printf("Generating key\n");
    uint8_t key[] = {'h', 'u', 'n', 't', 'e', 'r', '2', '\0'};

    try {
        printf("Creating user toto\n");
        database.create_user("toto", (const char *) key);
        database.create_user("toto2", (const char *) key);
    } catch (uint32_t error_code) {
        printf("ERROR: exception thrown when adding toto. Did a previous run fail?. Error %ld\n.", error_code);
    }

    printf("Is toto part of group1: %d\n", database.is_user_part_of_group("group1", "toto"));

    printf("Creating group1 with toto as member\n");
    database.create_group("group1", "toto");

    printf("Is toto part of group1: %d\n", database.is_user_part_of_group("group1", "toto"));

    printf("Adding toto to group1 again\n");
    database.add_user_to_group("group1", "toto");

    printf("Is toto part of group1: %d\n", database.is_user_part_of_group("group1", "toto"));

    try {
        printf("Creating user toto again\n");
        database.create_user("toto", (const char *) key);
    } catch (uint32_t error_code) {
        printf("OK, exception thrown when adding toto again. Error %ld\n", error_code);
    }

    printf("All keys of group1\n");
    KeyArray list = database.get_keys_of_group("group1");
    int i = 1;
    for (auto it = list.begin(); it != list.end(); it++, i++) {
        printf("Key no %d: %s\n", i, (*it).data());
    }

    printf("Removing toto from group1\n");
    database.remove_user_from_group("group1", "toto");
    printf("Is toto part of group1: %d\n", database.is_user_part_of_group("group1", "toto"));

    printf("Adding toto to group1 again\n");
    database.add_user_to_group("group1", "toto");
    printf("Is toto part of group1: %d\n", database.is_user_part_of_group("group1", "toto"));

    printf("Deleting toto\n");
    database.delete_user("toto");
    printf("Is toto part of group1: %d\n", database.is_user_part_of_group("group1", "toto"));

    printf("Deleting everything\n");
    database.delete_all_data();
    return 0;
}
