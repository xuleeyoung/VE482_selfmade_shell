
#ifndef IO_H
#define IO_H

/* An ADT cmd_t to store command lines
 *
 * str: command line string
 * len: length of str or ID of the background process
 */
typedef struct cmdLine {
    char* str;
    int   len;
    struct cmdLine* next;
} cmd_t;

void sig_handler(int sig);
// Handle SIGINT signal

void sig_handler_child(int sig);
// Handle SIGINT signal


/* **************************  Linked list operations  **************************** */
cmd_t* cmd_constructor();
// Constructor

void Free_cmd(cmd_t* victim);
// Destructor

void Insert_Cmd_Back(char* op, cmd_t* first);
// Insert element at the back of the list (Used with pipeline)

void Insert_job_Back(char* op, int PID, cmd_t* first);
// Insert element at the back of the list (Used with Jobs_t)

void Print_bg_jobs(cmd_t* jobs);
// Taverse the whole list and print all background commands stored.

void Remove_Cmd_Back(cmd_t* first);
// Remove elements at the back of the list

void Concantenate_Cmd(cmd_t* cmd1, cmd_t* cmd2);
// Concatenate two list


/* **************************  Shell I/O  **************************** */
void prompt(void);
// Print prompt

int getLine_C(char *line, int len);
// C-type "getline"

void output_syntax_handler(char ch);
// Syntax error handler

#endif // IO_H
