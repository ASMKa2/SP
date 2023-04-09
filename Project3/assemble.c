#include "20181592.h"

void assemble(char*filename){
	asm_error=0;
	//check if the filename is .asm
	char*check=(char*)malloc(sizeof(char)*(int)strlen(filename));
	char*strptr;
	strcpy(check,filename);//use strcpy to preserve the original filename
	check=strtok_r(check,".",&strptr);//split by '.'
	if(strcmp(strptr,"asm")){
		valid_command=0;
		printf("%s is not an assemble file\n",filename);
		return;
	}
	free(check);

	//open file
	FILE*fp=fopen(filename,"r");
	if(fp==NULL){
		valid_command=0;
		printf("No file named %s in the directory.\n",filename);
		return;
	}
	
	//=======start of PASS 1==========

	int locctr=0;//location counter
	num_of_lines=0;//number of lines in the .asm file
	num_of_symbols=0;//number of symbols in the .asm file
	char**input=(char**)malloc(sizeof(char*));//store each line in the .asm line
	char first;//for the first character in each line
	char strptr2[32];//for spliting input
	char*tok;//for spliting input

	int start_address;

	//calculate location, format
	int i;
	for(i=0;;i++){
		//read lines
		input=(char**)realloc(input,sizeof(char*)*(i+1));//memory allocation
		input[i]=(char*)malloc(sizeof(char)*200);
		fgets(input[i],200,fp);//read a line from the .asm file
		if(feof(fp))//end of the file
			break;
		if((int)strlen(input[i])>0)
			input[i][(int)strlen(input[i])-1]='\0';
		data=(line*)realloc(data,sizeof(line)*(i+1));//memory allocation
		(data+i)->line_num=5*(i+1);//calculate line number
		first=input[i][0];
		if((first>='0' && first<='9')||(first>='A' && first<='Z')||(first>='a' && first<='z')){//if the line has a symbol
			//split and store symbol, mnemonic for opcode, and operands
			tok=strtok_r(input[i]," ",&strptr);
			strcpy(data[i].symbol,tok);
			if(strcmp(strptr,"")){
				tok=strtok_r(NULL," ",&strptr);
				strcpy(data[i].mnem,tok);
				if(strcmp(strptr,"")){
					int j;
					for(j=0;j<(int)strlen(strptr);j++)
						if(strptr[j]!=' ')
							break;
					strcpy(strptr2,strptr+j);
					strcpy(data[i].operands,strptr2);
				}
			}
		}
		else if(first==' '){//if the line has no symbols
			//split and store mnemonic for opcode, and operands
			strcpy(data[i].symbol,"");
			tok=strtok_r(input[i]," ",&strptr);
			strcpy(data[i].mnem,tok);
			if(strcmp(strptr,"")){
				int j;
				for(j=0;j<(int)strlen(strptr);j++)
					if(strptr[j]!=' ')
						break;
				strcpy(strptr2,strptr+j);
				strcpy(data[i].operands,strptr2);
			}
		}
		else{//if it is a commnet line
			strcpy(data[i].symbol,".");
		}//comment
		data[i].format=find_format(data[i].mnem);//find format for the line
		if(i==0){//if it is the first line, initiate the location counter
			if(!strcmp(data[0].mnem,"START"))
				locctr=char_to_hex(data[0].operands);
			else
				locctr=0;
			start_address=locctr;//store the starting address(used when generating obj file)
		}
		if(data[i].format!=FORMAT_COMMENT && strcmp(data[i].symbol,"") && strcmp(data[i].symbol,".")){//if it has a symbol, insert to symbol table
			insert_to_symbol_table(data[i]);
		}
		if(data[i].format!=FORMAT_COMMENT){//if it is not a comment line(if there is an increasement for the location counter), calculate location counter
			locctr+=calculate_loc(data[i]);
		}
		data[i+1].loc=locctr;//store location. also for the comment lines.(used when calculating the size of object code for each line
		if(!strcmp(data[i].mnem,"END"))//if it is the last line, break;
			break;
		if(asm_error){//if error occured, end assembling
			valid_command=0;
			printf("Error in line %d.\n",data[i].line_num);
			return;
		}
	}
	num_of_lines=i+1;//store number of lines in the asm file.
	int program_length=locctr-start_address;//store program length. used when generating object file.
	fclose(fp);//close the asm file
	
	//=======end of PASS 1==========

	//sort symbol table(selection sort)
	int min;
	symbol_node tmp;
	for(int i=0;i<num_of_symbols;i++){
		min=i;
		for(int j=i+1;j<num_of_symbols;j++){
			if(strcmp(symbol_table[min].symbol,symbol_table[j].symbol)>0){
				min=j;
			}
		}
		//swap
		strcpy(tmp.symbol,symbol_table[min].symbol);
		strcpy(symbol_table[min].symbol,symbol_table[i].symbol);
		strcpy(symbol_table[i].symbol,tmp.symbol);
		tmp.loc=symbol_table[min].loc;
		symbol_table[min].loc=symbol_table[i].loc;
		symbol_table[i].loc=tmp.loc;
	}

	//=======start of PASS 2==========

	FILE*fp_lst,*fp_obj;//file pointers for .lst file and .obj file.
	char*name_with_no_extension=(char*)malloc(sizeof(char)*(int)strlen(filename));//to make file name for .lst file and .obj file
	char*lst_name=(char*)malloc(sizeof(char)*(int)strlen(filename));//filename of .lst file
	char*obj_name=(char*)malloc(sizeof(char)*(int)strlen(filename));//filename of .obj file
	//get the name without ".asm"
	strcpy(name_with_no_extension,filename);
	name_with_no_extension=strtok_r(name_with_no_extension,".",&strptr);
	//generate filename for .lst file and .obj file
	strcpy(lst_name,name_with_no_extension);
	strcpy(obj_name,name_with_no_extension);
	lst_name=strcat(lst_name,".lst");
	obj_name=strcat(obj_name,".obj");
	//open .lst and .obj file
	fp_lst=fopen(lst_name,"w");
	fp_obj=fopen(obj_name,"w");

	int length_of_obj_line=0;//length of the current record which will be printed to obj file
	int need_new_line=0;//flag if we meet a variable line, need to print in a new line to obj file. 1: yes 2: no

	//check if files are opened
	if(fp_lst==NULL){
		asm_error=1;
		valid_command=0;
		printf("Failed to open .lst file.\n");
		return;
	}
	if(fp_obj==NULL){
		asm_error=1;
		valid_command=0;
		printf("Failed to open .obj file.\n");
		return;
	}

	int loc;//location of the operand
	int addr;//the address(or display) field
	int found;//used when checking if a symbol or a mnemonic is found
	int pc,base;//program counter, base
	int opcode;//opcode of the mnemonic
	char**token=(char**)malloc(sizeof(char*)*3);//for the operands
	for(int i=0;i<3;i++)
		token[i]=(char*)malloc(sizeof(char)*30);
	char*obj_line=(char*)calloc(sizeof(char),266);//for the object line. 266=1+6+2+0xFF+1(text record max length)

	for(int i=0;i<num_of_lines;i++){
		if(strcmp(data[i].mnem,"END"))//if it is not end
			pc=data[i+1].loc;//set program counter
		if(data[i].format!=FORMAT_COMMENT){//if it is not comment line
			addr=0;
			opcode=find_opcode(data[i].mnem,&found);//find opcode for the memonic
			if(found){//if mnemonic is found
				strcpy(strptr,data[i].operands);//copy operands to preserve the original data
				split_command(strptr,&token);//split operands
				if(strcmp(data[i].operands,"")){//if the line has operands
					//find location of the operand in the symbol table(if it is number, calculate the number)
					if(token[0][0]=='#' || token[0][0]=='@')
						loc=find_location(token[0]+1,&found);
					else
						loc=find_location(token[0],&found);
					if(found){//if the operand is found in the symbol table or it is a number
						addr=loc;//set address field to the calculated location or number
					}
					else if(register_number(token[0])!=-1){//check if operand is register
						addr=0;
					}
					else{//invalid operand
						addr=0;
						asm_error=1;
					}
				}
				else{//if the line doesn't have operands
					addr=0;
					found=0;
				}
				//generate obj code
				data[i].obj_code=generate_obj_code(opcode,data[i],addr,&pc,&base,found);
			}
			else{//if mnemonic is not found
				data[i].obj_code=0;//we need this for the case of assembling several times
				if(!strcmp(data[i].mnem,"BYTE")){//if it is a byte constant
					switch(data[i].operands[0]){
						case 'C'://if it is character
							for(int j=2;j<(int)strlen(data[i].operands)-1;j++){
								data[i].obj_code+=data[i].operands[j];
								if(j<(int)strlen(data[i].operands)-2)
									data[i].obj_code=data[i].obj_code<<8;
							}
							break;
						case 'X'://if it is hexadecimal
							strcpy(strptr,data[i].operands+2);
							strptr[(int)strlen(strptr)-1]='\0';
							data[i].obj_code=char_to_hex(strptr);
							break;
						default://invalid operands
							asm_error=1;
							break;
					}
				}
				else if(!strcmp(data[i].mnem,"WORD")){//if it is a word constant
					switch(data[i].operands[0]){
						case 'C'://if it is character
							for(int j=2;j<(int)strlen(data[i].operands)-1;j++){
								data[i].obj_code+=data[i].operands[j];
								if(j<(int)strlen(data[i].operands)-2)
									data[i].obj_code=data[i].obj_code<<8;
							}
							break;
						case 'X'://if it is hexadecimal
							strcpy(strptr,data[i].operands+2);
							strptr[(int)strlen(strptr)-1]='\0';
							data[i].obj_code=char_to_hex(strptr);
							if(valid_command==0)//char_to_hex failed(invalid parameter)
								asm_error=1;
							break;
						default://invalid operands
							asm_error=1;
							break;
					}
				}
				else if(data[i].format!=FORMAT_VARIABLE){//if it is not variable, it is an invalid instruction
					asm_error=1;
				}
			}
			//generate obj line
			if(i==1 || (data[i].format!=FORMAT_VARIABLE && ((length_of_obj_line-9+data[i+1].loc-data[i].loc)/2)>MAX_LENGTH_OF_OBJ_LINE) || (data[i].format==FORMAT_VARIABLE && need_new_line)){//need next line
				//write Text record to obj
				if(i!=1){//i!=1 means we don't have to print Header record here
					char*str=hex_to_char((length_of_obj_line-9)/2,2);//add length of object code in this record in bytes
					obj_line[7]=str[0];
					obj_line[8]=str[1];
					fprintf(fp_obj,"%s\n",obj_line);//print the object record
					obj_line[0]='\0';
					need_new_line=0;//clear the flag
				}
				//initialize new Text record
				obj_line[0]='T';
				obj_line[1]='\0';
			}
			if(data[i].format!=FORMAT_VARIABLE){//if it's not a variable
				if(!strcmp(obj_line,"T")){//check if the line is empty, only initialized
					strcat(obj_line,hex_to_char(data[i].loc,6));//add starting address for object code in this record
					obj_line[7]=' ';
					obj_line[8]=' ';
					length_of_obj_line=9;//set
					obj_line[length_of_obj_line]='\0';
				}
				need_new_line=1;//if we meet a variable line, we need a new line
				strcat(obj_line,hex_to_char(data[i].obj_code,(data[i+1].loc-data[i].loc)*2));//add object code to current record
				length_of_obj_line+=((data[i+1].loc-data[i].loc)*2);//set
				obj_line[length_of_obj_line]='\0';
			}
		}
		if(!strcmp(data[i].mnem,"START")){//Header record
			obj_line[0]='H';
			obj_line[1]='\0';
			obj_line=strcat(obj_line,data[i].symbol);//add program name
			for(int i=(int)strlen(obj_line);i<7;i++)
				obj_line=strcat(obj_line," ");
			obj_line[7]='\0';
			strcat(obj_line,hex_to_char(start_address,6));//add starting address of object program
			strcat(obj_line,hex_to_char(program_length,6));//add length of object program in bytes
			obj_line[19]='\0';
			fprintf(fp_obj,"%s\n",obj_line);//print Header record
			obj_line[0]='\0';
		}
		if(!strcmp(data[i].mnem,"END")){//End record
			//print if text record is not empty
			if(strcmp(obj_line,"T")){
				//add length of object code in this record in bytes
				char*str=hex_to_char((length_of_obj_line-9)/2,2);
				obj_line[7]=str[0];
				obj_line[8]=str[1];
				fprintf(fp_obj,"%s\n",obj_line);//print the text record
				obj_line[0]='\0';
			}
			//print modification record
			for(int j=0;j<num_of_lines;j++){
				if(data[j].format==4){//format 4 need relocation
					strcpy(strptr,data[j].operands);//copy to preserve original data
					split_command(strptr,&token);//split operands
					//check if the first operand is a symbol
					if(token[0][0]=='#' || token[0][0]=='@')
						find_location(token[0]+1,&found);
					else
						find_location(token[0],&found);
					if(found==1){//if the first operand is a symbol
						obj_line[0]='M';
						obj_line[1]='\0';
						strcat(obj_line,hex_to_char(data[j].loc+1,6));//add starting location of the address field to be modified(relative to the beginning of the program)
						obj_line[7]='\0';
						strcat(obj_line,"05");//add the length of the address field to be modified in half bytes
						obj_line[9]='\0';
						fprintf(fp_obj,"%s\n",obj_line);//print modification record
					}
				}
			}
			obj_line[0]='E';
			obj_line[1]='\0';
			strcat(obj_line,hex_to_char(start_address,6));//add address of first executable instruction in object program
			obj_line[7]='\0';
			fprintf(fp_obj,"%s\n",obj_line);//print the end record
			obj_line[0]='\0';
		}
		//write to .lst file
		fprintf(fp_lst,"%-8d ",data[i].line_num);//print line number
		if(!strcmp(data[i].symbol,"."))//case for comment line
			fprintf(fp_lst,"        %-s\n",input[i]);
		else{
			if(data[i].format==FORMAT_COMMENT && strcmp(data[i].mnem,"START"))//case for assembly directive lines and first line
				fprintf(fp_lst,"        %-7s%-7s%-15s\n",data[i].symbol,data[i].mnem,data[i].operands);
			else{
			fprintf(fp_lst,"%04X    %-7s%-7s%-15s",data[i].loc,data[i].symbol,data[i].mnem,data[i].operands);
				if(data[i].format==2)
					fprintf(fp_lst,"%04X\n",data[i].obj_code);
				else if(data[i].format==3)
					fprintf(fp_lst,"%06X\n",data[i].obj_code);
				else if(data[i].format==4)
					fprintf(fp_lst,"%08X\n",data[i].obj_code);
				else if(data[i].format==FORMAT_CONSTANT){
					int num=data[i].obj_code;
					for(int j=0;j<data[i+1].loc-data[i].loc;j++){
						fprintf(fp_lst,"%02X",(int)(num/pow(16,(data[i+1].loc-data[i].loc-1-j)*2)));
						num%=(int)pow(16,(data[i+1].loc-data[i].loc-1-j)*2);
					}
					fprintf(fp_lst,"\n");
				}
				else//case for FORMAT_VARIABLE
					fprintf(fp_lst,"\n");
			}
		}
		//check if asm error occured
		if(asm_error){
			fclose(fp_lst);
			fclose(fp_obj);
			valid_command=0;
			printf("Error in line %d.\n",data[i].line_num);
			return;
		}
	}

	//=======end of PASS 2==========

	fclose(fp_lst);
	fclose(fp_obj);

	for(int i=0;i<3;i++)
		free(token[i]);
	free(token);
	
	for(int i=0;i<num_of_lines;i++)
		free(input[i]);
	free(input);

	free(name_with_no_extension);
	free(lst_name);
	free(obj_name);

	printf("Successfully assemble %s.\n",filename);

	return;
}

void insert_to_symbol_table(line data){
	//check if already in the table
	for(int i=0;i<num_of_symbols;i++){
		if(!strcmp(data.symbol,symbol_table[i].symbol)){
			asm_error=1;
			return;
		}
	}
	//add to symbol table
	symbol_table=(symbol_node*)realloc(symbol_table,(num_of_symbols+1)*sizeof(symbol_node));//allocate memory
	strcpy(symbol_table[num_of_symbols].symbol,data.symbol);
	symbol_table[num_of_symbols].loc=data.loc;
	num_of_symbols++;

	return;
}
	
int find_format(char*mnem){
	if(!strcmp(mnem,"START") || !strcmp(mnem,"END"))
		return FORMAT_COMMENT;
	if(!strcmp(mnem,"BYTE") || !strcmp(mnem,"WORD"))
		return FORMAT_CONSTANT;
	if(!strcmp(mnem,"RESB") || !strcmp(mnem,"RESW"))
		return FORMAT_VARIABLE;

	int value;//hash value
	hash_node*i;
	if(mnem[0]=='+'){
		//then check if it is format 4
		value=hash_func(mnem+1);
		for(i=hash_table[value]->next;i!=NULL;i=i->next)
			if(!strcmp(i->mnemonic,mnem+1) && i->format==3)
				return 4;
	}
	else{//check if it is format 1,2,3
		value=hash_func(mnem);
		for(i=hash_table[value]->next;i!=NULL;i=i->next)
			if(!strcmp(i->mnemonic,mnem))
				return i->format;
	}
	//it is none of the cases->assembly directive or comment
	return FORMAT_COMMENT;
}

int calculate_loc(line data){
	int ret=0;//return value(the amount location counter is increased)
	switch(data.format){
		case 1: ret=1;//format 1
				break;
		case 2: ret=2;//format 2
				break;
		case 3: ret=3;//format 3
				break;
		case 4: ret=4;//format 4
				break;
		case FORMAT_CONSTANT: //BYTE or WORD
				//check if operand is char, hexadecimal, or decimal(checking if operand is valid)
				if(!strcmp(data.mnem,"BYTE")){//BYTE constant
					switch(data.operands[0]){
						case 'C'://character
							ret=((int)strlen(data.operands)-3);
							break;
						case 'X'://hexadecimal
							ret=((int)strlen(data.operands)-3+1)/2;
							break;
						default://invalid operands
							asm_error=1;
							break;
					}
				}
				else if(!strcmp(data.mnem,"WORD")){//WORD constant
					switch(data.operands[0]){
						case 'C'://character
							ret=((int)strlen(data.operands)-3+2)/3*3;
							break;
						case 'X'://hexadecimal
							ret=((int)strlen(data.operands)-3+5)/6*6;
							break;
						default://invalid operands
							asm_error=1;
							break;
					}
				}
				else//invalid instruction
					asm_error=1;
				break;
		case FORMAT_VARIABLE: //RESB or RESW
				//check if operand is number
				for(int j=0;j<(int)strlen(data.operands);j++){
					if(data.operands[j]<'0' || data.operands[j]>'9'){//not a number
						asm_error=1;
						break;
					}
				}
				if(!strcmp(data.mnem,"RESB"))//BYTE variable
					ret=atoi(data.operands)*1;
				else if(!strcmp(data.mnem,"RESW"))//BYTE variable
					ret=atoi(data.operands)*3;
				else//invalid instruction
					asm_error=1;
				break;
		case FORMAT_COMMENT: //comment or assembly directives have no location counter increasment
				ret=0;
				break;
		default: //invalid format
				asm_error=1;
				break;
	}
	
	return ret;
}

int find_opcode(char*mnem,int*found){
	*found=1;
	
	int value;
	if(mnem[0]=='+')//format 4
		value=hash_func(mnem+1);
	else//format 1,2,3
		value=hash_func(mnem);
	
	//search symbol hash table
	hash_node*i;
	for(i=hash_table[value]->next;i!=NULL;i=i->next){
		if(mnem[0]=='+'){//format 4
			if(!strcmp(i->mnemonic,mnem+1))
				break;
		}
		else{//format 3
			if(!strcmp(i->mnemonic,mnem))
				break;
		}
	}

	if(i==NULL){//not found
		*found=0;
		return 0;
	}
	//found
	return char_to_hex(i->opcode);
}

int register_number(char*mnem){
	if(!strcmp(mnem,"A"))
		return 0;
	if(!strcmp(mnem,"X"))
		return 1;
	if(!strcmp(mnem,"L"))
		return 2;
	if(!strcmp(mnem,"PC"))
		return 8;
	if(!strcmp(mnem,"SW"))
		return 9;
	if(!strcmp(mnem,"B"))
		return 3;
	if(!strcmp(mnem,"S"))
		return 4;
	if(!strcmp(mnem,"T"))
		return 5;
	if(!strcmp(mnem,"F"))
		return 6;

	asm_error=1;//not found
	return -1;
}

int generate_obj_code(int opcode,line data,int addr,int*pc,int*base,int symbol_flag){
	int obj_code=0;
	int r1,r2;//register(format 2)
	int n,i,x,b,p,e;//(format 3,4)
	int disp;//(format 3)

	//split operands
	int num_of_token = 0;
	char**token=(char**)malloc(sizeof(char*)*3);
	for(int i=0;i<3;i++)
		token[i]=(char*)malloc(sizeof(char)*30);
	num_of_token=split_command(data.operands,&token);
	//set n,i to 1,1(simple addressing. general case)
	n=1;
	i=1;
	if(num_of_token>0){//if operand exist
		switch(token[0][0]){
			case '#'://immediate addressing
				n=0;
				i=1;
				break;
			case '@'://indirect addressing
				n=1;
				i=0;
				break;
			default://simple addressing
				n=1;
				i=1;
				break;
		}
	}

	switch(data.format){
		case 1: //format 1
			obj_code=opcode;
			if(num_of_token!=0)//if format 1 has operands
				asm_error=1;
			break;
		case 2: //format 2
			obj_code=opcode<<8;
			switch(num_of_token){//number of operands
				case 0:
					r1=0;
					r2=0;
					break;
				case 1:
					r1=register_number(token[0]);
					r2=0;
					break;
				case 2:
					r1=register_number(token[0]);
					r2=register_number(token[1]);
					break;
				default:
					r1=0;
					r2=0;
					asm_error=1;
					break;
			}
			r1=r1<<4;
			obj_code+=(r1+r2);
			break;
		case 3: //format 3
			if(!strcmp(data.mnem,"LDB"))//check if have to set base register
				*base=addr;
			e=0;//no extension for format 3
			obj_code=(opcode>>2)<<18;//clear 2 less significant bits and shift to most significant bits
			switch(num_of_token){//the number of operands
				case 2:
					if(!strcmp(token[1],"X")){//check if second operand is X
						x=1;
					}
					else{//if second operand is not x, error
						x=0;
						asm_error=1;
					}
					break;
				case 1://no index
					x=0;
					break;
				case 0://no index
					x=0;
					break;
				default:
					x=0;
					asm_error=1;
					break;
			}
			if(symbol_flag==1){
				//decide disp,b,p
				if((addr-*pc) >= -2048 && (addr-*pc) <= 2047){//PC relative
					b=0;
					p=1;
					disp=addr-*pc;
				}
				else if((addr-*base)>=0 && (addr-*base)<=4095){//Base relative
					b=1;
					p=0;
					disp=addr-*base;
				}
				else{//had to be format 4
					asm_error=1;
				}
			}
			else{//not a symbol, number
				b=0;
				p=0;
				disp=addr;
			}
			obj_code+=n<<17;
			obj_code+=i<<16;
			obj_code+=x<<15;
			obj_code+=b<<14;
			obj_code+=p<<13;
			obj_code+=e<<12;
			if(disp<0){//have to make 32bit 2's complement number to 12bit 2's complement
				disp = disp & 4095;//disp AND 00...000111111111111
			}
			obj_code+=disp;
			break;
		case 4: //format 4
			if(data.mnem[0]=='+' && !strcmp(data.mnem+1,"LDB"))//check if have to set base register
				*base=addr;
			b=0;
			p=0;
			e=1;//extension
			obj_code=(opcode>>2)<<26;//clear 2 least significant bits and shift to most significant bits
			switch(num_of_token){
			case 2:
				if(!strcmp(token[1],"X")){//check if second operand is X
					x=1;
				}
				else{
					x=0;
					asm_error=1;
				}
				break;
			case 1://no index
				x=0;
				break;
			case 0://no index
				x=0;
				break;
			default:
				x=0;
				asm_error=1;
				break;
			}
			obj_code+=n<<25;
			obj_code+=i<<24;
			obj_code+=x<<23;
			obj_code+=b<<22;
			obj_code+=p<<21;
			obj_code+=e<<20;
			obj_code+=addr;
			break;
		default: //constant, comment
			break;
	}
	
	return obj_code;
}

int find_location(char*symbol,int*found){
	int i,j;
	*found=1;//1: found in symbol table
	//search symbol table
	for(i=0;i<num_of_symbols;i++){
		if(!strcmp(symbol_table[i].symbol,symbol)){
			break;
		}
	}
	//check if symbol is number
	if(i>=num_of_symbols){
		for(j=0;j<(int)strlen(symbol);j++){
			if(symbol[j]<'0' || symbol[j]>'9')
				break;
		}
		if(j<(int)strlen(symbol)){//not a number.
			*found=0;//0: invalid operand or register
			return 0;
		}
		else{
			*found=2;//2: number
			return atoi(symbol);
		}
	}
	
	return symbol_table[i].loc;
}

void print_symbol_table(void){
	if(num_of_symbols==0){
		printf("Symbol table is empty.\n");
		printf("Recent assemble was failed or no symbols generated.\n");
	}

	for(int i=0;i<num_of_symbols;i++)
		printf("\t%-8s%04X\n",(symbol_table+i)->symbol,(symbol_table+i)->loc);

	return;
}

char*hex_to_char(int hex,int length){
	char*str=(char*)malloc(sizeof(char)*(length+1));
	
	for(int i=0;i<length;i++){
		str[length-i-1]=hex%16;
		if(str[length-i-1]>9)
			str[length-i-1]+=('A'-10);
		else
			str[length-i-1]+='0';
		hex/=16;
	}
	
	return str;
}
