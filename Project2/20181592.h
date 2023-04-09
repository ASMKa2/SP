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

#define FORMAT_CONSTANT 5
#define FORMAT_VARIABLE 6
#define FORMAT_COMMENT 7
#define MAX_LENGTH_OF_OBJ_LINE 29

int memory[65536][16];//virtual memory for shell
int dump_address;//for dump command. store the last memory address.
int valid_command;//1: input is a valid command with approprate parameters. 0: invalid.
int asm_error;

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

//to store informations for .asm, .lst, and .obj files
typedef struct line{
	int line_num;
	int loc;
	char symbol[31];
	char mnem[31];
	char operands[33];
	int obj_code;
	int format;//format of mnem. 5: constant 6: variable 7: nothing
}line;
line*data;
int num_of_lines;//num of lines of the asm file

//to store the symbol table generated during assembling
typedef struct symbol_node{
	char symbol[31];
	int loc;
}symbol_node;
symbol_node*symbol_table;//symbol_table
int num_of_symbols;

//in main.c
int split_command(char*input,char***split);//split the input into tokens(command and parameters). return the number of parameters

//in shell.c
void help(void);//print every command
void dir(void);//print every file in the directory
void history(void);//print history
void add_to_history(int num_of_token,char**token);//add a command to the history linked list
void type(char*filename);//read file in the directory and print

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

//in assemble.c
void assemble(char*filename);//assemble file named filename. generate .lst and .obij.
int find_format(char*mnem);//return format for a mnemonic. 5: constant 6: variable 7: comment
int calculate_loc(line data);//calculate location for line. return the amount of incresement of location
int find_opcode(char*mnem, int*found);//return opcdoe for a mnemonic. found is 1 if found mnemonic in the symbol table, 0 if not.
int register_number(char*mnem);//return SIC/XE register number for mnemonic.
int find_location(char*symbol,int*found);//find location for symbol. if it is not in the symbol table, check if it is digits, and if it is, return the integer value for symbol.
void insert_to_symbol_table(line data);//insert symbol(and location) to symbol table.
int generate_obj_code(int opcode,line data,int addr,int*pc,int*base,int symbol_flag);//generate object code
void print_symbol_table(void);//print the most recent symbol table generated during assembling
char*hex_to_char(int hex,int length);//return a string with given length and value is the hexadecimal.
#endif
