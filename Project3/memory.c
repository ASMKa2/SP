#include "20181592.h"

void dump_1(void){//dump command with no parameters
	if(dump_address+16*10-1<=0XFFFFF){//if dump_address is small enough to print 10 lines
		print_dump(dump_address,dump_address+16*10-1);//print 10lines
		dump_address=dump_address+16*10;//next dump_address
	}
	else{
		print_dump(dump_address,0XFFFFF);//print till the end
		dump_address=0X100000;//next dump_address
	}

	if(dump_address>0XFFFFF)//if dump_address is bigger than 0XFFFFF
		dump_address=0;
	
	return;
}

void dump_2(char*start){//dump command with one parameter
	int start_hex=char_to_hex(start);//convert the input(char) to hex(int)
	if(valid_command==0){
		printf("Error: Invalid Parameter.\n");
		return;
	}
	//check range
	if(start_hex>0XFFFFF || start_hex<0){//if start address is out of range
		printf("Error: Address is not in the range.\n");
		valid_command=0;
	}
	else if(start_hex+16*10-1>0XFFFFF)//if start address is too big to print 10 lines
		print_dump(start_hex,0XFFFFF);//print 10lines
	else//can print 10 lines
		print_dump(start_hex,start_hex+16*10-1);


	return;
}

void dump_3(char*start, char*end) {
	//convert the input(char) to hex(int)
	int start_hex=char_to_hex(start);//convert the start address to int
	int end_hex=char_to_hex(end);//convert the start address to int	
	if(valid_command==0){
		printf("Error: Invalid Parameter.\n");
		return;
	}
	//check range
	if(start_hex>end_hex){
		printf("Error: Start address must not be greater than end address.\n");
		valid_command=0;
		return;
	}
	if(start_hex>0XFFFFF || start_hex<0){
		printf("Error: Start address is not in the range.\n");
		valid_command=0;
		return;
	}
	if(end_hex>0XFFFFF || end_hex<0){
		printf("Error: End address is not in the range.\n");
		valid_command=0;
		return;
	}
	//print
	print_dump(start_hex,end_hex);
	
	return;
}

int char_to_hex(char*str){
	int length=(int)strlen(str);
	int hex=0X0;

	for(int i=0;i<length;i++){
		if(str[i]>=65 && str[i]<=70)//'A'~'F'
			hex+=(str[i]-65+10)*pow(16,length-1-i);
		else if(str[i]>=97 && str[i]<=102)//'a'~'f'
			hex+=(str[i]-97+10)*pow(16,length-1-i);
		else if(str[i]>=48 && str[i]<=57)//'0'~'9'
			hex+=(str[i]-48)*pow(16,length-1-i);
		else{
			valid_command=0;
			break;
		}
	}

	return hex;
}

void print_dump(int start,int end){
	int i,j;
	for(i=start/16;i<=end/16;i++){
		printf("%05X ",i*16);
	
		if(start/16==end/16){
			for(j=0;j<start%16;j++)
				printf("   ");
			for(j=start%16;j<=end%16;j++)
				printf("%02X ",memory[i][j]);
			for(j=end%16+1;j<16;j++)
				printf("   ");
			printf("; ");
			for(j=0;j<start%16;j++)
				printf(".");
			for(j=start%16;j<=end%16;j++){
				if(memory[i][j]>=0x20 && memory[i][j]<=0x7E)
					printf("%c",memory[i][j]);
				else
					printf(".");
			}
			for(j=end%16+1;j<16;j++)
				printf(".");
			printf("\n");
			continue;//break;
		}

		if(i==start/16){
			for(j=0;j<start%16;j++)
				printf("   ");
			for(j=start%16;j<16;j++)
				printf("%02X ",memory[i][j]);
		}
		else if(i==end/16){
			for(j=0;j<=end%16;j++)
				printf("%02X ",memory[i][j]);
			for(j=end%16+1;j<16;j++)
				printf("   ");
		}
		else
			for(j=0;j<16;j++)
				printf("%02X ",memory[i][j]);
		printf("; ");
		
		if(i==start/16){
			for(j=0;j<start%16;j++)
				printf(".");
			for(j=start%16;j<16;j++){
				if(memory[i][j]>=0x20 && memory[i][j]<=0x7E)
					printf("%c",memory[i][j]);
				else
					printf(".");
			}
		}
		else if(i==end/16){
			for(j=0;j<=end%16;j++){
				if(memory[i][j]>=0x20 && memory[i][j]<=0x7E)
					printf("%c",memory[i][j]);
				else
					printf(".");
			}
			for(j=end%16+1;j<16;j++)
				printf(".");
		}
		else{
			for(j=0;j<16;j++){
				if(memory[i][j]>=0x20 && memory[i][j]<=0x7E)
					printf("%c",memory[i][j]);
				else
					printf(".");
			}	
		}
		printf("\n");
	}
}
	

void edit(char*address, char*value) {
	int hex_address=char_to_hex(address);
	int hex_value=char_to_hex(value);
	if(valid_command==0){
		printf("Error: Invalid Parameter.\n");
		return;
	}
	//check range
	if(hex_address<0 || hex_address>0XFFFFF){
		printf("Error: Address is not in the range.\n");
		valid_command=0;
		return;
	}
	if(hex_value<0 || hex_value>0XFF){
		printf("Error: Value is not in the range.\n");
		valid_command=0;
		return;
	}
	memory[hex_address/16][hex_address%16]=hex_value;
}

void fill(char*start, char*end, char*value) {
	int hex_start=char_to_hex(start);
	int hex_end=char_to_hex(end);
	int hex_value=char_to_hex(value);
	if(valid_command==0){
		printf("Error: Invalid Parameter.\n");
		return;
	}

	//check range
	if(hex_start>hex_end){
		printf("Error: Start address must not be greater than end address.\n");
		valid_command=0;
		return;
	}
	if(hex_start<0 || hex_start>0XFFFFF){
		printf("Error: Start address is not in the range.\n");
		valid_command=0;
		return;
	}
	if(hex_end<0 || hex_end>0XFFFFF){
		printf("Error: End address is not in the range.\n");
		valid_command=0;
		return;
	}
	if(hex_value<0 || hex_value>0XFF){
		printf("Error: Value is not in the range.\n");
		valid_command=0;
		return;
	}

	for(int i=hex_start;i<=hex_end;i++)
		memory[i/16][i%16]=hex_value;

	return;
}

void reset(void) {
	for(int i=0;i<65536;i++)
		for(int j=0;j<16;j++)
			memory[i][j]=0;

	return;
}
