//Author: Tanmay Anand
//Graduate Student (Trad. MS) Computer Science
//Section: Prof. Mike Swift

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>

#define MAX 10

int main(int argc, char* argv[]) {
    if(argc<5) {
        printf("USAGE: \n\tbadger-fortune -f <file> -n <number> (optionally: -o <output file>) \n\t\t OR \n\tbadger-fortune -f <file> -b <batch file> (optionally: -o <output file>)\n");
        return 1;
    }

    int fortuneFlag = 0;
    int modeFlag = 2;
    int outFlag = 0;
    int outIndex = 1;
    int fortuneIndex = 0;
    int modeIndex = 0;

    //Error handling
    for(int i=1; i<argc; i++) {
        
        if((argv[i])[0]=='-' && isalpha((argv[i])[1])){
            if(strcmp(argv[i],"-f")==0){
                if((i<(argc-1) && (strcmp(argv[i+1],"-f")==0 || strcmp(argv[i+1],"-b")==0 || strcmp(argv[i+1],"-n")==0 || strcmp(argv[i+1],"-o")==0)) || (i==argc-1)) {
                    printf("ERROR: Fortune File Empty\n");
                    return 1;
                }
                else {
                    fortuneFlag = 1;
                    fortuneIndex = i+1; 
                }
            }
            else if(strcmp(argv[i],"-b")==0){
                if(modeFlag==0){
                    printf("ERROR: You can't use batch mode when specifying a fortune number using -n\n");
                    return 1;
                }

                if((i<(argc-1) && (strcmp(argv[i+1],"-f")==0 || strcmp(argv[i+1],"-b")==0 || strcmp(argv[i+1],"-n")==0 || strcmp(argv[i+1],"-o")==0)) || (i==argc-1)) {
                    printf("ERROR: Batch File Empty\n");
                    return 1;
                }
                else {
                    modeFlag = 1;
                    modeIndex = i+1; 
                }
            }
            else if(strcmp(argv[i],"-n")==0){
                if(modeFlag==1){
                    printf("ERROR: You can't specify a specific fortune number in conjunction with batch mode\n");
                    return 1;
                }

                if((i<(argc-1) && (strcmp(argv[i+1],"-f")==0 || strcmp(argv[i+1],"-b")==0 || strcmp(argv[i+1],"-n")==0 || strcmp(argv[i+1],"-o")==0)) || (i==argc-1)) {
                    printf("ERROR: Invalid Fortune Number\n");
                    return 1;
                }
                else {
                    modeFlag = 0;
                    modeIndex = i+1; 
                }
            }
            else if(strcmp(argv[i],"-o")==0) {
                outFlag = 1;
                outIndex = i+1; 
            }
            else {
                printf("ERROR: Invalid Flag Types\n");
                return 1;
            }
        }
        else if(strlen(argv[i])==1 && isalpha(argv[i][0])){
            printf("ERROR: Invalid Flag Types\n");
            return 1; 
        }
    }

    if(fortuneFlag==0) {
        printf("ERROR: No fortune file was provided\n");
        return 1;
    }

    //Open Files
    FILE *batchFile = NULL;
    FILE *fortuneFile = NULL;
    FILE *outFile = NULL;

    int numFortunes = 0, numBatches = 0, maxlen = 0, counter = -1;
    char temp[MAX];

    fortuneFile = fopen(argv[fortuneIndex],"r");

    if (!fortuneFile) {
      printf("ERROR: Can't open fortune file\n");
      return 1;
    }

    if ((outFlag==0))
      outFile = stdout;
    else    
      outFile = fopen(argv[outIndex],"w");

    if(!outFile)
      outFile = stdout;     

    int *fortuneValues;

    if (fgets(temp, MAX, fortuneFile)!=NULL) {
        numFortunes = atoi(temp);
    }
    else {
        printf("ERROR: Fortune File Empty\n");
        return 1;
    }

    fgets(temp, MAX, fortuneFile);
    maxlen = atoi(temp);
    
    char *fortunes[numFortunes];
    char *tempString = (char *) calloc((maxlen+1),sizeof(char));

    //Iterate through fortune file lines 
    while(fgets(tempString, maxlen, fortuneFile)!=NULL && (counter<numFortunes)) {
        if(strcmp(tempString,"%\n")==0){
            counter++;
            fortunes[counter] = (char *) calloc((maxlen+1),sizeof(char)); 
            continue;
        }
        else if(strcmp(tempString,"\n")==0){
            continue;
        }
        int n = strlen(fortunes[counter]);
        strcpy(fortunes[counter]+n,tempString); 
    }

    //Print the desired fortunes
    if(modeFlag == 0){
        int numFortuneOut = atoi(argv[modeIndex]);

        if(numFortuneOut<=0 || numFortuneOut>numFortunes) {
            printf("ERROR: Invalid Fortune Number\n");
            return 1; 
        }

        fprintf(outFile,"%s",fortunes[numFortuneOut-1]);
    }
    else if(modeFlag == 1){
        batchFile = fopen(argv[modeIndex],"r");

        if (!batchFile) {
            printf("ERROR: Can't open batch file\n");
            return 1;
        }

        while(fgets(temp, MAX, batchFile)!=NULL){
            fortuneValues = realloc(fortuneValues, sizeof(int)*(numBatches+1));
            fortuneValues[numBatches] = atoi(temp);
            numBatches++;
        }

        if(numBatches==0) {
            printf("ERROR: Batch File Empty\n");
            return 1;            
        }

        for(int i=0; i<numBatches; i++){
            if(fortuneValues[i]<=0 || fortuneValues[i]>numFortunes) {
                printf("ERROR: Invalid Fortune Number\n\n");
                continue;
            }
            fprintf(outFile,"%s\n\n",fortunes[fortuneValues[i]-1]);
        }
    }

    return 0;
}