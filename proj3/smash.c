#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_ARG_SIZE 100
#define MAX_BUF_SIZE 100
#define MAX_PIPE 10

int lexer(char *line, char ***args, int *num_args){
    *num_args = 0;

    char *l = strdup(line);
    if(l == NULL){
        return -1;
    }
    char *token = strtok(l, " \t\n");
    while(token != NULL){
        (*num_args)++;
        token = strtok(NULL, " \t\n");
    }
    free(l);

    *args = malloc(sizeof(char **) * *num_args);
    *num_args = 0;
    token = strtok(line, " \t\n");
    while(token != NULL){
        char *token_copy = strdup(token);
        if(token_copy == NULL){
            return -1;
        }
        (*args)[(*num_args)++] = token_copy;
        token = strtok(NULL, " \t\n");
    }
    return 0;
}

int search(char **array, char *ch, int l) {

    for(int i=0; i<l; i++) {
        if(strcmp(array[i],ch)==0){
            return i;
        }
    }
    return -1;
}

void print_error() {

    char err_msg[30] = "An error has occurred\n"; 
    write(STDERR_FILENO, err_msg, strlen(err_msg)); 
}


int execute_file(char **words, int wc, char *inp_stream, char* outp_stream, int std_err_pipe){
    if(wc<=0) {
        return 0;
    }

    int pid = 0, status = 0, k=0;
    pid = fork(); 
    if(pid == 0) { 

        char *args[wc+1];

        for(int i=0;i<wc;i++) {
            args[i] = words[i];
        }
        args[wc] = NULL;
        if(strlen(args[0])>0 && args[0][0]!='/') {
            char* temp = malloc(sizeof(char) * (strlen(args[0])+1)); 
            strcat(temp, "/");
            strcat(temp, "\0");
            strcat(temp, args[0]);
            strcpy(args[0], temp);
            free(temp);
        }
        if(access(args[0], X_OK) != 0) { 
            print_error();
            _exit(0);
        }
        if(outp_stream) {
            int fp = open(outp_stream, O_CREAT|O_WRONLY|O_TRUNC, 0644);
            if(fp > 0) { 
                dup2(fp, STDOUT_FILENO); 
                if(std_err_pipe==1) {
                    dup2(fp, STDERR_FILENO);
                }
            }
        }
        k = execv(args[0], args); 
        perror("");
        _exit(0);
    } else if( pid < 0) { 
        print_error();
        return 1;
    } else { 
        wait(&status); 
        k = WEXITSTATUS(status); 
        return k;
    }
    return k;
}

int fork_pipes (int n, char **cmd[], char* outp_stream)
{
  int   p[2];
  pid_t pid;
  int   fd_in = 0;

  while (*(cmd) != NULL)
    {
      pipe(p);
      if ((pid = fork()) == -1)
      {
        exit(EXIT_FAILURE);
      }
      else if (pid == 0)
      {
        if(strlen((*cmd)[0])>0 && (*cmd)[0][0]!='/') {
            char* temp = malloc(sizeof(char) * (strlen((*cmd)[0])+1)); 
            strcat(temp, "/");
            strcat(temp, "\0");
            strcat(temp, (*cmd)[0]);
            strcpy((*cmd)[0], temp);
            free(temp);
        } 

        if(access((*cmd)[0], X_OK) != 0) { 
            print_error();
            _exit(0);
        }

        dup2(fd_in, 0); 
        if (*(cmd + 1) != NULL)
            dup2(p[1], 1);
        else {
            if(outp_stream) {
                int fp = open(outp_stream, O_CREAT|O_WRONLY|O_TRUNC, 0644);
                if(fp > 0) { 
                    dup2(fp, STDOUT_FILENO); 
                    dup2(fp, STDERR_FILENO);
                }
            }
        }
        close(p[0]);

        execv((*cmd)[0], *cmd);
        exit(EXIT_FAILURE);
    }
    else
        {
          wait(NULL);
          close(p[1]);
          fd_in = p[0];
          cmd++;
        }
    }
    return 0;
}

int run_cmd(char **words, int wc, FILE* fp) {
    if(wc <= 0){
        return 0;
    }

    int cnt = 1;
    if(strcmp(words[0],"loop")==0) {
        cnt = atoi(words[1]);
        if(wc>=2) {
            words += 2;
            wc -= 2;
        }
    }
    for(int i=0;i<cnt;i++) {
 
        if(wc>0 && (strcmp(words[0],"exit")==0)){
            return 1;
        }
        else if(wc>0 && (strcmp(words[0],"cd")==0)){
            int status = chdir(words[1]); 

            if(status == -1) {
                print_error();
            }
        }
        else if(wc>0 && (strcmp(words[0],"pwd")==0)){
            char *buffer = (char *)malloc(MAX_BUF_SIZE * sizeof(char));
            getcwd(buffer, MAX_BUF_SIZE );
            fprintf(stdout,"%s\n",buffer);
            free(buffer);
        }
        else if(wc>0) {
            char *redirect = NULL;
            int index = search(words, ">", wc);
            int wc_temp = wc;
            if((index > 0) && (index == (wc-2)) && (strcmp(words[wc-1],">")!=0))
            {   redirect = words[wc-1];  
                wc_temp-=2;
            }
            else if(index != -1){
                print_error();
                continue; 
            }
         
            if(search(words,"|",wc)==-1) {
                execute_file(words, wc_temp, NULL, redirect, 1);
            }
            else{
                char **words_args = malloc((wc_temp+1)*sizeof(char *));

                for(int i=0;i<wc_temp;i++) {
                    words_args[i] = words[i];
                }
                words_args[wc_temp] = NULL; 
                int t = 0, cnt = 0;

                char **commands[MAX_PIPE] = {NULL};

                for(int i=0; i<MAX_PIPE; i++)
                    commands[i] = (char **)malloc((MAX_PIPE)*sizeof(char **));
                commands[cnt] = &(words_args[0]);

                do{
                    t = (search(words_args,"|",wc_temp) == -1) ? wc_temp : search(words_args,"|",wc_temp); 
                    commands[++cnt] = &(words_args[t+1]); 
                    words_args[t] = NULL;
                    words_args += t+1;
                    wc_temp -= (t+1);

                } while(wc_temp>0);
                commands[cnt] = NULL;

                fork_pipes (cnt, commands, redirect); 
        
            }
        }

    }
    return 0;
}

int line_read(FILE *fp) {
    size_t bufsize = 1024;
    char *buffer = (char *)malloc(bufsize * sizeof(char));
    
    getline(&buffer, &bufsize, fp);
    int wc=0, index = 0;
    char ** words;
    
    lexer(buffer, &words, &wc); 
    free(buffer);

    if(wc==0)
        return 0;

    if((index = search(words,";",wc)) == -1) {

        return run_cmd(words, wc, fp);
    }

    do {
        while(wc>0 && (strcmp(words[0],";")==0)) {
            words++;
            wc--;
            index = (search(words,";",wc)==-1) ? wc : search(words,";",wc);
        }
    
        index = (search(words,";",wc)==-1) ? wc : search(words,";",wc);
        int return_val = run_cmd(words, index, fp);

        words += index+1;
        wc -= (index+1);
        if(return_val == 1) 
            return 1;
    } while(wc>0);
    return 0;
}

int main(int argc, char* argv[]) {

    FILE* file;

    int ret_val;;
    setbuf(stdout,NULL);
    while(1) { 
        if(argc == 1) { 
            fprintf(stdout, "smash> ");
            ret_val = line_read(stdin);
            if(ret_val  == 0) {
                continue;
            }
            else {
                return 0;
            }
        } else if(argc == 2) {
            file = fopen(argv[1], "r");
            if(file == NULL) {
                print_error();
                exit(1);
            }
            while(1) {
                //fflush(stdout);
                fprintf(stdout, "smash> "); 
                ret_val  = line_read(file); 
                if(ret_val  == 0) { 
                    continue;
                }
                else {
 
                    return 0;
                }
            }
        } else { 
            print_error();
            exit(1);
        }
    }
    return 0;
}