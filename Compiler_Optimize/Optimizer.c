/*
 *********************************************
 *  314 Principles of Programming Languages  *
 *  Spring 2017                              *
 *  Author: Ulrich Kremer                    *
 *  Student Version                          *
 *********************************************
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "InstrUtils.h"
#include "Utils.h"
#include <string.h>

int main()
{
	//printf("got into main\n");
	Instruction *head;
	head = ReadInstructionList(stdin);
	if (!head) {
		WARNING("No instructions\n");
		exit(EXIT_FAILURE);
	}
	
	/* YOUR CODE GOES HERE */
	Instruction* inst = LastInstruction(head);
	Instruction* tmp;
	int critical = 0;
	for(;inst!=NULL;inst=inst->prev){
		if(inst->prev ==NULL){
			inst->critical = '1';
		}
		else if(inst->opcode==OUTPUTAI){
			if(inst->next!= NULL){
				for(tmp=inst;tmp!=NULL;tmp=tmp->next){
					if(tmp->opcode==OUTPUTAI){
						tmp->critical = '1';
					}
					else if(tmp->critical =='1'){
						continue;
					}
					else{
						tmp->critical = '0';
					}
				}
			}
			inst->critical = '1';
		}
		else if(inst->opcode==STOREAI){
			for(tmp = inst->next;tmp!=NULL;tmp =tmp->next){
				if(tmp->field2 == inst->field2&&tmp->field3 == inst->field3 &&tmp->opcode==STOREAI){
					break;
				}
				else if(tmp->field1 == inst->field2&&tmp->field2 == inst->field3&&tmp->critical=='1'){
					if(tmp->opcode == STOREAI){
						break;
					}
					critical = 1;
					break;
				}
			}
			if(critical ==1){
				inst->critical = '1';
			}
			else if(critical ==0){
				inst->critical = '0';
			}
		}
		else if(inst->opcode==LOADI){
			for(tmp = inst->next;tmp!=NULL;tmp =tmp->next){
				if((tmp->field1==inst->field2&&tmp->critical=='1')||(tmp->field2==inst->field2&&tmp->critical=='1')){
					if(tmp->opcode ==LOADI ||tmp->opcode ==OUTPUTAI|| tmp->opcode==LOADAI){
						continue;
					}
					critical = 1;
					break; 
				}
			}
			if(critical ==1){
				inst->critical = '1';
			}
			else if(critical ==0){
				inst->critical = '0';
			}
		}
		else if(inst->opcode==LOADAI){
			for(tmp = inst->next;tmp!=NULL;tmp =tmp->next){
				if((tmp->field1==inst->field3&&tmp->critical=='1')||(tmp->field2==inst->field3&&tmp->critical=='1')){
					if(tmp->opcode ==LOADI||tmp->opcode ==OUTPUTAI){
						continue;
					}
					critical = 1;
					break;
				} 
			}
			if(critical ==1){
				inst->critical = '1';
			}
			else if(critical ==0){
				inst->critical = '0';
			}
		}
		else if(inst->opcode==SUB||inst->opcode==ADD||inst->opcode==MUL||inst->opcode==DIV){
			for(tmp=inst->next;tmp!=NULL;tmp = tmp->next){
				if((tmp->field1==inst->field3 &&tmp->critical=='1')||(tmp->field2==inst->field3 &&tmp->critical=='1')){
					if(tmp->opcode ==LOADI||tmp->opcode ==OUTPUTAI){
						continue;
					}
					critical = 1;
					break;
				}
			}
			if(critical ==1){
				inst->critical = '1';
			}
			else if(critical ==0){
				inst->critical = '0';
			}
		}
		critical = 0;
	}
	int em = 0;
	for(inst = head;inst!=NULL;inst = inst->next){
		if(inst->next ==NULL){
			break;
		}
		else if(inst->next->critical =='0'){
			tmp = inst->next;
			while(tmp->critical!='1'){
				if(tmp->next == NULL){
					free(tmp);
					em = 1;
					break;
				}
				tmp= tmp->next;
				free(tmp->prev);
			}
			if(em ==0){
				inst->next = tmp;
			}
			else{
				inst->next = NULL;
				break;
			}
		}
		else{
			continue;
		}
	}
	if (head){ 
		PrintInstructionList(stdout, head);
	}
	return EXIT_SUCCESS;
}
