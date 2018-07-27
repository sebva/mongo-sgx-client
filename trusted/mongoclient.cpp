#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include <mongoc.h>

#include "MongoDatabase.h"
#include "my_wrappers.h"

#include "mongoclient.h"
#include "mongoclient_t.h"  /* print_string */

#include <vector>
#include <array>


#define KEY_LENGTH 7


int ecall_mongoclient_sample() {
	printf("IN MONGOCLIENT\n");

	MongoDatabase database(false);

	printf("PING %s\n", database.ping() ? "success" : "fail");

	printf("Init collection");
	database.init_collection();

	printf("Generating key\n");
	uint8_t key[] = {'h', 'u', 'n', 't', 'e', 'r', '2'};

	try {
		printf("Creating user toto\n");
		database.create_user<KEY_LENGTH>("toto", key);
	} catch (uint32_t error_code) {
		printf("ERROR: exception thrown when adding toto. Did a previous "
				"run fail?. Error %ld\n.",
				error_code);
	}

	printf("Is toto part of group1: %d\n", database.is_user_part_of_group("toto", "group1"));

	printf("Adding toto to group1\n");
	database.add_user_to_group("toto", "group1");

	printf("Adding toto to group1 again\n");
	database.add_user_to_group("toto", "group1");

	//*
	printf("Is toto part of group1: %d\n", database.is_user_part_of_group("toto", "group1"));
	//*/

	try {
		printf("Creating user toto again\n");
		database.create_user<KEY_LENGTH>("toto", key);
	} catch(uint32_t error_code) {
		printf("OK, exception thrown when adding toto again. Error %ld\n.", error_code);
	}

	//*
	printf("All keys of group1\n");
	std::vector<std::array<uint8_t, KEY_LENGTH>> list = database.get_keys_of_group<KEY_LENGTH>("group1");
	int i = 1;
	for(std::vector<std::array<uint8_t, KEY_LENGTH>>::iterator it = list.begin(); it != list.end(); it++, i++) {
		printf("Key no %d: %s\n", i, (*it).data());
	}
	//*/

	//*
	printf("Removing toto from group1\n");
	database.remove_user_from_group("toto", "group1");
	//*/

	printf("Is toto part of group1: %d\n", database.is_user_part_of_group("toto", "group1"));

	//*
	printf("Delete toto\n");
	database.delete_user("toto");
	//*/
	return 0;
}
