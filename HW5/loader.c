#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//判斷string是否一樣
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

//印出estab
//col1  判斷symbol要當contral section還是symbol
//loc  位址 
//len  長度，如果col1 == H 才要印
void printESTAB(char col1,char symbol[],char loc[],char len[]){ 
    if(col1 == 'H'){
        printf("%-20s\t\t\t %-10s\t %-10s \n", symbol, loc, len);
    }else if(col1 == 'D'){
        printf("\t\t\t %-10s\t %-10s\t \n", symbol, loc);
    }
}

int main(int argc, char *argv[])
{
    printf("contral section \t symbol name \t location \t lenght\n");
    /*printf("argc :%d \n",argc);
    for(int i = 0; i<argc ; i++){
        printf("argv[%d]:%s \n",i,argv[i]);
    }*/
    int contralSectionLen = 0;//每一個控制區段的位址
    contralSectionLen = hexToDec(argv[1]);//最一開始的位址為load到記憶體的位址

    for(int round = 2; round<argc ; round++){
        char col1;//每行的第一個字
        char contralSection[7];//控制區段
        char symbolName[7];//符號名稱
        char loc[7];//位置
        char startLoc[7];//開始位子(判斷有沒有在0000)
        char lenght[7];//程式長度
        int len = 0;//用10來計算程式的長度
        int file = 1;
        //重製字串
        memset(contralSection, 0, sizeof(contralSection));
        memset(symbolName, 0, sizeof(symbolName));
        memset(loc, 0, sizeof(loc));
        memset(lenght, 0, sizeof(lenght));
        memset(startLoc, 0, sizeof(startLoc));

        //讀每個檔
        FILE *fp = fopen(argv[round], "r");
        while (file)//讀每一行
        {
            //重製字串
            memset(contralSection, 0, sizeof(contralSection));
            memset(symbolName, 0, sizeof(symbolName));
            memset(loc, 0, sizeof(loc));
            memset(lenght, 0, sizeof(lenght));
            memset(startLoc, 0, sizeof(startLoc));
            
            //抓取每一行第一個字，看是哪種紀錄
            col1 = fgetc(fp);
            if(col1=='H'){//如果是Header
                fgets(contralSection ,7 , fp);//存控制區段
                fgets(startLoc ,7 , fp);//開始位子
                if(hexToDec(startLoc)!=0){//不是放在0000的位子
                    int tmpLoc = hexToDec(startLoc);
                }
                fgets(lenght ,7 , fp);//整個檔案的長度
                len = contralSectionLen;//每一個新的檔案的開始位址
                sprintf(loc, "%X" ,contralSectionLen);//位址
                contralSectionLen += hexToDec(lenght);//幫下一個檔案的contral section開始位址做處理

                //印出
                printESTAB('H',contralSection ,loc ,lenght);
            }
            else if(col1 == 'D'){
                char Ctmp[75];
                memset(Ctmp, '0', sizeof(Ctmp));
                int tmp = 0;
                fgets(Ctmp , 74,fp);
                do
                {
                    strncpy(symbolName, Ctmp+tmp, 6);//從Ctmp的第tmp個字元複製6個字元
                    symbolName[6] = '\0';
                    tmp += 6;
                    strncpy(loc, Ctmp+tmp, 6);//從Ctmp的第tmp個字元複製6個字元
                    loc[6] = '\0';
                    tmp += 6;

                    // fgets(symbolName ,7 , fp);//symbol名稱
                    // fgets(loc ,7 , fp);//符號的相對位址

                    //幫符號的相對位址加上記憶體位址再存入loc
                    int lolcTmp = len + hexToDec(loc);
                    sprintf(loc, "%X" ,lolcTmp);//存回loc

                    //印出
                    printESTAB('D',symbolName ,loc ,"0");
                    
                } while (Ctmp[tmp]!='\0'&&Ctmp[tmp]!='\n');//讀到換行tmp<strlen(Ctmp)-1
            }
            else if(col1 == 'R'||col1 == 'T'||col1 == 'E'||col1 == EOF){//external define都讀完了
                file = 0; 
            }
        }
        fclose(fp);
    }

    return 0;
}

