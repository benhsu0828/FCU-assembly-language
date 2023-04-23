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
								strcpy(line->operand2, buf);
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
    char sName[MAX_SIZE][MAX_LENGTH]; //放symbol名字
	int data[MAX_SIZE];				//放sybol的位子
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
    strcpy(q->sName[q->rear], value);
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
    return q->sName[q->front];
}

void print_queue(queue *q) {
    int i = q->front;
    if (is_empty(q)) {
        printf("Queue is empty\n");
        return;
    }
    for (i = q->front; i < q->rear; i++) {
        printf("%10s: %06x \n", q->sName[i],q->data[i]);
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

//搜尋symbol table
int serchSymbolTable(queue *q,char str1[]){
	if(str1[0]=='#'){//immediate addressing
		//把#拿掉
		for(int i = 0;i < strlen(str1);i++){
			str1[i] = str1[i+1];
		}
	}
	//開始比對
		for (int i = q->front; i < q->rear; i++) {
        	if(string_equal(q->sName[i],str1)){
				return q->data;
			}
    	}
		printf("Error! no found symbol\n");
		return 0;
}

//合併陣列(16進位)
int merge(int arr[]) {
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result = result * 16 + arr[i];
    }
    return result;
}

int main(int argc, char *argv[])
{
	int			i, c, line_count;
	char		buf[LEN_SYMBOL];
	LINE		line;

    int tmpop;
	int ProgramLength = 0;
	int symLength=0;
	char programName[MAX_LENGTH];//程式名稱
	int startLoc;

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
				//char Ctmp2[MAX_LENGTH]= "\0";
				if(c == LINE_ERROR){
					printf("%04d : Error\n", line_count);
				}
				else if(c == LINE_COMMENT){
					//printf("\t\tComment line\n", line_count);
				}
				else{
					if(string_equal(line.op,"START")){ //抓開始位子
						symLength = hexToDec(line.operand1);
						ProgramLength = symLength;
						startLoc = symLength;
						strcpy(programName,line.symbol);
					}
					//printf("%04x %-12s %-12s %-40s,%-40s (FMT=%X, ADDR=%X)\n", symLength, line.symbol, line.op, line.operand1, line.operand2, line.fmt, line.addressing);
					if(line.symbol[0]!='\0'&& !string_equal(line.op,"START")){ //push到symbol table
						strcat(Ctmp,line.symbol);
						push(&q,Ctmp,symLength);//把symbol放到symbol table
					}
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
					}else if(string_equal(line.op,"RESW")){
						tmpop = atoi(line.operand1);
						symLength = symLength + tmpop*3;
					}else if(string_equal(line.op,"RESB")){
						tmpop = atoi(line.operand1);
						symLength = symLength + tmpop;
					}else if(string_equal(line.op,"BYTE")){
						if(line.operand1[0]=='X'){
							if((strlen(line.operand1)-3)%2==1){
								tmpop = (strlen(line.operand1)-3)/2 + 1;
							}else{
								tmpop = (strlen(line.operand1)-3)/2;
							}
							symLength = symLength + tmpop;
						}else{
							tmpop = strlen(line.operand1)-3;
							symLength = symLength + tmpop;
							}
					}
				}
			}
			ProgramLength = symLength - ProgramLength;

			//pass 2
			rewind(argv[1]);
			int PC = 0;//PC指標
			int Base = 0;//base指標
			printf("H%-6s%06x%06x",programName,startLoc,ProgramLength);//表頭紀錄
			char totalOp[200][MAX_LENGTH];//把產生出的opcode放在這裡，最後印出本文紀錄時比較好用，如果遇到FM0紀錄為-1
			int totalOpPointer = 0;//紀錄輸出到第幾組opcode，一行最多1E個，遇到FM0要強制換行
			Instruction *optmp;//用來存opcode的執行碼
			int tmpSerch = 0 ;//用來計算disp

			for(line_count = 1 ; (c = process_line(&line)) != LINE_EOF; line_count++)
			{
				char Ctmp[MAX_LENGTH] = "\0";
				int opCode[4] = {NULL,NULL,NULL,NULL};//存放opcode
				//char Ctmp2[MAX_LENGTH]= "\0";
				if(c == LINE_ERROR)
					printf("%04d : Error\n", line_count);
				else if(c == LINE_COMMENT)
					printf("\t\tComment line\n", line_count);
				else{
					//設定PC的開始位子
					if(string_equal(line.op,"START")){ 
						PC = hexToDec(line.operand1);
						strcpy(programName,line.symbol);//存程式名稱
					}
					//設定B的開始位子
					if(string_equal(line.op,"LDB")){//先預設他只會有immediate addressing
						if(line.addressing == ADDR_IMMEDIATE ){//如果是immediate addressing
							if(line.operand1[1]<57&&line.operand1[1]>48){//如果是#數字
								line.operand1[0] = '0';
								Base = atoi(line.operand1);
								line.operand1[0] = '#';
							}else{//如果是#接symbol
								Base = serchSymbolTable(&q,line.operand1);
							}
						}
					}
					//PC往下指
					if(line.fmt==4||line.fmt==3){
						PC +=3;
					}else if(line.fmt==2){
						PC +=2;
					}else if(line.fmt==1){
						PC +=1;
					}else if(string_equal(line.op,"START")||string_equal(line.op,"END")){
						/*start end*/
					}
					else if(string_equal(line.op,"WORD")){
						PC +=3;
					}else if(string_equal(line.op,"RESW")){
						tmpop = atoi(line.operand1);
						PC = PC + tmpop*3;
					}else if(string_equal(line.op,"RESB")){
						tmpop = atoi(line.operand1);
						PC = PC + tmpop;
					}else if(string_equal(line.op,"BYTE")){
						if(line.operand1[0]=='X'){
							if((strlen(line.operand1)-3)%2==1){
								tmpop = (strlen(line.operand1)-3)/2 + 1;
							}else{
								tmpop = (strlen(line.operand1)-3)/2;
							}
							PC = PC + tmpop;
						}else{
							tmpop = strlen(line.operand1)-3;
							PC = PC + tmpop;
							}
					}
					//先判斷FM1 2 4 8
					optmp = is_opcode(line.op);
					opCode[0] = optmp->code/16;
					opCode[1] = optmp->code%16;
					if (line.fmt == 0){
						//判斷是RESW、RESB或是WORD、BYTE
						if(string_equal(line.op,"RESW")||string_equal(line.op,"RESB")){
							strcpy(totalOp[totalOpPointer],"-1");
						}else if (string_equal(line.op,"WORD"))
						{
							opCode[3] = atoi(line.operand1);
						}else if (string_equal(line.op,"BYTE"))
						{
							line.operand1[strlen(line.operand1)-1] = '\0';//讓最後一個'變成空字元
							for(int i = 0;i < strlen(line.operand1);i++){//把C|X''拿掉，讓字串往前兩個位元且不要讓最後一位執行到
								line.operand1[i] = line.operand1[i+2];
							}
							if(line.operand1[0]=='C'){
								//轉ASCII
								int tmp = 0;
								for(int i = 0;i < strlen(line.operand1);i++){//把operand1的每個字元轉ASCII存到tmp中
									tmp = tmp*16*16 +line.operand1[i];
								}
								itoa(tmp,totalOp[totalOpPointer],16);//將tmp轉成16進位放入totalOp
							}else//X
							{
								strcpy(totalOp[totalOpPointer],line.operand1);
							}
						}
						
					}else if(line.fmt==1){
						itoa(merge(opCode),totalOp[totalOpPointer],16);
					}else if(line.fmt == 2){
						//operand1
						if(string_equal(line.operand1,'A')){
							opCode[2] = 0; 
						}else if(string_equal(line.operand1,'X')){
							opCode[2] = 1; 
						}else if(string_equal(line.operand1,'S')){
							opCode[2] = 4; 
						}else if(string_equal(line.operand1,'T')){
							opCode[2] = 5; 
						}else if(string_equal(line.operand1,'F')){
							opCode[2] = 6; 
						}
						//operand2
						if(string_equal(line.operand2,'\0')){
							opCode[3] = 0; 
						}else if(string_equal(line.operand2,'A')){
							opCode[3] = 0; 
						}else if(string_equal(line.operand2,'X')){
							opCode[3] = 1; 
						}else if(string_equal(line.operand2,'S')){
							opCode[3] = 4; 
						}else if(string_equal(line.operand2,'T')){
							opCode[3] = 5; 
						}else if(string_equal(line.operand2,'F')){
							opCode[3] = 6; 
						}
						itoa(merge(opCode),totalOp[totalOpPointer],16);
					}else if(line.fmt == 4){
						int flag=0;//flag = 1為PC，2為Base，0為無解
						//先算disp，決定是用PC還是Base
						tmpSerch = serchSymbolTable(&q,line.operand1);
						if((tmpSerch-PC)>2047||(tmpSerch-PC)<-2048){ //PC
							tmpSerch -= PC;
							flag = 1;
						}else if ((tmpSerch-PC)>4095||(tmpSerch-PC)<0){ //Base
							tmpSerch -= Base;
							flag = 2;
						}
						opCode[3] = tmpSerch;
						//判斷addrresing mod
						if(line.addressing>=8)//index addr
						{ 
							opCode[1] += 4; //simple adrresing
							if(flag == 1){//xbpe = 1010
								opCode[2] = 10;
							}else if (flag == 2){//1100
								opCode[2] = 12;
							}
						}else if (line.addressing == 1)//simple addr
						{
							opCode[1] += 4; 
							if(flag == 1){//xbpe = 0010
								opCode[2] = 2;
							}else if (flag == 2){//0100
								opCode[2] = 4;
							}
						}else if (line.addressing == 2)//immediate addr
						{
							opCode[1] += 1;
							if(flag == 1){//xbpe = 0010
								opCode[2] = 2;
							}else if (flag == 2){//0100
								opCode[2] = 4;
							}
						}else if (line.addressing == 4)//indirect addr
						{
							opCode[1] += 2;
							if(flag == 1){//xbpe = 0010
								opCode[2] = 2;
							}else if (flag == 2){//0100
								opCode[2] = 4;
							}
						}
						itoa(merge(opCode),totalOp[totalOpPointer],16);
					}else if (line.fmt == 8)//FM4
					{
						tmpSerch = serchSymbolTable(&q,line.operand1);
						if(line.addressing == 2){//immediate addr
							opCode[1] += 1; 
						}else if(line.addressing == 4){//indirect addr
							opCode[1] += 2;
						}else{//simple
							opCode[1] += 4;
						}
						opCode[3] = tmpSerch;
						itoa(merge(opCode),totalOp[totalOpPointer],16);
					}
					totalOpPointer++;
				}
			}
			//處理本文紀錄

			ASM_close();
			/*
			printf(".\n.\n.\n");
			printf("Program length = %05x\n\n",ProgramLength);
			print_queue(&q);
			printf(".\n.\n.\n");
			*/
		}
	}
}
