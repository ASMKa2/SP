#include "20181592.h"

void new_break_point(char*address){
	int hex_address=char_to_hex(address);
	if(valid_command==0){
		printf("Error: Invalid parameter.\n");
		return;
	}
	if(hex_address<0 || hex_address>0XFFFFFF){
		valid_command=0;
		printf("Error: Address out of range.\n");
		return;
	}

	bp_node*i;
	for(i=bp_head;i->next!=NULL;i=i->next);

	bp_node*new;
	new=(bp_node*)malloc(sizeof(bp_node));
	new->address=hex_address;
	new->next=NULL;
	i->next=new;

	printf("	[ok] create breakpoint %s\n",address);

	return;
}

void clear_break_point(void){
	bp_node*i;
	for(i=bp_head->next;i!=NULL;i=i->next){
		free(bp_head);
		bp_head=i;
	}
	printf("	[ok] clear all breakpoints\n");

	return;
}

void print_break_point(void){
	printf("	breakpoint\n");
	printf("	----------\n");
	for(bp_node*i=bp_head->next;i!=NULL;i=i->next)
		printf("	%X\n",i->address);

	return;
}

int check_break_point(void){
	bp_node*i;
	for(i=bp_head->next;i!=NULL;i=i->next){
		if(i->address==reg[8])//reg[8]:PC
			break;
	}

	if(i!=NULL)
		return 1;
	else
		return 0;
}
