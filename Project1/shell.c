#include "20181592.h"

void help(void) {
	printf("h[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp] [start, end]\n");
	printf("e[dit] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode mnemonic\n");
	printf("opcodelist\n");

	return;
}

void add_to_history(int num_of_token,char**str) {//str has the tokens which are tokenized inputs
	his_node*new = (his_node*)malloc(sizeof(his_node));//new node that will be added to linked list
	//put input from user to new->command
	strcpy(new->command, str[0]);
	if(num_of_token>1){
	strcat(new->command, " ");
	strcat(new->command, str[1]);
		for(int i=2;i<num_of_token;i++){
			strcat(new->command, ", ");
			strcat(new->command, str[i]);
		}
	}
	new->next = NULL;//it will be added to the end of the linked list

	his_node*i;
	for (i = head; i->next != NULL; i = i->next);//find the end of the linked list

	i->next = new;

	return;
}

void history(void) {
	int num = 1;
	for (his_node*i = head->next; i != NULL; i = i->next, num++)
		printf("%-5d %s\n", num, i->command);

	return;
}


void dir(void) {
	int ret=0;
	int i;
	struct dirent**filelist;
	struct stat sb;

	if((ret=scandir(".",&filelist,NULL,alphasort))==-1)
		printf("Error while scanning the directory\n");

	char*tok;
	for(i=0;i<ret;i++){
		stat(filelist[i]->d_name,&sb);
		switch(sb.st_mode & S_IFMT){
			case S_IFDIR: tok="/";
						  break;
			case S_IFREG: if((sb.st_mode & S_IEXEC)==S_IXUSR)
							  tok="*";
						  else
							  tok="";
						  break;
			default: tok="";
					 break;
		}
		strcat(filelist[i]->d_name,tok);
		printf("%-20s",filelist[i]->d_name);
		if(i%4==3)
			printf("\n");
	}
	if(i%4!=0)
		printf("\n");

	return;
}
