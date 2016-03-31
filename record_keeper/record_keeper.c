/* john-ko */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <sys/types.h>

/****************
     STRUCTS
*****************/

/* record (node) */
struct record {
	char *name;
	char *address;
	char *phone_number;
	struct record *next; /* pointer to next node */
};

/* record_keeper (linked list) */
struct record_keeper {
	int size;								/* size */
	struct record *records; /* linked list head */
};

/* command */
struct command {
	char command[100];
	char value[100];
};
/************************
    HELPER FUNCTIONS
************************/

/**
 * print_menu
 */
void print_menu()
{
	puts("=============");
	puts("    lab 1    ");
	puts("-------------");
	puts("commands:");
	puts("  read [file]\t\treads in file");
	puts("  write [file]\t\twrites to file");
	puts("  print      \t\tprints contents");
	puts("  delete [num]\t\tdeletes record");
	puts("  quit       \t\texits");
}

/**
 * record_keeper_factory()
 * constructs the record_keeper object (struct)
 * and defaults the size to zero
 * returns struct record_keeper pointer
 */
struct record_keeper *record_keeper_factory()
{
	struct record_keeper *rc = malloc(sizeof(struct record_keeper));
	rc->size = 0;
	rc->records = NULL;
	return rc;
}

/**
 * delete_record_keeper()
 * helper function to handle deletion of
 * dynamically allocated memory
 */
void delete_record_keeper(struct record_keeper *record_keeper)
{
	if (record_keeper) {
		if (record_keeper->size != 0) {
			free(record_keeper->records);
		}
		free(record_keeper);
	}
}

/**
 * record_factory
 * returns a record
 *
 * Return:
 * struct record pointer
 */
struct record *record_factory()
{
	struct record *record = malloc(sizeof(struct record));
	record->next = NULL;

	return record;
}

/**
 * delete_record
 * frees up a record node memory
 *
 * Params:
 * struct record pointer
 */
void delete_record(struct record * record)
{
	if (record->name) {
		free(record->name);
	}

	if (record->address) {
		free(record->address);
	}

	if (record->phone_number) {
		free(record->phone_number);
	}

	free(record);
}

/**
 * command_factory
 * generates command struct
 *
 * Return:
 * struct command
 */
struct command *command_factory()
{
	struct command *cmd = malloc(sizeof(struct command));
	return cmd;
}

/**
 * delete_command
 * frees up the command
 *
 * Params:
 * struct command pointer
 */
void delete_command(struct command *command)
{
	free(command);
}

/**
 * to_lower
 * lowercases the string
 * if newline character is found, replace with null terminator
 * and return current string
 * 
 * Params:
 * char pointer string
 */
void to_lower(char *string)
{
	for(int i = 0; string[i]; i++){

		if (string[i] == '\n') {
			string[i] = '\0';
			return;
		}

		string[i] = tolower(string[i]);
	}
}

/**
 * get_command
 * helper function to get the string command
 *
 * Params:
 * char pointer cmd
 */
void parse_user_input(char *cmd)
{
	fgets(cmd, 100, stdin);
	to_lower(cmd);
}

/**
 * parse_command
 * parses the command and puts it into command struct
 *
 * Params:
 * struct command pointer command
 * const char pointer user_input
 */
void parse_command(struct command *command, const char *user_input)
{
	char cmd_copy[100];
	char *temp;

	strcpy(cmd_copy, user_input);
	strcpy(command->command, strtok(cmd_copy, " "));

	// temp variable to check if strtokenize returns null
	temp = strtok(NULL, "\0");
	if (temp) {
		strcpy(command->value, temp);
	}
}

/**
 * check_command
 * checks for valid commands and returns a number
 * returns 0 if not valid command
 * and -1 for quit
 *
 * Params:
 * const char pointer cmd
 *
 * Return:
 * int
 */
int check_command(const char *cmd)
{
	char cmd_copy[100]; /* copy of cmd(mutable) */
	char *command; /* just command portion of cmd */

	strcpy(cmd_copy, cmd);
	command = strtok(cmd_copy, " ");

	if (strcmp(command, "read") == 0) {
		return 1;
	} else if (strcmp(command, "write") == 0) {
		return 2;
	} else if (strcmp(command, "print") == 0) {
		return 3;
	} else if (strcmp(command, "delete") == 0) {
		return 4;
	} else if (strcmp(command, "quit") == 0) {
		return -1;
	} else {
		return 0;
	}
}

/**
 * parse_record
 * parses the recorder base on the input line of the file
 *
 * Params;
 * struct record pointer
 * char pointer (line of a file)
 */
void parse_record(struct record *record, char *line)
{
	char *line_copy = malloc(strlen(line) + 1);
	char *temp;
	strcpy(line_copy, line);

	temp = strtok(line_copy, "\t");

	// name
	if (temp) {
		record->name = malloc(strlen(temp) + 1);
		strcpy(record->name, temp);
	}

	// address
	temp = strtok(NULL, "\t");
	if (temp) {
		record->address = malloc(strlen(temp) + 1);
		strcpy(record->address, temp);
	}

	// phone_number
	temp = strtok(NULL, "\n");
	if (temp) {
		record->phone_number = malloc(strlen(temp) + 1);
		strcpy(record->phone_number, temp);
	}
}

/**
 * append_record
 * appends record to the record keeper
 * to abstract away some code
 *
 * Params:
 * struct record_keeper pointer
 * struct record pointer
 */
void append_record(struct record_keeper *record_keeper, struct record* record)
{
	struct record *temp_record;

	// check if record is null or not
	if (record_keeper->records) {

		temp_record = record_keeper->records;
		while(temp_record->next) {
			temp_record = temp_record->next;
		}

		temp_record->next = record;
	} else {
		record_keeper->records = record;
	}

	record_keeper->size++;
}

/**
 * delete_all
 * clears and frees the record keeper struct
 *
 * Params:
 * struct record_keeper pointer
 */
void delete_all(struct record_keeper *record_keeper)
{
	struct record *victim;
	struct record *record = record_keeper->records;

	while(record) {
		victim = record;
		record = record->next;

		delete_record(victim);
	}

	record_keeper->size = 0;
	record_keeper->records = NULL;
}

/**********************
		MAIN COMMANDS
**********************/

/**
 * read
 * reads in file string
 *
 * Params:
 * struct record_keeper pointer
 * char pointer
 */
void read(const struct command *command, struct record_keeper *record_keeper)
{
	FILE *file;
	char *mode = "r";
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	delete_all(record_keeper);

	file = fopen(command->value, mode);

	if ( ! file) {
		printf("an error occured while opening\n");
		return;
	}

	// read in file
	while ((read = getline(&line, &len, file)) != -1) {
		struct record *record = record_factory();
		parse_record(record, line);
		append_record(record_keeper, record);
	}

	fclose (file);
}

/**
 * write
 * writes to file
 *
 * Params:
 * const struct command pointer
 * const struct record_keeper pointer
 */
void write(const struct command *command, const struct record_keeper *record_keeper)
{
	struct record *record = record_keeper->records;
	FILE *file = fopen(command->value, "w");

	while(record) {
		fprintf(file, "%s	%s	%s\n", record->name, record->address, record->phone_number);
		record = record->next;
	}

	fclose(file);
}

/**
 * print
 * outputs data in record keeper
 * 
 * Params:
 * const struct record_keeper pointer
 */
void print(const struct record_keeper *record_keeper)
{
	struct record *record;
	int count = 1;
	record = record_keeper->records;
	while (record) {
		printf("%d. %s	%s	%s\n", count, record->name, record->address, record->phone_number);
		count++;
		record = record->next;
	}
}

/**
 * delete
 * deletes a specific node
 *
 * Params:
 * const struct command pointer
 * const struct record_keeper pointer
 */
void delete(const struct command *command, struct record_keeper *record_keeper)
{
	int number = atoi(command->value);
	int counter = 1;
	struct record *victim;
	struct record *previous = record_keeper->records;
	struct record *record = record_keeper->records;

	// checks if number is beyond size
	if (record_keeper->size < number || number < 0) {
		puts("Error you cannot delete beyond the size");
		return;
	}

	// checks if only 1
	if (record_keeper->size == 1 && number == counter) {
		victim = record_keeper->records;
		record_keeper->records = NULL;
		record_keeper->size = 0;
		delete_record(victim);
		return;
	}

	// go down the records
	while(record) {
		if (counter == number) {
			victim = record;

			// checks if he first one only
			if (counter == 1) {
				record_keeper->records = victim->next;
			} else {
				previous->next = victim->next;
			}
			delete_record(victim);
			record_keeper->size--;
			return;
		}

		previous = record;
		record = record->next;
		counter++;
	}
}

/**
 * event_loop
 * main event loop, just runs the command given
 *
 * Params:
 * struct record_keeper pointer
 */
int event_loop(struct record_keeper *record_keeper)
{
	printf("Enter command: ");
	char *user_input = malloc(sizeof(char)*100);
	struct command *command = command_factory();

	parse_user_input(user_input);
	parse_command(command, user_input);

	int number = check_command(command->command);

	switch(number) {

		// READ
		case 1:
			read(command, record_keeper);
		break;

		// WRITE
		case 2:
			write(command, record_keeper);
		break;

		// PRINT
		case 3:
			print(record_keeper);
		break;

		// DELETE
		case 4:
			delete(command, record_keeper);
		break;

		// QUIT
		case -1:
			delete_command(command);
			delete_all(record_keeper);
			return 1;

		// 404 not found
		default:
			puts("unknown command");
	}

	delete_command(command);
	return 0;
}

/**
 * run
 * runs the whole thing
 */
void run()
{
	struct record_keeper *records = record_keeper_factory();
	
	while(event_loop(records) != 1)
	{
		//event_loop will handle everything :)
	}
	delete_record_keeper(records);

	puts("exiting\n");

}

/**
 * main
 * int main!
 */
int main()
{
	print_menu();
	run();
	return 0;
}

