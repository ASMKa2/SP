#include "20181592.h"

void run(void){
	if(num_of_breaks==0)
		end_of_program=reg[2];//reg[2]:L

	int opcode;
	int byte;
	int CC;//condition code for comparing
	int num_of_exec;//for checking if PC is on the bp because it is the first execution after bp stop
	for(num_of_exec=0;;num_of_exec++){
		if(reg[8]==end_of_program){//reg[8]:PC
			num_of_breaks=0;
			//print registers
			printf("A : %06X  X : %06X\n",reg[0],reg[1]);
			printf("L : %06X PC : %06X\n",reg[2],reg[8]);
			printf("B : %06X  S : %06X\n",reg[3],reg[4]);
			printf("T : %06X\n",reg[5]);
			printf("End program\n");
			break;
		}
		if(check_break_point() && num_of_exec!=0){//meet a breakpoint
			num_of_breaks++;
			printf("A : %06X  X : %06X\n",reg[0],reg[1]);
			printf("L : %06X PC : %06X\n",reg[2],reg[8]);
			printf("B : %06X  S : %06X\n",reg[3],reg[4]);
			printf("T : %06X\n",reg[5]);
			printf("Stop at breakpoint[%X]\n",reg[8]);//reg[8]:PC
			break;
		}
		//read next BYTE
		byte=memory[reg[8]/16][reg[8]%16];//reg[8]:PC
		opcode=byte & 0b11111100;
		//select by opcode
		if(opcode/0X10==0XC || opcode/0x10==0XF){//format 1
			reg[8]+=1;
			switch(opcode){
				case 0XC4://FIX
					break;
				case 0XC0://FLOAT
					break;
				case 0XF4://HIO
					break;
				case 0XC8://NORM
					break;
				case 0XF0://SIO
					break;
				case 0XF8://TIO
					break;
				default:
					valid_command=0;
					printf("Error: Invalid opcode.\n");
					break;
			}
		}
		else if(opcode/0X10==0X9 || opcode/0X10==0XA || opcode/0X10==0XB || opcode==0X60){//format 2
			byte=memory[(reg[8]+1)/16][(reg[8]+1)%16];//reg[8]:PC
			int r1,r2;
			r1=byte/16;
			r2=byte%16;
			if(r1==7 || r2==7){
				valid_command=0;
				printf("Error while running program.\n");
				break;
			}
			reg[8]+=2;//PC+=2
			switch(opcode){
				case 0X90://ADDR
					reg[r2]=reg[r2]+reg[r1];
					break;
				case 0XB4://CLEAR
					reg[r1]=0;
					break;
				case 0XA0://COMPR
					if(reg[r1]<reg[r2])
						CC='<';
					else if(reg[r1]==reg[r2])
						CC='=';
					else
						CC='>';
					break;
				case 0X9C://DIVR
					reg[r2]=reg[r2]/reg[r1];
					break;
				case 0X60://MULR
					reg[r2]=reg[r2]*reg[r1];
					break;
				case 0XAC://RMO
					reg[r2]=reg[r1];
					break;
				case 0XA4://SHIFTL
					reg[r1]=((reg[r1]<<r2)&(int)pow(2,24)) | (reg[r1]>>(24-r2));
					break;
				case 0XA8://SHIFTR
					if(reg[r1]/(int)pow(2,23) == 0)
						reg[r1]=(reg[r1]>>r2);
					else if(reg[r1]/(int)pow(2,23) == 1)
						reg[r1]=(reg[r1]>>r2) & (((int)pow(2,24)-1) - ((int)pow(2,24-r2)-1));
					break;
				case 0X94://SUBR
					reg[r2]=reg[r2]-reg[r1];
					break;
				case 0XB0://SVC
					break;
				case 0XB8://TIXR
					reg[1]+=1;
					if(reg[1]<reg[r1])
						CC='<';
					else if(reg[1]==reg[r1])
						CC='=';
					else
						CC='>';
					break;
				default:
					valid_command=0;
					printf("Error while running program.\n");
					break;
			}
		}
		else{//format 3/4
			int n,i,x,b,p,e;
			n=byte/2%2;
			i=byte%2;
			byte=memory[(reg[8]+1)/16][(reg[8]+1)%16];//reg[8]:PC
			x=byte/(int)pow(2,7)%2;
			b=byte/(int)pow(2,6)%2;
			p=byte/(int)pow(2,5)%2;
			e=byte/(int)pow(2,4)%2;
			int TA=0;//target address. 
			int disp=0;//for format 3, disp field. for format 4, address field.
			int m, addr;
			if(n==0 && i==0){
				byte=memory[(reg[8]+1)/16][(reg[8]+1)%16];//reg[8]:PC
				disp+=(byte%32)*(int)pow(2,16);//5 LSB
				byte=memory[(reg[8]+2)/16][(reg[8]+2)%16];//reg[8]:PC
				disp+=byte*(int)pow(2,8);
				byte=memory[(reg[8]+3)/16][(reg[8]+3)%16];//reg[8]:PC
				disp+=byte;
			}
			else{
				if(e==1 && b==0 && p==0){//format 4
					byte=memory[(reg[8]+1)/16][(reg[8]+1)%16];//reg[8]:PC
					disp+=(byte%4)*(int)pow(2,16);//2 LSB
					byte=memory[(reg[8]+2)/16][(reg[8]+2)%16];//reg[8]:PC
					disp+=byte*(int)pow(2,8);
					byte=memory[(reg[8]+3)/16][(reg[8]+3)%16];//reg[8]:PC
					disp+=byte;
					TA=disp;
					reg[8]+=4;//PC+=4
				}
				else if(e==0){//format 3
					byte=memory[(reg[8]+1)/16][(reg[8]+1)%16];//reg[8]:PC
					disp+=(byte%16)*(int)pow(2,8);//2 LSB
					byte=memory[(reg[8]+2)/16][(reg[8]+2)%16];//reg[8]:PC
					disp+=byte;
					reg[8]+=3;//PC+=3
					if(b==1 && p==0){//base relative
						TA=reg[3]+disp;//reg[3]:B
					}
					else if(b==0 && p==1){//PC relative
						if(disp/(int)pow(2,11)%2 == 1){//if disp is '-'
							//convert 12bit 2'complement negative to positive
							//ex.FFE(-2)->002(+2)
							disp=~disp;
							disp++;
							disp=disp&((int)pow(16,3)-1);
							//calculate TA
							TA=reg[8]-disp;//reg[8]:PC
						}
						else
							TA=reg[8]+disp;//reg[8]:PC
					}
					else if(b==1 && p==1){
						printf("Error2\n");
					}
					else if(b==0 && p==0){
						TA=disp;
					}
				}
				else{
					printf("Error1\n");
				}
				//check x(index)
				if(x==1)
					TA+=reg[1];//reg[1]:X
				//check n,i (addressing mode)
				if(n==0 && i==1){//immediate addressing
					m=TA;
				}
				else if(n==1 && i==0){//indirect addressing
					byte=memory[TA/16][TA%16]*(int)pow(16,4)+memory[(TA+1)/16][(TA+1)%16]*(int)pow(16,2)+memory[(TA+2)/16][(TA+2)%16];
					m=memory[byte/16][byte%16]*(int)pow(16,4)+memory[(byte+1)/16][(byte+1)%16]*(int)pow(16,2)+memory[(byte+2)/16][(byte+2)%16];
					addr=byte;
				}
				else if(n==1 && i==1){//simple addressing
					m=memory[TA/16][TA%16]*(int)pow(16,4)+memory[(TA+1)/16][(TA+1)%16]*(int)pow(16,2)+memory[(TA+2)/16][(TA+2)%16];
					addr=TA;
				}
				else
					printf("Error3\n");
			}
			switch(opcode){
				case 0X18://ADD
					reg[0]+=m;//reg[0]:A
					break;
				case 0X58://ADDF
					break;
				case 0X40://AND
					reg[0]=reg[0]&m;//reg[0]:A
					break;
				case 0X28://COMP
					if(reg[0]<m)//reg[0]:A
						CC='<';
					else if(reg[0]==m)//reg[0]:A
						CC='=';
					else
						CC='>';
					break;
				case 0X88://COMPF
					break;
				case 0X24://DIV
					reg[0]/=m;//reg[0]:A
					break;
				case 0X64://DIVF
					break;
				case 0X3C://J
					reg[8]=addr;//reg[8]:PC
					break;
				case 0X30://JEQ
					if(CC=='=')
						reg[8]=addr;//reg[8]:PC
					break;
				case 0X34://JGT
					if(CC=='>')
						reg[8]=addr;//reg[8]:PC
					break;
				case 0X38://JLT
					if(CC=='<')
						reg[8]=addr;//reg[8]:PC
					break;
				case 0X48://JSUB
					reg[2]=reg[8];//reg[2]:L, reg[8]:PC
					reg[8]=addr;//reg[8]:PC
					break;
				case 0X00://LDA
					reg[0]=m;//reg[0]:A
					break;
				case 0X68://LDB
					reg[3]=m;//reg[3]:B
					break;
				case 0X50://LDCH
					reg[0]-=reg[0]%(int)pow(16,2);//clear rightmost byte
					int temp=m/(int)pow(16,4);
					reg[0]+=temp;
					break;
				case 0X70://LDF
					break;
				case 0X08://LDL
					reg[2]=m;//reg[2]:L
					break;
				case 0X6C://LDS
					reg[4]=m;//reg[4]:S
					break;
				case 0X74://LDT
					reg[5]=m;//reg[5]:T
					break;
				case 0X04://LDX
					reg[1]=m;//reg[1]:X
					break;
				case 0XD0://LPS
					break;
				case 0X20://MUL
					reg[0]*=m;//reg[0]:A
					break;
				case 0X60://MULF
					break;
				case 0X44://OR
					reg[0]=reg[0]|m;//reg[0]:A
					break;
				case 0XD8://RD
					CC='=';
					break;
				case 0X4C://RSUB
					reg[8]=reg[2];//reg[8]:PC, reg[2]:L
					break;
				case 0XEC://SSK
					break;
				case 0X0C://STA
					for(int i=0;i<3;i++)
						memory[(addr+i)/16][(addr+i)%16]=reg[0]/(int)pow(16,4-2*i)%(int)pow(16,2);//reg[0]:A
					break;
				case 0X78://STB
					for(int i=0;i<3;i++)
						memory[(addr+i)/16][(addr+i)%16]=reg[3]/(int)pow(16,4-2*i)%(int)pow(16,2);//reg[3]:B
					break;
				case 0X54://STCH
					memory[addr/16][addr%16]=reg[0]%(int)pow(16,2);//reg[0]:A
					break;
				case 0X80://STF
					break;
				case 0XD4://STI
					break;
				case 0X14://STL
					for(int i=0;i<3;i++)
						memory[(addr+i)/16][(addr+i)%16]=reg[2]/(int)pow(16,4-2*i)%(int)pow(16,2);//reg[2]:L
					break;
				case 0X7C://STS
					for(int i=0;i<3;i++)
						memory[(addr+i)/16][(addr+i)%16]=reg[4]/(int)pow(16,4-2*i)%(int)pow(16,2);//reg[4]:S
					break;
				case 0XE8://STSW
					for(int i=0;i<3;i++)
						memory[(addr+i)/16][(addr+i)%16]=reg[9]/(int)pow(16,4-2*i)%(int)pow(16,2);//reg[9]:SW
					break;
				case 0X84://STT
					for(int i=0;i<3;i++)
						memory[(addr+i)/16][(addr+i)%16]=reg[5]/(int)pow(16,4-2*i)%(int)pow(16,2);//reg[5]:T
					break;
				case 0X10://STX
					for(int i=0;i<3;i++)
						memory[(addr+i)/16][(addr+i)%16]=reg[1]/(int)pow(16,4-2*i)%(int)pow(16,2);//reg[1]:X
					break;
				case 0X1C://SUB
					reg[0]-=m;//reg[0]:A
					break;
				case 0X5C://SUBF
					break;
				case 0XE0://TD
					CC='<';
					break;
				case 0X2C://TIX
					reg[1]+=1;
					if(reg[1]<m)
						CC='<';
					else if(reg[1]==m)
						CC='=';
					else
						CC='>';
					break;
				case 0XDC://WD
					break;
				default:
					valid_command=0;
					printf("Error: Invalid opcode.\n");
					break;
			}
		}
		if(valid_command==0)
			break;
	}

	if(valid_command==0)
		return;

	return;
}
