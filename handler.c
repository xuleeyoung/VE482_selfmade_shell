
#include "io.h"
#include "handler.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

cmd_t* pipeline = NULL; // Linked list for storing piped commands

/* 
 * Temporarily store special char (<|>\n'" ) 
 */
char Special_char_List[1024]; // A char array for storing special characters

int num_special = 0;  // Number of special characters in the list


/* Check wether there is command executed in background
 *
 * find "&" in string line
 */
int check_bg_cmd(char *line)
{
    int size = (int)strlen(line);
    int i = size;
    
    if(size>=1){
    while(line[i]==' ' || line[i] == '\0') i--;
    if(line[i]=='&') {
        return 1;
    }
    else return 0;
    }
    return 0;
}


 /* 
  * Erase the background indicator (&)
  */
void Erase_bg_cmd(char *line)
{
    int size = (int)strlen(line);
    int i = size;
    
    if(size>=1){
    while(line[i]==' ' || line[i] == '\0') i--;
    if(line[i]=='&') {
        line[i] = '\0';
    }
    }
}


/* Execute internal command "pwd"
 *
 * Call "getcwd"
 */
void Internal_cmd_handler(char* cmd)
{
    if(cmd != NULL){
        if(strncmp( cmd, "pwd", 3)==0){
        char* path = NULL;
        path = getcwd(path, 1024);
        printf("%s\n", path);
        fflush(stdout);
        free(path);
        exit(0);
    }
    else return;
    }
}


/* Execute unit command with arguments
 *
 * command with its arguments stored in cmd_list[], Call "execvp"
 */
void execute(char *cmd_list[])
{
    Internal_cmd_handler(cmd_list[0]);   // Handle built-in command first ("pwd")
    int status = execvp(cmd_list[0], cmd_list);
    fflush(stdout);
    if(status == -1){
        printf("%s: command not found\n", cmd_list[0]);
        fflush(stdout);
        exit(-1);
    }
}


/* Execute internal command "cd"
 *
 * Call "chdir"
 */
int Dir_change_handler(char* cmd)
{
    while(*cmd == ' ') cmd++;
    
    if(strcmp(cmd, "cd") == 0){
        // Without argm: HOME directory
        char* path = getenv("HOME");
        chdir(path);
        return 1;
    }
    else if(strncmp(cmd, "cd ", 3) == 0){
        char* token = strtok(cmd, " ");
        token = strtok(NULL, " ");
        int status = chdir(token);
        if(status != 0){
            printf("%s: No such file or directory\n", token);
            fflush(stdout);
        }
        return 1;
    }
    else return 0;
}


/* Restore the encoded command line with special characters
 * 
 * The special characters are stored in Special_char_list before
 */
void Special_character_decoder(char *line)
{
    int i = 0, j = 0;
    while(line[i] != '\0'){
        if(line[i] == 26) line[i] = Special_char_List[j++];
        i++;
    }
}


/* Parse command line with redirectors
 *
 * Extract redirector with its redirection-file names
 * 
 * Call "dup2" "open"
 */
void Parse_With_Redi(char *line, int size)
{
    char* token;
    char* app = " ";
    char* cmd_list[size];
    int num_cmd = 0;

    char* redi_in = NULL, *redi_out = NULL, *redi_ap = NULL;

    // Match redirector positions:
    redi_in = strchr(line, '<');
    redi_out = strchr(line, '>');

    // Check and handle error: duplicated redirection
    if(redi_in != NULL){
        char* tmp1 = redi_in;
        tmp1++;
        while(*(tmp1)==' ') tmp1++;
        if(*tmp1 == '<') output_syntax_handler('<');
        if(*tmp1 == '>') output_syntax_handler('>');
        if(strchr(tmp1, '<') != NULL){
            printf("error: duplicated input redirection\n");
            fflush(stdout);
            exit(-1);
        }
    }
    if(redi_out != NULL){
        if(*(redi_out+1) == '>'){
            redi_ap = redi_out+1;
            redi_out = NULL;
        }
    }
    if(redi_out != NULL){
        char* tmp2 = redi_out;
        tmp2++;
        while(*(tmp2)==' ') tmp2++;
        if(*tmp2 == '<') output_syntax_handler('<');
        if(*tmp2 == '>') output_syntax_handler('>');
        if(strchr(tmp2, '>') != NULL){
            printf("error: duplicated output redirection\n");
            fflush(stdout);
            exit(-1);
        }
    }
    if(redi_ap != NULL){
        char* tmp3 = redi_ap;
        tmp3++;
        while(*(tmp3)==' ') tmp3++;
        if(*tmp3 == '<') output_syntax_handler('<');
        if(*tmp3 == '>') output_syntax_handler('>');
        if(strchr(tmp3, '>') != NULL){
            printf("error: duplicated output redirection\n");
            fflush(stdout);
            exit(-1);
        }
    }

    Special_character_decoder(line);   // Restore the encoded command line with special characters
    
    // Extract redirection-file names, erase redirection arguments by spacing 
    unsigned int i = 0, j = 0;
    char input_filename[size], output_filename[size], append_filename[size];
    
    if(redi_in != NULL){
        i = j = 0;
        *redi_in = ' ';
        while(*(redi_in + i) == ' ') i++;
        j=i;
        while(*(redi_in + i) != ' ' && *(redi_in + i) != '\0' && *(redi_in + i) != '<' && *(redi_in + i) != '>') i++;
        strncpy(input_filename, redi_in+j, i-j);
        input_filename[i-j]='\0';
        for(i = j; i<strlen(input_filename)+j; i++) *(redi_in + i) = ' ';
    }
    if(redi_out != NULL){
        i = j = 0;
        *redi_out = ' ';
        while(*(redi_out + i) == ' ') i++;
        j=i;
        while(*(redi_out + i) != ' ' && *(redi_out + i) != '\0' && *(redi_out + i) != '<' && *(redi_out + i) != '>') i++;
        strncpy(output_filename, redi_out+j, i-j);
        output_filename[i-j]='\0';
        for(i = j; i<strlen(output_filename)+j; i++) *(redi_out + i) = ' ';
    }
    if(redi_ap != NULL){
        i = j = 0;
        *(redi_ap-1) = ' ';
        *redi_ap = ' ';
        *(redi_ap+1) = ' ';
        while(*(redi_ap + i) == ' ') i++;
        j=i;
        while(*(redi_ap + i) != ' ' && *(redi_ap + i) != '\0' && *(redi_ap + i) != '<' && *(redi_ap + i) != '>') i++;
        strncpy(append_filename, redi_ap+j, i-j);
        append_filename[i-j]='\0';
        for(i = j; i<strlen(append_filename)+j; i++) *(redi_ap + i) = ' ';
    }
    
    // parse the input string and extract the information(arguments)
    token = strtok(line, app);
    while(token != NULL){
        cmd_list[num_cmd++] = token;
        token = strtok(NULL, app);
    }
    cmd_list[num_cmd]=NULL;
    // Check and handle error: missing program
    if(cmd_list[0]==NULL){
        printf("error: missing program\n");
        fflush(stdout);
        exit(-1);
    }
    
    // Handle the redirection command, file descriptor
    if(redi_in != NULL){
        int rd_file = open(input_filename, O_RDONLY);
        if(rd_file == -1){
            printf("%s: No such file or directory\n", input_filename);
            fflush(stdout);
            exit(-1);
        }
        int fd = dup2(rd_file, fileno(stdin));
        if(fd == -1){
            printf("dup2 Error!\n");
            exit(-1);
        }
        close(rd_file);
    }
    if(redi_out != NULL){
        int wr_file = open(output_filename, O_RDWR|O_TRUNC|O_CREAT, 0777);
        if(wr_file == -1){
            printf("%s: Permission denied\n", output_filename);
            fflush(stdout);
            exit(-1);
        }
        int fd = dup2(wr_file, fileno(stdout));
        if(fd == -1){
            printf("dup2 Error!\n");
            exit(-1);
        }
        close(wr_file);
    }
    if(redi_ap != NULL){
        int ap_file = open(append_filename, O_RDWR|O_CREAT|O_APPEND, 0777);
        if(ap_file == -1){
            printf("%s: Permission denied\n", append_filename);
            fflush(stdout);
            exit(-1);
        }
        int fd = dup2(ap_file, fileno(stdout));
        if(fd == -1){
            printf("dup2 Error!\n");
            exit(-1);
        }
        close(ap_file);
    }
    execute(cmd_list); // brach to execution of commands
}


/* Execute pipe function in the command line
 *
 * Redirect stdout of the previous command to stdin of the next command
 * 
 * Call "pipe"
 */
void pipe_execute(cmd_t* pipe_list)
{
    if(pipe_list!=NULL && pipe_list->next == NULL){
        Parse_With_Redi(pipe_list->str, pipe_list->len);
    }
    else{
        int fds[2];   // File descriptor
    pid_t pid;
    int num = 0;
    while(pipe_list!= NULL){
        int fd[2];
        if(pipe_list->next!=NULL) {
            pipe(fd);       // Create a pipe between adjacent commands
            fds[1] = fd[1];
        }
        else fds[1] = fileno(stdout);
        if((pid=fork())<0){
            printf("Fork Error!\n");
            exit(-1);
        }
        else if(pid == 0){
            // Reirect the input to the output of the previous command
            if(pipe_list->next!=NULL) close(fd[0]);
            if(fds[0] != fileno(stdin)){
                dup2(fds[0], fileno(stdin));
                close(fds[0]);
            }
            // Reirect the output to the input of the next command.
            if(fds[1] != fileno(stdout)){
                dup2(fds[1], fileno(stdout));
                close(fds[1]);
            }
            Parse_With_Redi(pipe_list->str, pipe_list->len);
        }
        if(fds[0]!=fileno(stdin)) close(fds[0]);
        if(fds[1]!=fileno(stdout)) close(fds[1]);
        fds[0] = fd[0];
        pipe_list = pipe_list->next;
        num++;
    }
    close(fileno(stdout));
    close(fileno(stdin));
    for(int i = 0; i < num; ++i) wait(NULL);
    exit(0);
    }
}


/* Parse incompleted redirection input command line
 * 
 * Complement command line if ">" "<" at the end
 */
void Parse_incompleted_redi()
{
    cmd_t* tmp = cmd_constructor();
    char* app = "|";
    char line[1024];
    int flg_pipe = 0, flg_redi = 0;

    printf("> ");
    fflush(stdout);
    int size = getLine_C(line, 1024);
    Parse_With_Quotes(line);
    size = (int)strlen(line);
    int i = size, delet = 0;

    if(size >= 1){
    while(line[i]==' ' || line[i] == '\0') i--;
        if(line[i]=='|') flg_pipe = 1; // Incompleted pipe command
        if(line[i]=='<' || line[i]=='>') flg_redi = 1; // Incompleted redi command
        if(line[size-1]!='|') delet = 1;
    }
    
    // parse the input string and divide commands by '|'
    char* token = strtok(line, app);
    cmd_t *temp = pipeline->next;
    while(temp->next!=NULL) temp = temp->next;
    strcat(temp->str, token);
    temp->len+=strlen(token);
    token = strtok(NULL, app);
    while(token != NULL){
        Insert_Cmd_Back(token, tmp);
        token = strtok(NULL, app);
    }

    if(flg_pipe == 1){
        if(delet == 1) Remove_Cmd_Back(tmp);
    }
    Concantenate_Cmd(pipeline, tmp);  // Concatenate pipe commands to "pipeline"

    if(flg_pipe == 1) Parse_incompleted_pipe();  // Recuisively complement command line until command are completed
    if(flg_redi == 1) Parse_incompleted_redi();
    Free_cmd(tmp);
}

void Parse_incompleted_pipe()
{
    cmd_t* tmp = cmd_constructor();
    char* app = "|";
    char line[1024];
    int flg_pipe = 0, flg_redi = 0;

    printf("> ");
    fflush(stdout);
    int size = getLine_C(line, 1024);
    Parse_With_Quotes(line);
    size = (int)strlen(line);
    int i = size, delet = 0;


    if(size >= 1){
    while(line[i]==' ' || line[i] == '\0') i--;
        if(line[i]=='|') flg_pipe = 1; // Incompleted pipe command
        if(line[i]=='<' || line[i]=='>') flg_redi = 1; // Incompleted redi command
        if(line[size-1]!='|') delet = 1;
    }
    
    // parse the input string and divide commands by '|'
    char* token = strtok(line, app);
    while(token != NULL){
        Insert_Cmd_Back(token, tmp);
        token = strtok(NULL, app);
    }

    if(flg_pipe == 1){
        if(delet == 1) Remove_Cmd_Back(tmp);
    }
    Concantenate_Cmd(pipeline, tmp);  // Concatenate pipe commands to "pipeline"
    // Recuisively complement command line until command are completed
    if(flg_pipe == 1) Parse_incompleted_pipe();
    if(flg_redi == 1) Parse_incompleted_redi();
    Free_cmd(tmp);
}


/* Parse command line with pipe operator
 *
 * Extract each pipe command and store them into linked list "pipeline"
 * 
 */
void Parse_With_Pipe(char *line, int size)
{
    pipeline = cmd_constructor();
    char* app = "|";
    size = (int)strlen(line);
    int flg_pipe = 0, flg_redi = 0, i = size, delet = 0;

    if(size>=1){
    while(line[i]==' ' || line[i]=='\0') i--;
        if(line[i]=='|') flg_pipe = 1; // Incompleted pipe command
        if(line[i]=='>' || line[i]=='<') {
            flg_redi = 1; // Incompleted redirection command
            if(i>=1){
                char ch = line[i];
                i--;
                while(line[i]==' ') i--;
                if(line[i]=='>' || line[i]=='<') output_syntax_handler(ch);
            }
        }
        if(line[size-1] != '|') delet = 1;
    }
    
    // parse the input string and divide commands by '|'
    char* token = strtok(line, app);
    while(token != NULL){
        Insert_Cmd_Back(token, pipeline);
        token = strtok(NULL, app);
    }

    // Recuisively complement incompleted command line until command are completed
    if(flg_pipe == 1) {
        if(delet == 1) Remove_Cmd_Back(pipeline);
        Parse_incompleted_pipe();
    }

    if(flg_redi == 1) {
        Parse_incompleted_redi();
    }

    // Check "missing program" error
    cmd_t* tmp = pipeline->next;
    char* flag;
    while(tmp!=NULL){
        flag = tmp->str;
        while(*flag==' ') flag++;
        if(*flag == '\0'){
                printf("error: missing program\n");
                fflush(stdout);
                exit(-1);
            }
        tmp = tmp->next;
    }

    // Handle errors in pipe commands
    if(pipeline !=NULL && pipeline->next !=NULL &&pipeline->next->next!=NULL){
        pipe_error_handler(pipeline->next);
    }
    pipe_execute(pipeline->next);
    Free_cmd(pipeline);
    size = 0;
}


/* Handle the error in pipe commands
 *
 * duplicated output, input
 * 
 */
void pipe_error_handler(cmd_t* pipe_list)
{
    char* tmp;
    if((tmp = strchr(pipe_list->str, '>'))!=NULL){
        tmp++;
        while(*tmp==' ') tmp++;
        if(*tmp == '\0') output_syntax_handler('|');
        else{
            printf("error: duplicated output redirection\n");
            fflush(stdout);
            exit(-1);
        }
        }
        while(pipe_list->next->next!=NULL){
            pipe_list = pipe_list->next;
            if(strchr(pipe_list->str, '<')!=NULL){
            printf("error: duplicated input redirection\n");
            fflush(stdout);
            exit(-1);
        }
        if(strchr(pipe_list->str, '>')!=NULL){
            printf("error: duplicated output redirection\n");
            fflush(stdout);
            exit(-1);
        }
        }
        pipe_list = pipe_list->next;
        if(strchr(pipe_list->str, '<')!=NULL){
            printf("error: duplicated input redirection\n");
            fflush(stdout);
            exit(-1);
        }
}


/* Substitute special characters in quotation marks to escape them
 * 
 * Using an SUB ASCII code to substitute special char
 */
void Special_character_encoder(char *ch)
{
    if(*ch == '>' || *ch == '<' || *ch == '|' || *ch == '\'' || *ch == '\"' || *ch == ' ' || *ch == '\n'){
        Special_char_List[num_special++] = *ch;
        *ch = 26;
    }
}

/* Parse commmands with quotation marks or incompleted quotation marks
 * 
 * Match single or double quote marks and recuisively eliminate them
 */
void Parse_With_Quotes(char *line)
{
    char *doubleq, *singleq, *first = NULL, *second;

    // Detect whether there is quotaion marks
    doubleq = strchr(line, '\"');
    singleq = strchr(line, '\'');

    // If matches, find the position of first half of quotation marks
    if(doubleq == NULL && singleq == NULL) return;
    else if(singleq == NULL) second = strchr(doubleq+1, '\"');
    else if(doubleq == NULL) second = strchr(singleq+1, '\'');
    else{
        if(doubleq > singleq) second = strchr(singleq+1, '\'');
        else second = strchr(doubleq+1, '\"');
    }

    // Find the position of second half of quotation marks
    if(doubleq != NULL && singleq != NULL) first = singleq < doubleq ? singleq : doubleq;
    if(doubleq == NULL || singleq == NULL) first = (singleq == NULL) ? doubleq : singleq;
    
    // Handle incompleted quotation marks
    if(second == NULL){
        printf("> ");
        fflush(stdout);
        char str[1024];
        int size = getLine_C(str, 1024);
        size = 0;
        strcat(line, "\n");
        strcat(line, str);
        Parse_With_Quotes(line);
        return;
    }
    else{
        // Shift to eliminate quotes & encoding the escaped special characters
        while(first != second - 1){
            *(first) = *(first+1);
            Special_character_encoder(first);
            ++first;
        }
        while(*first != '\0'){
            *(first) = *(first+2);
            ++first;
        }
        Parse_With_Quotes(line);
        return;
    }
}


/* Parse command line
 *
 * Parse and find pipe first, then parse the reidrection commands
 * 
 * subroutes: parse pipe, parse redi, error_handler...
 */
void parse(char *line, int size)
{
    Parse_With_Quotes(line);
    Parse_With_Pipe(line, size);
    size = 0;
}
