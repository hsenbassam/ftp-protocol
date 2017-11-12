extern enum BOOLEAN bool;

typedef enum BOOLEAN boolean;
/* PURPOSE: Check if there are just 2 arguments in the command line.
 */
void checkParameters(int argc);

/* PURPOSE: Read a resonse from the server.
 */
void read_response(int controlSocket, char response[MAX_LENGTH]);

/* PURPOSE: Create The Socket Client.
 */
int CreateSocketClient(char *port, char *ipAdd);

/* PURPOSE: This is to get the filename that is specified in a command.
 */
void get_filename(char command[], char filename[]);

/* PURPOSE: This is for checking if a file really is in the client directory.
 *          This is done by actually trying to open the file. If we can't open
 *          the file then we know it doesn't exist.
 */
boolean check_directory(char command[]);

/* PURPOSE: Process the LIST command entered by client. Server will send us a list
 *		    of it's current directory.
 */
void process_list(int sock_data);

/* PURPOSE: From the passive mode response Syntax: (a1,a2,a3,a4,p1,p2)
 *          IP address a1.a2.a3.a4, port p1*256+p2.
 */
void process_pasv(char *response);

/* PURPOSE: Process the RETR command entered by client. This will make a new
 *          file and put it in our directory.
 */
void process_retr(char command[], int sock_data);

/* PURPOSE: Process the STOR command entered by the user.
 */
void process_stor(char command[], int sock_data);

/* PURPOSE: Process the RESM command entered by the user.
 */
char *process_resm(char *filenm);

/* PURPOSE: Creates a data connection whenever a LIST, RETR or STOR command
 *		    is entered by client.
 */
void create_data_connect(char command[], char ip[]);


