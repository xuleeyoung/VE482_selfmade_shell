// Handling IO

#include "io.h"
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

int num_jobs = 0;

void sig_handler(int sig)
{
    if(sig == SIGINT){
        printf("\n");
        printf("mumsh $ ");
        fflush(stdout);
    }
}

void sig_handler_child(int sig)
{
    if(sig == SIGINT){
        printf("\n");
        fflush(stdout);
    }
}

void prompt(void)
{
    printf("mumsh $ ");
    fflush(stdout);
}

int getLine_C(char *line, int len)
{
    int i = 0, c = 0;
    if((c = getchar()) == EOF){
        printf("exit\n");
        fflush(stdout);
        exit(0);
    }
    else {
        line[i++] = (char)c;
        if(c=='\n'){
            line[i-1] = '\0';
            return i-1;
        }
    }
    while( i < len && (c=getchar())!=0 )
    {
        if(c==EOF){
            continue;
        }
        else{
        line[i++] = (char)c;
        if(c=='\n'){
            line[i-1] = '\0';
            return i-1;
        }
        }
    }
    return i;
}

cmd_t* cmd_constructor()
{
    cmd_t* new_cmd;
    new_cmd = (cmd_t*)malloc(sizeof(cmd_t));
    new_cmd->str = (char*)malloc(1024*sizeof(char));
    new_cmd->len = 0;
    new_cmd->next = NULL;
    return new_cmd;
}

void Free_cmd(cmd_t* victim)
{
    while(victim!=NULL)
    {
        cmd_t* vctm = victim->next;

        free(victim->str);
        free(victim);
        victim = vctm;
    }
    victim = NULL;
}

void Insert_Cmd_Back(char* op, cmd_t* first)
{
    cmd_t* new_cmd = cmd_constructor();
    cmd_t* tmp = first;

    
    new_cmd->len = (int)strlen(op);
    strcpy(new_cmd->str, op);
    new_cmd->next = NULL;

    while(tmp->next!=NULL){
        tmp = tmp->next;
        }
    tmp->next = new_cmd;
    
}

void Insert_job_Back(char* op, int PID, cmd_t* first)
{
    cmd_t* new_cmd = cmd_constructor();
    cmd_t* tmp = first;

    new_cmd->len = PID;
    strcpy(new_cmd->str, op);
    new_cmd->next = NULL;

    while(tmp->next!=NULL){
        tmp = tmp->next;
        }
    tmp->next = new_cmd;
    printf("[%d] %s\n", ++num_jobs, new_cmd->str);
    
}

void Print_bg_jobs(cmd_t* jobs)
{
    cmd_t* tmp = jobs;
    int num = 0;
    while(tmp!=NULL){
        if (waitpid(tmp->len, NULL, WNOHANG) != 0) {
            printf("[%d] done %s\n", ++num, tmp->str);
        } else {
            printf("[%d] running %s\n", ++num, tmp->str);
        }
        tmp = tmp->next;
    }
}

void Remove_Cmd_Back(cmd_t* first)
{
    cmd_t* tmp = first;

    while(tmp->next!=NULL){

        cmd_t *victim = tmp->next;
        if(victim->next==NULL){
            free(victim->str);
            free(victim);
            tmp->next = NULL;
            break;
        }
        else tmp = victim;
    }
}

void Concantenate_Cmd(cmd_t* cmd1, cmd_t* cmd2)
{
    cmd_t *first = cmd2->next;

    while(first!=NULL){
        Insert_Cmd_Back(first->str, cmd1);
        first = first->next;
    }
}


void output_syntax_handler(char ch)
{
    printf("syntax error near unexpected token `%c'\n", ch);
    fflush(stdout);
    exit(-1);
}
