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

	MongoDatabase database;

	printf("PING %s\n", database.ping() ? "success" : "fail");

	printf("Generating key\n");
	uint8_t key[] = {'h', 'u', 'n', 't', 'e', 'r', '2'};

	printf("Creating user toto\n");
	database.create_user<KEY_LENGTH>("toto", key);

	printf("Is toto part of group1: %d\n", database.is_user_part_of_group("toto", "group1"));

	printf("Adding toto to group1\n");
	database.add_user_to_group("toto", "group1");

	//*
	printf("Is toto part of group1: %d\n", database.is_user_part_of_group("toto", "group1"));
	//*/

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

