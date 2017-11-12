

extern enum states state ; /* Status of The User */

extern int dataSocket; /* For accepting Data Connections */

extern char *LoggedUsername; /* Username of The Logged in User*/

/* Functions */

/* PURPOSE: Check if there are just 1 arguments in the command line.
 */
void checkParameters(int argc);

/* PURPOSE: Create the Socket Server.
 */
int createSocketServer(int portNo);

/* PURPOSE: Open Data connection for a Client (Accept).
 */
int open_data_connection();

/* PURPOSE: Write a Message To a Client.
 */
void write_message(int controlSocket, char *strMsg);

/* PURPOSE: The following functions print the responses.
 */
void print_not_logged_in(int controlSocket);
void print_opening_ASCII_mode(int controlSocket, char *file);
void print_transfer_complete(int controlSocket, int numBytes);
void print_help(int controlSocket, char *cmd);
void print_failed_to_open_file(int controlSocket);
void print_file_unavailable(int controlSocket);
void print_command_not_okay_permission(int controlSocket);
void print_cant_open_data_cnx(int controlSocket);
void print_command_okay(int controlSocket);
void print_command_successful(int controlSocket, char *cmd);
void print_connection_established(int controlSocket);
void print_command_not_implemented_superfluous(int controlSocket);
void print_no_files(int controlSocket);
void print_unknown_command(int controlSocket);
void print_no_command(int controlSocket);
void print_switch_pasv(int controlSocket);


/* PURPOSE: The following functions detail each command what to do , cmd_xxxx.
 */


void cmd_user(char *username, enum states *ptrState, int controlSocket);
void cmd_pass(char *password, enum states *ptrState, int controlSocket);
void cmd_pasv(int controlSocket);
void cmd_pwd(int controlSocket);
void cmd_cwd(int controlSocket, char *destination);
void cmd_cdup(int controlSocket);
void cmd_type(int controlSocket, char *type);
void cmd_feat(int controlSocket);
void cmd_syst(int controlSocket);
void cmd_list(int controlSocket);
void cmd_mkd(int controlSocket, char *dir);
void cmd_retr(int controlSocket, char *filename, intmax_t position);
void cmd_resm(int controlSocket, char *filedetails);
void cmd_stor(int controlSocket, char *filename);
void cmd_noop(int controlSocket);
void cmd_help(int controlSocket, char *cmd);
void cmd_quit(int controlSocket, enum states *ptrState);

int doProcessing(int controlSocket, enum states *ptrState);
