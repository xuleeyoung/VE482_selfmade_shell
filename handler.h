
#ifndef HANDLER_H
#define HANDLER_H

int check_bg_cmd(char *line);
// EFFECTS: return 1 if there exits "&" characters at the end of string "line", 
//          otherwise return 0.

void Erase_bg_cmd(char *line);
// MODIFIES: line
// EFFECTS: Eliminate the appended "&" character if there exists.

void Internal_cmd_handler(char* cmd);
// MODIFIES: output
// EFFECTS: Print the path of the current directory if "cmd" matches "pwd".

int Dir_change_handler(char* cmd);
// MODIFIES: output, current working directory
// EFFECTS: Change the working directory to that specified by argument in "cmd"
//          (If no argumnets, default ~ dir). If dir not found, print error
//          message. Return 1 if dir successfully changed.

void execute(char *cmd_list[]);
// MODIFIES: output
// EFFECTS: Execute commands in the command_list(command name and its arguments).
//          Print out error messages if execution failed.

void Special_character_encoder(char *line);
// MODIFIES: line
// EFFECTS: Restore the original string "line" by substituting SUB with the 
//          characters stored in the "Special_char_list" in order.

void Parse_With_Redi(char *line, int size);
// MODIFIES: line
// EFFECTS: Parse line with redirectors '<''>'
//          Extract redirector with its redirection-file names

void pipe_execute(cmd_t* pipe_list);
// EFFECTS: Execute pipe function in the command line.

void Parse_incompleted_redi();
// EFFECTS: Recursively get and parse the input command line until the whole 
//          command is completed (without '<''>''|' at the end).

void Parse_incompleted_pipe();
// EFFECTS: Recursively get and parse the input command line until the whole 
//          command is completed (without '<''>''|' at the end).

void Special_character_encoder(char *ch);
// MODIFIES: line
// EFFECTS: Encode the original string "line" by substituting special char with 
//          SUB and store them in the "Special_char_list" in order.

void Parse_With_Quotes(char *line);
// MODIFIES: line
// EFFECTS: Parse line with redirectors '\'' '\"'
//          Eliminate them and encode special characters within quotes.

void Parse_With_Pipe(char *line, int size);
// MODIFIES: line
// EFFECTS: Extract each pipe command and store them into linked list "pipeline".

void pipe_error_handler(cmd_t* pipe_list);
// MODIFIES: output
// EFFECTS: Detect and print the error messages possibly appeared with pipe commands.

void parse(char *line, int size);
// EFFECTS: Parse commands
//          Subroutes: Parse_with_quotes->Parse_with_pipe->Parse_with_Redi.


#endif // HANDLER_H
