#include "20181592.h"

void init_hash_table(void) {
	for (int i = 0; i < HASH_VALUE_NUM; i++) {
		hash_table[i] = (hash_node*)malloc(sizeof(hash_node));
		hash_table[i]->next = NULL;
	}

	FILE*fp = fopen("opcode.txt", "r");

	char opcode[3];
	char mnemonic[MAX_LENGTH_OF_TOKEN];
	char format_str[4];//for "3/4"
	int format;

	int value;
	while (1) {
		fscanf(fp, "%s %s %s", opcode, mnemonic, format_str);
		if (feof(fp))
			break;
		format = atoi(&(format_str[0]));

		hash_node*new = (hash_node*)malloc(sizeof(hash_node));
		new->next = NULL;
		strcpy(new->opcode, opcode);
		strcpy(new->mnemonic, mnemonic);
		new->format = format;

		//hash function
		value = hash_func(mnemonic);
		insert_hash_table(value, &new);
	}

	return;
}

int hash_func(char*key) {
	int value = 0;
	for(int i=0;i<(int)strlen(key);i++)
		value += key[i];
	value %= 20;

	return value;
}

void insert_hash_table(int value, hash_node**new) {//insert new to hash table

	hash_node*i = hash_table[value];

	for (; i->next != NULL; i = i->next);

	i->next = *new;

	return;
}

void opcode_mnemonic(char*mnem) {

	for (int i = 0; i < (int)strlen(mnem); i++)//convert small letter to capital
		if(mnem[i]>=97 && mnem[i]<=122)
			mnem[i] -= 32;

	int value = hash_func(mnem);
	hash_node*i = hash_table[value]->next;
	for (; i != NULL; i = i->next){
		if (!strcmp(i->mnemonic, mnem))
			break;
	}

	if (i == NULL){
		printf("Error: Invalid mnemonic. Type opcodelist to check opcodelist.\n");
		valid_command=0;
	}
	else
		printf("opcode is %s\n", i->opcode);

	return;
}

void opcode_list(void) {
	hash_node*j;
	for (int i = 0; i < 20; i++) {
		printf("%d : ", i);
		for (j = hash_table[i]->next; j != NULL; j = j->next) {
			printf("[%s,%s]", j->opcode, j->mnemonic);
			if (j->next != NULL)
				printf(" -> ");
		}
		printf("\n");
	}

	return;
}
