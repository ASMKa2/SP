#include "20181592.h"

void set_progaddr(char*address){
	progaddr=char_to_hex(address);
	if(valid_command==0){//check if address is valid
		printf("Error: Invalid parameter.\n");
		return;
	}
	//check if address is in range(0X00000~0XFFFFF)
	if(progaddr<0 || progaddr>0XFFFFF){
		valid_command=0;
		printf("Error: Address is out of range.\n");
	}

	return;
}

void load(int num_of_files,char**filename){
	size_of_estab=0;
	int i,j,k;

	//read files and save to int***obj_file
	FILE*fp;
	int*num_of_lines=(int*)malloc(sizeof(int)*num_of_files);
	for(i=0;i<num_of_files;i++){
		fp=fopen(filename[i],"r");
		if(fp==NULL){//check file open error
			valid_command=0;
			printf("Error: File open error for %s.\n",filename[i]);
			return;
		}
		obj_file=(char***)realloc(obj_file,sizeof(char**)*(i+1));
		obj_file[i]=(char**)malloc(sizeof(char*));
		//read file
		for(j=0;;j++){
			obj_file[i]=(char**)realloc(obj_file[i],sizeof(char*)*(j+1));
			obj_file[i][j]=(char*)malloc(sizeof(char)*(MAX_LENGTH_OF_OBJ_LINE*2+14));
			fgets(obj_file[i][j],MAX_LENGTH_OF_OBJ_LINE*2+14,fp);
			if(feof(fp))
				break;
			obj_file[i][j][(int)strlen(obj_file[i][j])-1]='\0';
		}
		num_of_lines[i]=j;
		fclose(fp);
	}

	//PASS 1
	int csaddr=progaddr;//control section address
	int cslth;//length of control section

	char control_section[7];//for control section name
	char symbol_name[7];//for symbol name
	char str[7];//used as temp
	int address;
	int found;
	int length[3];
	
	for(i=0;i<num_of_files;i++){
		//read H record, set cslth
		if(obj_file[i][0][0]!='H'){
			valid_command=0;
			printf("Error: First record of object file is not a header record.\n");
			break;
		}
		else{
			strncpy(symbol_name,obj_file[i][0]+1,6);
			symbol_name[6]='\0';
			strtok(symbol_name," ");
			strncpy(control_section,symbol_name,6);
			control_section[6]='\0';
			strncpy(str,obj_file[i][0]+13,6);
			str[6]='\0';
			strtok(str," ");
			cslth=char_to_hex(str);
			length[i]=cslth;
		}
		//search estab for control section name
		search_estab(symbol_name,&found);
		if(found){
			valid_command=0;
			printf("Error: Duplicate symbols.\n");
			break;
		}
		else{
			//insert to estab
			insert_to_estab(symbol_name,csaddr,control_section);
		}
		for(j=1;j<num_of_lines[i];j++){
			//read record
			//if D record
			if(obj_file[i][j][0]=='D'){
				//for each symbol in record
				for(k=1;k+12<=(int)strlen(obj_file[i][j]);k+=12){
					strncpy(symbol_name,obj_file[i][j]+k,6);
					symbol_name[6]='\0';
					strtok(symbol_name," ");
					//search estab for symbol name
					search_estab(symbol_name,&found);
					if(found){
						valid_command=0;
						printf("Error: Duplicate symbols.\n");
						break;
					}
					else{
						//insert to estab with csaddr+address
						strncpy(str,obj_file[i][j]+k+6,6);
						str[6]='\0';
						strtok(str," ");
						address=char_to_hex(str);
						insert_to_estab(symbol_name,csaddr+address,control_section);
					}
				}
			}
			//if E record break;
			if(obj_file[i][j][0]=='E')
				break;
			if(valid_command==0)
				break;
		}
		csaddr+=cslth;
		if(valid_command==0)
			break;
	}

	if(valid_command==0){
		for(i=0;i<num_of_files;i++){
			for(j=0;j<num_of_lines[i];j++){
				free(obj_file[i][j]);
			}
			free(obj_file[i]);
		}
		free(num_of_lines);
		return;
	}
	
	//PASS 2
	
	csaddr=progaddr;
	int index;//for reference number
	int execaddr;//address of the first executable instruction
	execaddr=progaddr;
	for(i=0;i<num_of_files;i++){
		size_of_ref_num=0;
		//read Header record
		cslth=length[i];
		//insert reference number 01 to ref_num
		ref_num=(reference_number*)realloc(ref_num,sizeof(reference_number)*(size_of_ref_num+1));
		ref_num[size_of_ref_num].index=1;
		strncpy(symbol_name,obj_file[i][0]+1,6);
		symbol_name[6]='\0';
		strtok(symbol_name," ");
		ref_num[size_of_ref_num++].address=search_estab(symbol_name,&found);
		if(!found){
			valid_command=0;
			printf("Error: Invalid symbol.\n");
		}
		for(j=1;j<num_of_lines[i];j++){
			//read next input record
			//if R record
			if(obj_file[i][j][0]=='R'){
				for(k=1;k+2<(int)strlen(obj_file[i][j]);k+=8){
					strncpy(str,obj_file[i][j]+k,2);
					str[2]='\0';
					index=char_to_hex(str);
					strncpy(symbol_name,obj_file[i][j]+k+2,6);
					symbol_name[6]='\0';
					strtok(symbol_name," ");
					//search estab for symbol name
					address=search_estab(symbol_name,&found);
					if(found){
						//save each symbol to ref_num
						ref_num=(reference_number*)realloc(ref_num,sizeof(reference_number)*(size_of_ref_num+1));
						ref_num[size_of_ref_num].index=index;
						ref_num[size_of_ref_num++].address=address;
					}
					else{
						valid_command=0;
						printf("Error: Invalid external reference.\n");
					}
				}
			}
			//if T record
			if(obj_file[i][j][0]=='T'){
				int length_of_T;
				int num;
				strncpy(str,obj_file[i][j]+1,6);
				str[6]='\0';
				address=char_to_hex(str);
				strncpy(str,obj_file[i][j]+7,2);
				str[2]='\0';
				length_of_T=char_to_hex(str);
				//place object code to memory
				for(k=0;k<length_of_T;k++){
					strncpy(str,obj_file[i][j]+9+k*2,2);
					str[2]='\0';
					num=char_to_hex(str);//convert character into binary
					memory[(csaddr+address+k)/16][(csaddr+address+k)%16]=num;
				}
			}
			//if M record
			if(obj_file[i][j][0]=='M'){
				int length_of_mod;
				int amount_of_mod;
				char add_or_sub;
				int num;
				strncpy(str,obj_file[i][j]+1,6);
				str[6]='\0';
				address=char_to_hex(str)+csaddr;
				strncpy(str,obj_file[i][j]+7,2);
				str[2]='\0';
				length_of_mod=char_to_hex(str);
				add_or_sub=obj_file[i][j][9];
				strncpy(str,obj_file[i][j]+10,2);
				str[2]='\0';
				index=char_to_hex(str);
				//search number from ref_num
				amount_of_mod=search_ref_num(index,&found);
				//if found
				if(found){
					num=0;
					//add or subtract symbol value at loacation
					for(k=(length_of_mod-1)/2;k>=0;k--){
						num+=memory[(address+k)/16][(address+k)%16]*pow(256,(length_of_mod-1)/2-k);
					}
					if(add_or_sub=='+')
						num+=amount_of_mod;
					else if(add_or_sub=='-')
						num-=amount_of_mod;
					for(k=(length_of_mod-1)/2;k>=0;k--){
						memory[(address+k)/16][(address+k)%16]=num%256;
						num/=256;
					}
				}
				//else
				else{
					//error
					valid_command=0;
					printf("Error: failed to find symbol.\n");
				}
				if(valid_command==0)
					break;
			}
			//if E record
			if(obj_file[i][j][0]=='E'){
				if(obj_file[i][j][1]!='\0'){
					strncpy(str,obj_file[i][j]+1,6);
					str[6]='\0';
					address=char_to_hex(str);
					execaddr=address;
				}
				break;
			}
			if(valid_command==0)
				break;
		}
		if(valid_command==0)
			break;
		csaddr+=cslth;
	}

	if(valid_command==0){
		for(i=0;i<num_of_files;i++){
			for(j=0;j<num_of_lines[i];j++){
				free(obj_file[i][j]);
			}
			free(obj_file[i]);
		}
		free(num_of_lines);
		return;
	}

	//print load map
	int total_length=0;
	printf("control symbol address length\n");
	printf("section name\n");
	printf("---------------------------\n");
	for(i=0,j=0;i<size_of_estab;i++){
		if(!strcmp(estab[i].symbol,estab[i].control_section))
			printf("%-13s",estab[i].symbol);
		else
			printf("%13s",estab[i].symbol);
		printf("   %04X",estab[i].address);
		if(!strcmp(estab[i].symbol,estab[i].control_section)){
			printf("  %04X\n",length[j]);
			total_length+=length[j++];
		}
		else
			printf("\n");
	}
	printf("---------------------------\n");
	printf("   	 total length %04X\n",total_length);

	//set registers for run
	for(i=0;i<10;i++)
		reg[i]=0;
	reg[register_number("PC")]=execaddr;
	for(i=0;i<num_of_files;i++)
		reg[register_number("L")]+=length[i];

	//free
	for(i=0;i<num_of_files;i++){
		for(j=0;j<num_of_lines[i];j++){
			free(obj_file[i][j]);
		}
		free(obj_file[i]);
	}
	free(num_of_lines);

	return;
}

int search_estab(char*symbol_name,int*found){
	*found=0;
	
	int i;
	for(i=0;i<size_of_estab;i++){
		if(!strcmp(estab[i].symbol,symbol_name)){
			*found=1;
			break;
		}
	}

	if(*found)
		return estab[i].address;
	else
		return 0;
}

void insert_to_estab(char*symbol_name,int address,char*control_section){
	estab=(external_symbol*)realloc(estab,sizeof(external_symbol)*(++size_of_estab));
	strcpy(estab[size_of_estab-1].symbol,symbol_name);
	strcpy(estab[size_of_estab-1].control_section,control_section);
	estab[size_of_estab-1].address=address;

	return;
}

int search_ref_num(int index,int*found){
	for(int i=0;i<size_of_ref_num;i++){
		if(index==ref_num[i].index){
			*found=1;
			return ref_num[i].address;
		}
	}

	return 0;
}
