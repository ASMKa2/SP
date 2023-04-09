#include "20181592.h"

int main(void) {
	//Initialization
	char input[INPUT_LENGTH];//input from user
	char**token=(char**)malloc(sizeof(char*)*MAX_NUM_OF_TOKEN);//for input split by ' '->get the command
	//initialize the 1Mbyte memory to 0
	for(int i=0;i<MAX_NUM_OF_TOKEN;i++)
		token[i]=(char*)malloc(sizeof(char)*(MAX_LENGTH_OF_TOKEN));
	for(int i=0;i<65536;i++)
		for(int j=0;j<16;j++)
			memory[i][j]=0;
	//Initialize the history linked list
	head=(his_node*)malloc(sizeof(his_node));
	head->next=NULL;
	//Initialization for assemble
	num_of_lines=0;
	num_of_symbols=0;
	asm_error=0;
	data=(line*)malloc(sizeof(line));
	symbol_table=(symbol_node*)malloc(sizeof(symbol_node));
	
	dump_address=0;//initialize dump address to 0
	init_hash_table();//hash_table_for_opcode

	while (1) {//loop till user quits
		valid_command=1;
		printf("sicsim > ");
		fgets(input,INPUT_LENGTH,stdin);
		if(strlen(input)!=0)
			input[strlen(input)-1]='\0';
		//split the command by ", "
		int num_of_token=split_command(input,&token);
		if(num_of_token>=MAX_NUM_OF_TOKEN+1){
			printf("A command can have max %d parameters.\n",MAX_NUM_OF_TOKEN-1);
			valid_command=0;
			continue;
		}
		//capital to small letter for command
		for(int i=0;i<(int)strlen(token[0]);i++){
			if(token[0][i]>=65 && token[0][i]<=90)
				token[0][i]+=32;
		}
		//select by token
		if (!strcmp(token[0], "type")) {
			if(num_of_token==2)
				type(token[1]);
			else{
				printf("Error: \"type\" command needs 1 parameter. (filename)\n");
				valid_command=0;
			}
		}
		else if(!strcmp(token[0], "assemble")){
			if(num_of_token==2)
				assemble(token[1]);
			else{
				printf("Error: \"assemble\" command needs 1 parameter. (filename)\n");
				valid_command=0;
			}
		}
		else if(!strcmp(token[0], "symbol")){
			if(num_of_token==1)
				print_symbol_table();
			else{
				printf("Error: \"symbol\" command does not need a parameter.\n");
				valid_command=0;
			}
		}
		else{
			//capital to small letter for parameters
			for(int i=0;i<num_of_token;i++){
				for(int j=0;j<(int)strlen(token[i]);j++){
					if(token[i][j]>=65 && token[i][j]<=90)
						token[i][j]+=32;
				}
			}
			if (!strcmp(token[0], "h") || !strcmp(token[0], "help")) {
				if(num_of_token==1)
					help();
				else{
					printf("Error: \"h[elp]\" command does not need a parameter.\n");
					valid_command=0;
				}
			}
			else if (!strcmp(token[0], "d") || !strcmp(token[0], "dir")) {
				if(num_of_token==1)
					dir();
				else{
					printf("Error: \"d[ir]\" command does not need a parameter.\n");
					valid_command=0;
				}
			}
			else if (!strcmp(token[0], "q") || !strcmp(token[0], "quit")) {//end the program
				if(num_of_token==1)
					break;
				else{
					valid_command=0;
					printf("Error: \"q[uit]\" command does not need a parameter.\n");
				}
			}
			else if (!strcmp(token[0], "hi") || !strcmp(token[0], "history")) {
				if(num_of_token==1){
					add_to_history(num_of_token,token);//in different cases, add_to_history is called at the end of the loop, but when history is the command, we have to add_to_history and then call history().
					history();
					continue;//To skip calling add_to_history()
				}
				else{
					printf("Error: \"h[istory]\" command does not need a parameter.\n");
					valid_command=0;
				}
			}
			else if (!strcmp(token[0], "du") || !strcmp(token[0], "dump")) {
				switch(num_of_token){
					case 1: dump_1();
							break;
					case 2: dump_2(token[1]);
							break;
					case 3: dump_3(token[1],token[2]);
							break;
					case 4: printf("Error: \"du[mp]\" command can have 0~2 parameters.\n");
							valid_command=0;
							break;
				}
			}
			else if (!strcmp(token[0], "e") || !strcmp(token[0], "edit")) {
				if(num_of_token==3)
					edit(token[1],token[2]);
				else{
					printf("Error: \"e[dit]\" command needs 2 parameters.(address, value)\n");
					valid_command=0;
				}
			}
			else if (!strcmp(token[0], "f") || !strcmp(token[0], "fill")) {
				if(num_of_token==4)
					fill(token[1],token[2],token[3]);
				else{
					printf("Error: \"f[ill]\" command needs 3 parameters.(start, end, value)\n");
					valid_command=0;
				}
			}
			else if (!strcmp(token[0], "reset")) {
				if(num_of_token==1)
					reset();
				else{
					printf("Error: \"reset\" command does not need a parameter.\n");
					valid_command=0;
				}
			}
			else if (!strcmp(token[0], "opcode")) {
				if(num_of_token==2)
					opcode_mnemonic(token[1]);
				else{
					printf("Error: \"opcode\" command needs 1 parameter. (mnemonic)\n");
					valid_command=0;
				}
			}
			else if (!strcmp(token[0], "opcodelist")) {
				if(num_of_token==1)
					opcode_list();
				else{
					printf("Error: \"opcodelist\" command does not need a parameter.\n");
					valid_command=0;
				}
			}
			else {
				printf("Error: Invalid command! Use h[elp] command for help.\n");
				valid_command=0;
			}
		}
		if(valid_command)
			add_to_history(num_of_token,token);
	}

	return 0;
}

int split_command(char*input,char***split){//tokenize input by ", " to split
	char*token;
	char*strptr;
	int i=0;//number of tokens
	token=strtok_r(input,", ",&strptr);
	while(token && i<MAX_NUM_OF_TOKEN){
		strcpy((*split)[i],token);
		i++;
		token=strtok_r(NULL,", ",&strptr);
	}
	
	if(token!=NULL)//executed while loop 4 times, and still token is not NULL: too much parameters
		return MAX_NUM_OF_TOKEN+1;

	return i;
}
