#ifndef __20181592_H__
#define __20181592_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>

#define INPUT_LENGTH 200
#define MAX_LENGTH_OF_TOKEN 50//make tokens by spliting the input by ',' or ' '.
#define MAX_NUM_OF_TOKEN 4
#define HASH_VALUE_NUM 20//num of hash table values

int memory[65536][16];//virtual memory for shell
int dump_address;//for dump command. store the last memory address.
int valid_command;//1: input is a valid command with approprate parameters. 0: invalid.

typedef struct his_node{//for history linked list
	char command[50];//input from user
	struct his_node*next;
}his_node;
his_node*head;//head of the linked list for history

//make a hash table for opcode
typedef struct hash_node{
	char opcode[3];
	char mnemonic[MAX_LENGTH_OF_TOKEN];
	int format;//1,2,3. 3 means 3/4
	struct hash_node*next;
}hash_node;
hash_node*hash_table[HASH_VALUE_NUM];//hash table for opcode

//in main.c
int split_command(char*input,char***split);//split the input into tokens(command and parameters). return the number of parameters

//in shell.c
void help(void);//print every command
void dir(void);//print every file in the directory
void history(void);//print history
void add_to_history(int num_of_token,char**token);//add a command to the history linked list

//in memory.c
void dump_1(void);//dump command with no parameters
void dump_2(char*start);//dump command with 1 parameter
void dump_3(char*start, char*end);//dump command with 2 parameters
void print_dump(int start,int end);//print memory from start to end
int char_to_hex(char*str);//convert char string to hexadecimal(int)
void edit(char*address, char*value);//edit command
void fill(char*start, char*end, char*value);//fill command
void reset(void);//reset command

//in opcode.c
void init_hash_table(void);//read data from opcode.txt and initiate hash table
int hash_func(char*key);//hash function. return value for key
void insert_hash_table(int value,hash_node**new_node);//insert a node to hash table
void opcode_mnemonic(char*mnem);//print the opcode of mnem
void opcode_list();//print opcode hash table

#endif
