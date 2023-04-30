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
    char sName[MAX_SIZE][LEN_SYMBOL]; //放symbol名字
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
int serchSymbolTable(queue *q,char str1[],int *serchFlag){
	*serchFlag = 0;
	if(str1[0]=='#'){//immediate addressing
		//把#拿掉
		*serchFlag = 2;
		for(int i = 0;i < strlen(str1);i++){
			str1[i] = str1[i+1];
		}
	}
	//開始比對
	for (int i = q->front; i < q->rear; i++) {
		//printf("size:%s is %d size:%s is %d\n",str1,strlen(str1),q->sName[i],strlen(q->sName[i]));
        if(string_equal(q->sName[i],str1)){
			//printf("return1: %X\n",q->data[i]);
			return q->data[i];
		}
    }
	if(*serchFlag == 2){
		int num = atoi(str1);
		*serchFlag = 1;
		//printf("return2: %d\n",num);
		return num;
	}
	//printf("Error! no found symbol\n");
	return 0;
}

//合併陣列(16進位)
int merge(int arr[]) {
    int result = 0;
    for (int i = 0; i < 8; i++) {
		if(arr[i]==-1){
			//不做
		}else{
			result = result * 16 + arr[i];
		}  
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
		//FILE *fp = 
		if(ASM_open(argv[1]) == NULL)
			printf("File not found!!\n");
		else
		{
			for(line_count = 1 ; (c = process_line(&line)) != LINE_EOF; line_count++)
			{
				char Ctmp[MAX_LENGTH] = "\0";
				//char Ctmp2[MAX_LENGTH]= "\0";
				if(c == LINE_ERROR){
					//printf("%04d : Error\n", line_count);
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
					// //印出當前行
					// if(line.operand2[0]=='\0'){
					// 	printf("%04x %-12s %-12s %-40s (FMT=%X, ADDR=%X)\n", symLength, line.symbol, line.op, line.operand1, line.fmt, line.addressing);
					// }else{
					// 	printf("%04x %-12s %-12s %-40s,%-40s (FMT=%X, ADDR=%X)\n", symLength, line.symbol, line.op, line.operand1, line.operand2, line.fmt, line.addressing);
					// }
					if(line.symbol[0]!='\0'&& !string_equal(line.op,"START")){ //push到symbol table
						strcat(Ctmp,line.symbol);
						strcat(Ctmp,"\0");
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
			
			/*
			printf(".\n.\n.\n");
			printf("Program length = %05x\n\n",ProgramLength);
			print_queue(&q);
			printf(".\n.\n.\npass2\n");
			*/

			//pass 2
			rewind(ASM_fp);
			int PC = 0;//PC指標
			int Base = 0;//base指標
			printf("H%-6s%06x%06x\n",programName,startLoc,ProgramLength);//表頭紀錄
			char totalOp[200][LEN_SYMBOL];//把產生出的opcode放在這裡，最後印出本文紀錄時比較好用，如果遇到FM0紀錄為-1
			int totalOpPointer = 0;//紀錄輸出到第幾組opcode，一行最多1E個，遇到FM0要強制換行
			Instruction *optmp;//用來存opcode的執行碼
			int tmpSerch = 0 ;//用來計算disp
			int count[2][200] ;//count[0][0]=有幾行，count[0][n]=每行長度，count[1][n]=該行起始位子
			int countFlag = 0;
			int serchFlag = 0;
			float judgeByte = 0;
			//reset array
			for(int i=0;i<2;i++){
				for(int j=0;j<200;j++){
					count[i][j]=0;
				}
			}
			count[0][0] = 1;
			for(int i=0;i<200;i++){
					strcpy(totalOp[i],"\0");
			}
			//modification record
			int modification[2][200];//modification[0][n]=紀錄LOC，[1][n]=紀錄要改變幾個half byte
			int modFlag = 0;//如果start的位子不是0就不需要產生modification record
			int modCounter = 0;//紀錄有幾個修改的code
			for(int i=0;i<2;i++){
				for(int j=0;j<200;j++){
					modification[i][j]=0;
				}
			}

			for(line_count = 1 ; (c = process_line(&line)) != LINE_EOF; line_count++)
			{
				char Ctmp[MAX_LENGTH];
				memset(Ctmp, 0, sizeof(Ctmp));//重製Ctmp
				int opCode[8] = {-1,-1,-1,-1,-1,-1,-1,-1};//存放opcode
				//char Ctmp2[MAX_LENGTH]= "\0";
				if(c == LINE_ERROR)
					printf("%04d : Error\n", line_count);
				else if(c == LINE_COMMENT){
					//printf("\t\tComment line\n", line_count);
				}	
				else{
					//設定PC的開始位子
					if(string_equal(line.op,"START")){ 
						PC = hexToDec(line.operand1);
						strcpy(programName,line.symbol);//存程式名稱
						if (PC==0) //需要重新定址
						{
							modFlag = 2;//還不確定有沒有FM4，先給2如果有FM4就給1
						}
						
						continue;//從下一行開始
					}
					//設定B的開始位子
					if(string_equal(line.op,"LDB")){//先預設他只會有immediate addressing
						if(line.addressing == ADDR_IMMEDIATE ){//如果是immediate addressing
							if(line.operand1[1]<57&&line.operand1[1]>48){//如果是#數字
								line.operand1[0] = '0';
								Base = atoi(line.operand1);
								line.operand1[0] = '#';
							}else{//如果是#接symbol
								Base = serchSymbolTable(&q,line.operand1,&serchFlag);
							}
						}
					}
					if(string_equal(line.op,"BASE")){ 
						continue;//從下一行開始
					}
					symLength = PC;//紀錄當前行的位子
					//PC往下指
					if(line.fmt==8){
						PC +=4;
					}else if(line.fmt==4){
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
					if(line.op[0]=='+'){//如果是FM4要把'+'拿掉
						modFlag = 1;//產生modification record
						modification[0][modCounter] = symLength;//紀錄當前位子
						modification[1][modCounter++] = 5;
						for(int i = 0;i < strlen(line.op);i++){
								line.op[i] = line.op[i+1];
							}
					}
					optmp = is_opcode(line.op);
					opCode[0] = optmp->code/16;
					opCode[1] = optmp->code%16;
					if (line.fmt == 0){
						//判斷是RESW、RESB或是WORD、BYTE
						if(string_equal(line.op,"RESW")||string_equal(line.op,"RESB")){
							sprintf(totalOp[totalOpPointer], "%d", -1);
						}else if (string_equal(line.op,"WORD"))
						{
							opCode[2] = atoi(line.operand1);
							//itoa(merge(opCode),totalOp[totalOpPointer],16);
							sprintf(totalOp[totalOpPointer], "%X" ,merge(opCode));//存入大寫的16進位
						}else if (string_equal(line.op,"BYTE"))
						{
							line.operand1[strlen(line.operand1)-1] = '\0';//讓最後一個'變成空字元
							if(line.operand1[0]=='C'){
								int changetmp = strlen(line.operand1);
								for(int i = 0;i < changetmp-2;i++){//把C'拿掉，讓字串往前兩個位元
									line.operand1[i] = line.operand1[i+2];
								}
								line.operand1[changetmp-2] = line.operand1[changetmp-1] = '\0';
								//printf("op1.len: %X ",strlen(line.operand1));
								//轉ASCII
								int tmp = 0;
								for(int i = 0;i < strlen(line.operand1);i++){//把operand1的每個字元轉ASCII存到tmp中
									tmp = tmp*16*16 + line.operand1[i];
								}
								//itoa(tmp,totalOp[totalOpPointer],16);//將tmp轉成16進位放入totalOp
								sprintf(totalOp[totalOpPointer], "%X" ,tmp);//存入大寫的16進位
							}else//X
							{
								int changetmp = strlen(line.operand1);
								for(int i = 0;i < changetmp-2 ;i++){//把X'拿掉，讓字串往前兩個位元
									line.operand1[i] = line.operand1[i+2];
								}
								line.operand1[changetmp-2] = line.operand1[changetmp-1] = '\0';
								strcpy(totalOp[totalOpPointer],line.operand1);
							}
						}else if(string_equal(line.op,"END")){//忽略END這行
							continue;
						}
					}else if(line.fmt==1){
						//itoa(merge(opCode),totalOp[totalOpPointer],16);
						sprintf(totalOp[totalOpPointer], "%X" ,merge(opCode));//存入大寫的16進位
					}else if(line.fmt == 2){
						//operand1
						if(string_equal(line.operand1,"A")){
							opCode[2] = 0; 
						}else if(string_equal(line.operand1,"X")){
							opCode[2] = 1; 
						}else if(string_equal(line.operand1,"S")){
							opCode[2] = 4; 
						}else if(string_equal(line.operand1,"T")){
							opCode[2] = 5; 
						}else if(string_equal(line.operand1,"F")){
							opCode[2] = 6; 
						}
						//operand2
						if(string_equal(line.operand2,"\0")){
							opCode[3] = 0; 
						}else if(string_equal(line.operand2,"A")){
							opCode[3] = 0; 
						}else if(string_equal(line.operand2,"X")){
							opCode[3] = 1; 
						}else if(string_equal(line.operand2,"S")){
							opCode[3] = 4; 
						}else if(string_equal(line.operand1,"T")){
							opCode[2] = 5; 
						}else if(string_equal(line.operand1,"F")){
							opCode[2] = 6; 
						}
						//itoa(merge(opCode),totalOp[totalOpPointer],16);
						sprintf(totalOp[totalOpPointer], "%X" ,merge(opCode));//存入大寫的16進位
					}else if(line.fmt == 4){
						int flag=0;//flag = 1為PC，2為Base，0為無解，3為immediate addr又接數字
						//先算disp，決定是用PC還是Base
						tmpSerch = serchSymbolTable(&q,line.operand1,&serchFlag);
						opCode[3] = opCode[4] = 0;
						if (string_equal(line.op,"RSUB"))//RSUB disp = 0
						{
							flag = 1;
							tmpSerch = 0;
						}else if(serchFlag==1){//immediate addr又接數字
							flag = 3;
						}
						else if((tmpSerch-PC)<2047||(tmpSerch-PC)>-2048){ //PC
							tmpSerch -= PC;
							flag = 1;
						}else if ((tmpSerch-PC)<4095||(tmpSerch-PC)>0){ //Base
							tmpSerch -= Base;
							flag = 2;
						}
						//printf("type: %d tmpSerch: %X ",flag,tmpSerch);
						opCode[5] = tmpSerch;
						//判斷addrresing mod
						if(line.addressing>=8)//index addr
						{ 
							line.addressing -= 8;
							if (line.addressing == 1)//simple addr
							{
								opCode[1] += 3; 
							}else if (line.addressing == 2)//immediate addr
							{
								opCode[1] += 1;
							}else if (line.addressing == 4)//indirect addr
							{
								opCode[1] += 2;
							}
							if(flag == 1){//xbpe = 1010
								opCode[2] = 10;
							}else if (flag == 2){//1100
								opCode[2] = 12;
							}else if(flag == 3){//1000
								opCode[2] = 8;
							}
						}else if (line.addressing == 1)//simple addr
						{
							opCode[1] += 3; 
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
							}else if(flag == 3){//0000
								opCode[2] = 0;
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
						//itoa(merge(opCode),totalOp[totalOpPointer],16);
						sprintf(totalOp[totalOpPointer], "%X" ,merge(opCode));//存入大寫的16進位
						char ctmp[7];
						memset(ctmp, 0, sizeof(ctmp));//重製Ctmp
						if(strlen(totalOp[totalOpPointer])==5){//前面少個0，把0補上
							strcat(ctmp,"0");
							strcat(ctmp,totalOp[totalOpPointer]);
							strcpy(totalOp[totalOpPointer],ctmp);
						}
					}else if (line.fmt == 8)//FM4
					{
						opCode[3] = opCode[4] = opCode[5] = opCode[6] = 0;
						tmpSerch = serchSymbolTable(&q,line.operand1,&serchFlag);
						if(line.addressing == 2){//immediate addr
							opCode[1] += 1; 
						}else if(line.addressing == 4){//indirect addr
							opCode[1] += 2;
						}else{//simple
							opCode[1] += 3;
						}
						opCode[2] = 1;//xbpe 0001
						opCode[7] = tmpSerch;
						//itoa(merge(opCode),totalOp[totalOpPointer],16);
						sprintf(totalOp[totalOpPointer], "%X" ,merge(opCode));//存入大寫的16進位
						char ctmp[9];
						memset(ctmp, 0, sizeof(ctmp));//重製ctmp
						if(strlen(totalOp[totalOpPointer])==7){//前面少個0，把0補上
							strcat(ctmp,"0");
							strcat(ctmp,totalOp[totalOpPointer]);
							strcpy(totalOp[totalOpPointer],ctmp);
						}
					}

					//printf("PC: %X ",PC);
					//printf("totalop: %s op: %s \n",totalOp[totalOpPointer],line.op);
					//printf("count[0][0]: %X line lenght: %X strlen: %X\n",count[0][0],count[0][count[0][0]],strlen(totalOp[totalOpPointer]));
					//先處理本文紀錄的一行要有多長並記錄該行開始位子
					if (string_equal(totalOp[totalOpPointer],"-1")&&countFlag!=2)//遇到RESW、RESB需要換行，增加總長度，並讓flag歸零
					{
						countFlag = 2;//如果countFlag = 2代表遇到RESW或RESB，連續遇到不連續換行
						
					}else if((count[0][count[0][0]]+strlen(totalOp[totalOpPointer])/2)>30){//該行放不下了
						countFlag = 0;
						count[0][0]++;
					}
					//記錄換行時的開始位子
					if(countFlag ==0){
						count[1][count[0][0]] = symLength;
						countFlag = 1;
						//printf("line: %X %d\n",count[1][count[0][0]],count[0][0]);
					}
					if(!string_equal(totalOp[totalOpPointer],"-1")){
						//當遇到RESW、RESB後遇到第一個可以加入totalop的opcode
						if(countFlag==2){
							countFlag = 1;
							count[0][0]++;//先換行
							count[1][count[0][0]] = symLength;//在記錄開始位子
						}
						count[0][count[0][0]] += strlen(totalOp[totalOpPointer])/2;
						if(strlen(totalOp[totalOpPointer])/2==0){ //遇到WORD只有1十六進位時
							count[0][count[0][0]] += 1;
							judgeByte +=0.5;
						}
						//如果連續遇到兩個1十六進位的時候，上面會多給他一個空間，但是兩個1十六進位可以放在一個BYTE裡
						if(judgeByte==1){
							count[0][count[0][0]] -= 1;
							judgeByte = 0;
						}
					}
					
					totalOpPointer++;
				}
			}
			//處理本文紀錄
			//printf("count[0][0] %d\n",count[0][0]);
			int totalOpPointer2 = 0;//指本文處理中opcode的位子
			for(int k=1;k<=count[0][0];k++){
				printf("T%06X%X",count[1][k],count[0][k]);
				//印出該長度的內容
				int i=0;
				while(i<count[0][k]){
					if(!string_equal(totalOp[totalOpPointer2],"-1")){//不是RESB、RESW才印
						printf("%s",totalOp[totalOpPointer2]);
						i +=strlen(totalOp[totalOpPointer2++])/2;
					}else{//遇到-1忽略
						totalOpPointer2++;
					}
					//不確定要不要
					//如果全部opcode都印出來就跳出while迴圈
					if(totalOpPointer2==totalOpPointer){
						break;
					}
				}
				printf("\n");
				//不確定要不要
				//如果全部opcode都印出來就跳出while迴圈
				if(totalOpPointer2==totalOpPointer){
					break;
				}
				
			}
			//修正紀錄
			if(modFlag == 1){
				for (int i = 0; i < modCounter; i++)
				{
					printf("M%06X%0X\n",modification[0][i],modification[1][i]);
				}
				
			}
			
			//結束紀錄
			printf("E%06X",count[1][1]);//印出第一個可執行指令的地址
			ASM_close();
		}
	}
}
