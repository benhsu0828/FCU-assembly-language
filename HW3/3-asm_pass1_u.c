/***********************************************************************/
/*  Program Name: 3-asm_pass1_u.c                                      */
/*  This program is the part of SIC/XE assembler Pass 1.	  		   */
/*  The program only identify the symbol, opcode and operand 		   */
/*  of a line of the asm file. The program do not build the            */
/*  SYMTAB.			                                               	   */
/*  2019.12.13                                                         */
/*  2021.03.26 Process error: format 1 & 2 instruction use + 		   */
/***********************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "2-optable.c"

/* Public variables and functions */
#define	ADDR_SIMPLE			0x01
#define	ADDR_IMMEDIATE		0x02
#define	ADDR_INDIRECT		0x04
#define	ADDR_INDEX			0x08

#define	LINE_EOF			(-1)
#define	LINE_COMMENT		(-2)
#define	LINE_ERROR			(0)
#define	LINE_CORRECT		(1)

typedef struct
{
	char		symbol[LEN_SYMBOL];
	char		op[LEN_SYMBOL];
	char		operand1[LEN_SYMBOL];
	char		operand2[LEN_SYMBOL];
	unsigned	code;
	unsigned	fmt;
	unsigned	addressing;	
} LINE;

int process_line(LINE *line);
/* return LINE_EOF, LINE_COMMENT, LINE_ERROR, LINE_CORRECT and Instruction information in *line*/

/* Private variable and function */

void init_LINE(LINE *line)
{
	line->symbol[0] = '\0';
	line->op[0] = '\0';
	line->operand1[0] = '\0';
	line->operand2[0] = '\0';
	line->code = 0x0;
	line->fmt = 0x0;
	line->addressing = ADDR_SIMPLE;
}

int process_line(LINE *line)
/* return LINE_EOF, LINE_COMMENT, LINE_ERROR, LINE_CORRECT */
{
	char		buf[LEN_SYMBOL];
	int			c;
	int			state;
	int			ret;
	char		tmpSpecial[2];
	strcpy(tmpSpecial,"\0");
	Instruction	*op;
	
	c = ASM_token(buf);		/* get the first token of a line */
	if(c == EOF)
		return LINE_EOF;
	else if((c == 1) && (buf[0] == '\n'))	/* blank line */
		return LINE_COMMENT;
	else if((c == 1) && (buf[0] == '.'))	/* a comment line */
	{
		do
		{
			c = ASM_token(buf);
		} while((c != EOF) && (buf[0] != '\n'));
		return LINE_COMMENT;
	}
	else
	{
		init_LINE(line);
		ret = LINE_ERROR;
		state = 0;
		while(state < 8)
		{
			switch(state)
			{
				case 0:
				case 1:
				case 2:
					op = is_opcode(buf);
					if((state < 2) && (buf[0] == '+'))	/* + */
					{
						line->fmt = FMT4;
						strcpy(tmpSpecial,buf);//
						state = 2;
					}
					else	if(op != NULL)	/* INSTRUCTION */
					{//
						strcpy(line->op, op->op);
						add_string(tmpSpecial,line->op);
						strcpy(tmpSpecial,"\0");
						line->code = op->code;
						state = 3;
						if(line->fmt != FMT4)
						{
							line->fmt = op->fmt & (FMT1 | FMT2 | FMT3);
						}
						else if((line->fmt == FMT4) && ((op->fmt & FMT4) == 0)) /* INSTRUCTION is FMT1 or FMT 2*/
						{	/* ERROR 20210326 added */
							printf("ERROR at token %s, %s cannot use format 4 \n", buf, buf);
							ret = LINE_ERROR;
							state = 7;		/* skip following tokens in the line */
						}
					}				
					else	if(state == 0)	/* SYMBOL */
					{
						strcpy(line->symbol, buf);
						state = 1;
					}
					else		/* ERROR */
					{
						printf("ERROR at token %s\n", buf);
						ret = LINE_ERROR;
						state = 7;		/* skip following tokens in the line */
					}
					break;	
				case 3:
					if(line->fmt == FMT1 || line->code == 0x4C)	/* no operand needed */
					{
						if(c == EOF || buf[0] == '\n')
						{
							ret = LINE_CORRECT;
							state = 8;
						}
						else		/* COMMENT */
						{
							ret = LINE_CORRECT;
							state = 7;
						}
					}
					else
					{
						if(c == EOF || buf[0] == '\n')
						{
							ret = LINE_ERROR;
							state = 8;
						}
						else	if(buf[0] == '@' || buf[0] == '#')
						{
							line->addressing = (buf[0] == '#') ? ADDR_IMMEDIATE : ADDR_INDIRECT;
							//printf("1. buf: %s ",buf);
							strcpy(tmpSpecial,buf);//
							state = 4;
						}
						else	/* get a symbol */
						{
							op = is_opcode(buf);
							if(op != NULL)
							{
								printf("Operand1 cannot be a reserved word\n");
								ret = LINE_ERROR;
								state = 7; 		/* skip following tokens in the line */
							}
							else
							{
								strcpy(line->operand1, buf);
								//printf("test %s \n",line->operand1 );
								state = 5;
							}
						}
					}			
					break;		
				case 4:
					op = is_opcode(buf);
					if(op != NULL)
					{
						printf("Operand1 cannot be a reserved word\n");
						ret = LINE_ERROR;
						state = 7;		/* skip following tokens in the line */
					}
					else//
					{
						//printf("2. buf: %s ",buf);
						//printf("3. tmpSpecial: %s ",tmpSpecial);
						strcpy(line->operand1,buf);
						add_string(tmpSpecial,line->operand1);
						strcpy(tmpSpecial,"\0");
						state = 5;
					}
					break;
				case 5:
					if(c == EOF || buf[0] == '\n')
					{
						ret = LINE_CORRECT;
						state = 8;
					}
					else if(buf[0] == ',')
					{
						state = 6;
					}
					else	/* COMMENT */
					{
						ret = LINE_CORRECT;
						state = 7;		/* skip following tokens in the line */
					}
					break;
				case 6:
					if(c == EOF || buf[0] == '\n')
					{
						ret = LINE_ERROR;
						state = 8;
					}
					else	/* get a symbol */
					{
						op = is_opcode(buf);
						if(op != NULL)
						{
							printf("Operand2 cannot be a reserved word\n");
							ret = LINE_ERROR;
							state = 7;		/* skip following tokens in the line */
						}
						else
						{
							if(line->fmt == FMT2)
							{
								strcpy(line->operand2, buf);
								ret = LINE_CORRECT;
								state = 7;
							}
							else if((c == 1) && (buf[0] == 'x' || buf[0] == 'X'))
							{
								line->addressing = line->addressing | ADDR_INDEX;
								ret = LINE_CORRECT;
								state = 7;		/* skip following tokens in the line */
							}
							else
							{
								printf("Operand2 exists only if format 2  is used\n");
								ret = LINE_ERROR;
								state = 7;		/* skip following tokens in the line */
							}
						}
					}
					break;
				case 7:	/* skip tokens until '\n' || EOF */
					if(c == EOF || buf[0] =='\n')
						state = 8;
					break;										
			}
			if(state < 8)
				c = ASM_token(buf);  /* get the next token */
		}
		return ret;
	}
}
// queue
#define MAX_SIZE 100
#define MAX_LENGTH 50

struct Queue {
    char items[MAX_SIZE][MAX_LENGTH];
	int data[MAX_SIZE];
    int front;
    int rear;
};

typedef struct Queue queue;

void init(queue *q) {
    q->front = 0;
    q->rear = 0;
}

int is_empty(queue *q) {
	if(q->front == q->rear){
		q->front = q->rear = 0;
		return 1;
	}
    else
    	return 0;
}

int is_full(queue *q) {
    return q->rear == MAX_SIZE - 1;
}

void push(queue *q, char *value ,int inputData) {
    if (is_full(q)) {
        printf("Queue is full\n");
        return;
    }
    strcpy(q->items[q->rear], value);
	q->data[q->rear] = inputData;
    q->rear++;
}

int pop(queue *q) {
    if (is_empty(q)) {
        printf("Queue is empty\n");
        return 0;
    }
        return q->front++;
}

char *front(queue *q) {
    if (is_empty(q)) {
        printf("Queue is empty\n");
        exit(1);
    }
    return q->items[q->front];
}

void print_queue(queue *q) {
    int i = q->front;
    if (is_empty(q)) {
        printf("Queue is empty\n");
        return;
    }
    for (i = q->front; i < q->rear; i++) {
        printf("%10s: %06x \n", q->items[i],q->data[i]);
    }
    printf("\n");
}
//queue

int string_equal(const char *str1, const char *str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if (len1 != len2) {
        return 0; // 長度不同，不相等
    }
    for (int i = 0; i < len1; i++) {
        if (str1[i] != str2[i]) {
            return 0; // 有一個字元不同，不相等
        }
    }
    return 1; // 兩個字串內容相等
}

//16進位轉10進位
#include <stdio.h>
#include <string.h>

int hexToDec(char hex[]) {
    int len = strlen(hex);
    int base = 1;
    int dec = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (hex[i] >= '0' && hex[i] <= '9') {
            dec += (hex[i] - '0') * base;
        } else if (hex[i] >= 'A' && hex[i] <= 'F') {
            dec += (hex[i] - 'A' + 10) * base;
        } else if (hex[i] >= 'a' && hex[i] <= 'f') {
            dec += (hex[i] - 'a' + 10) * base;
        }
        base *= 16;
    }
    return dec;
}

//
void add_string(char *str1, char *str2) {
    // 取得字串長度
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    // 移動 str2 中的字元，為 str1 留出空間
    for (int i = len2 - 1; i >= 0; i--) {
        str2[i + len1] = str2[i];
    }
	//printf("len1:%d len2:%d str1: %s str2: %s\n",len1,len2,str1,str2);
    // 將 str1 複製到 str2 開頭
    for (int i = 0; i < len1; i++) {
        str2[i] = str1[i];
    }
	str2[len1+len2] = '\0';
	//printf("len1:%d len2:%d str1: %s str2: %s\n",len1,len2,str1,str2);
}

int main(int argc, char *argv[])
{
	int			i, c, line_count;
	char		buf[LEN_SYMBOL];
	LINE		line;

    int tmpop;
	int ProgramLength = 0;
	int symLength=0;

	queue q;
	init(&q);
	
	if(argc < 2)
	{
		printf("Usage: %s fname.asm\n", argv[0]);
	}
	else
	{
		if(ASM_open(argv[1]) == NULL)
			printf("File not found!!\n");
		else
		{
			for(line_count = 1 ; (c = process_line(&line)) != LINE_EOF; line_count++)
			{
				char Ctmp[MAX_LENGTH] = "\0";
				char Ctmp2[MAX_LENGTH]= "\0";
				if(c == LINE_ERROR)
					printf("%04d : Error\n", line_count);
				else if(c == LINE_COMMENT)
					printf("\t\tComment line\n", line_count);
				else{
					if(string_equal(line.op,"START")){
						symLength = hexToDec(line.operand1);
						ProgramLength = symLength;
					}
					//印出當前行
					if(line.operand2[0]=='\0'){
						printf("%04x %-12s %-12s %-40s (FMT=%X, ADDR=%X)\n", symLength, line.symbol, line.op, line.operand1, line.fmt, line.addressing);
					}else{
						printf("%04x %-12s %-12s %-40s,%-40s (FMT=%X, ADDR=%X)\n", symLength, line.symbol, line.op, line.operand1, line.operand2, line.fmt, line.addressing);
					}
					//如果是symbol就push到symbol table
					if(line.symbol[0]!='\0'&& !string_equal(line.op,"START")){
						strcat(Ctmp,line.symbol);
						push(&q,Ctmp,symLength);
					}
					//行數增加
					if(line.fmt==8){
						symLength +=4;
					}else if(line.fmt==4){
						symLength +=3;
					}else if(line.fmt==2){
						symLength +=2;
					}else if(line.fmt==1){
						symLength +=1;
					}else if(string_equal(line.op,"START")||string_equal(line.op,"END")){
						/*start end*/
					}
					else if(string_equal(line.op,"WORD")){
						symLength +=3;
						// strcat(Ctmp,line.symbol);
						// strcat(Ctmp , ":  ");
						// itoa(symLength,Ctmp2,16);
						// strcat(Ctmp ,Ctmp2);
						// strcat(Ctmp ,"\0");
						// push(&q,Ctmp);
					}else if(string_equal(line.op,"RESW")){
						tmpop = atoi(line.operand1);
						//printf("tmpop:%d \n",tmpop);
						symLength = symLength + tmpop*3;
						// strcat(Ctmp,line.symbol);
						// strcat(Ctmp , ":  ");
						// itoa(symLength,Ctmp2,16);
						// strcat(Ctmp ,Ctmp2 );
						// strcat(Ctmp ,"\0");
						// push(&q,Ctmp);
					}else if(string_equal(line.op,"RESB")){
						tmpop = atoi(line.operand1);
						//printf("tmpop:%d \n",tmpop);
						symLength = symLength + tmpop;
						// strcat(Ctmp,line.symbol);
						// strcat(Ctmp , ":  ");
						// itoa(symLength,Ctmp2,16);
						// strcat(Ctmp ,Ctmp2 );
						// strcat(Ctmp ,"\0");
						// push(&q,Ctmp);
					}else if(string_equal(line.op,"BYTE")){
						if(line.operand1[0]=='X'){
							if((strlen(line.operand1)-3)%2==1){
								tmpop = (strlen(line.operand1)-3)/2 + 1;
							}else{
								tmpop = (strlen(line.operand1)-3)/2;
							}
							//printf("tmpop:%d \n",tmpop);
							symLength = symLength + tmpop;
							// strcat(Ctmp,line.symbol);
							// strcat(Ctmp , ":  ");
							// itoa(symLength,Ctmp2,16);
							// strcat(Ctmp ,Ctmp2 );
							// strcat(Ctmp ,"\0");
							// push(&q,Ctmp);
						}else{
							tmpop = strlen(line.operand1)-3;
							//printf("tmpop:%d \n",tmpop);
							symLength = symLength + tmpop;
							// strcat(Ctmp,line.symbol);
							// strcat(Ctmp , ":  ");
							// itoa(symLength,Ctmp2,16);
							// strcat(Ctmp ,Ctmp2 );
							// strcat(Ctmp ,"\0");
							// push(&q,Ctmp);
							}
					}
				}
			}
			ProgramLength = symLength - ProgramLength;
			ASM_close();
		}
		printf(".\n.\n.\n");
		printf("Program length = %05x\n\n",ProgramLength);
		print_queue(&q);
		printf(".\n.\n.\n");
	}
}
