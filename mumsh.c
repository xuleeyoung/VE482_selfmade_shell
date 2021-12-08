// main

#include "io.h"
#include "handler.h"
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_LEN_LINE 1024

cmd_t *job_t = NULL;

int main()
{
    job_t = cmd_constructor();
    while(1)
    {
        char line[MAX_LEN_LINE];
        int bg_flag = 0;

        signal(SIGINT, sig_handler);
        
        
        prompt();
        
        int size = getLine_C(line, 1024);

        if(check_bg_cmd(line) == 1) bg_flag = 1;

        if(strncmp( line, "exit", 4) == 0){
            printf("exit\n");
            Free_cmd(job_t);
            exit(0);
        }

        if(strncmp(line, "jobs", 4) == 0) {
            Print_bg_jobs(job_t->next);
            continue;
        }

        int if_dir = Dir_change_handler(line);
        if(if_dir == 1) continue;
        
        signal(SIGINT, sig_handler_child);
        
        pid_t fpid = fork();
        if(fpid > 0){
            if(bg_flag == 1){
                Insert_job_Back(line, fpid, job_t);
            }
            else waitpid(-1, NULL, 0);
        }
        else if(fpid < 0){
            printf("Fork failed!\n");
        }
        else{
            Erase_bg_cmd(line);
            parse(line, size);
            fflush(stdout);
        }
    }
}
