
#include "includes.h"
#include "users.h"
#include "ftpServerFunctions.h"

enum states {STATE_NOT_CONNECTED, STATE_CONNECTED, STATE_WAITING_FOR_PASS, STATE_LOGGED_IN}; /* Status of The User */

int dataSocket = 0; /* For accepting Data Connections */

char *LoggedUsername = NULL; /* Username of The Logged in User*/

/* Functions */

/* PURPOSE: Check if there are just 1 arguments in the command line.
 */
void checkParameters(int argc) {
    if (argc != 1)
    {
        fprintf(stderr, "Usage: ./server --Without Parameters\n");
        exit(EXIT_FAILURE);
    }
}

/* PURPOSE: Create the Socket Server.
 */
int createSocketServer(int portNo) {
    int listenfd;
    struct sockaddr_in servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == 1)
    {
        perror("Error calling socket().");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(portNo);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Error calling bind() ");
        return 0;
    }

    if (listen(listenfd, SERV_BACKLOG) < 0)
    {
        perror("Error calling listen().");
        return 0;
    }

    return listenfd;
}


/* PURPOSE: Open Data connection for a Client (Accept).
 */
int open_data_connection() {
    int  datafd;
    if ((datafd = accept(dataSocket, (struct sockaddr*) NULL, NULL)) < 0)
    {
        perror("Error calling accept().");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Data connection established.\n\n");
    return datafd;
}

/* PURPOSE: Write a Message To a Client.
 */
void write_message(int controlSocket, char *strMsg) {
    if ( write(controlSocket, strMsg, strlen(strMsg)) != strlen(strMsg))
    {
        fprintf(stderr, "Error writing to client. Server exiting.\n");
        exit(EXIT_FAILURE);
    }
}

/* PURPOSE: The following functions print the responses.
 */
void print_not_logged_in(int controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "530 User not logged in.\r\n");
    write_message(controlSocket, tempStr);
}

void print_opening_ASCII_mode(int controlSocket, char *file) {
    char tempStr[MAX_SIZE];
    snprintf(tempStr, MAX_SIZE - 1, "150 Opening ASCII mode data connection for : %s \r\n", file);
    write_message(controlSocket, tempStr);
}

void print_transfer_complete(int controlSocket, int numBytes) {
    char tempStr[MAX_SIZE];
    snprintf(tempStr, MAX_SIZE - 1, "226 Transfer complete (%d) Bytes, Closed data connection.\r\n", numBytes);
    write_message(controlSocket, tempStr);
}

void print_help(int controlSocket, char *cmd) {

    if (!strcmp(cmd, ""))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214 The Commands Are:\n"
               "USER\tPASS\tCWD\tCDUP\tQUIT\tPASV\tRETR\tTYPE\nFEAT\tSYST\tSTOR\tRESM\tMKD\tPWD\tLIST\tHELP\tNOOP\n"
               "200 Command okay.\r\n");
        write_message(controlSocket, tempStr);
        return;
    }
    if      (!strcasecmp(cmd, "USER"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: USER <SP> Username <CRLF>.\n"
               "214-Send this command to begin the login process.\n"
               "214-username should be a valid username on the system\n"
               "214 or anonymous to initiate an anonymous login.\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "PASS"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: PASS <SP> Password <CRLF>.\n"
               "214-After sending the USER command\n"
               "214 Send this command to complete the login process.\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "CWD"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: CWD <SP> Path <CRLF>.\n"
               "214 Makes the given directory be the current directory on the remote host.\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "CDUP"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: CDUP <CRLF>.\n"
               "214 Makes the parent of the current directory be the current directory.\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "QUIT"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: QUIT <CRLF>.\n"
               "214 Terminates the command connection.\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "SYST"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: SYST <CRLF>.\n"
               "214 Return system type .\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "TYPE"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: TYPE <SP> Type-Character <CRLF>.\n"
               "214 Set transfer type\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "FEAT"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: FEAT <CRLF>.\n"
               "214 Get the feature list implemented by the server.\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "PASV"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: PASV <CRLF>.\n"
               "214-Tells the server to enter passive mode.\n"
               "214-The server will wait for the client to establish a connection with it\n"
               "214-Rather than attempting to connect to a client-specified port.\n"
               "214-The server will respond with the address of the port it is listening on\n"
               "214-With a message like: 227 Entering Passive Mode (a1,a2,a3,a4,p1,p2)\n"
               "214 where a1.a2.a3.a4 is the IP address and p1*256+p2 is the port number.\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "RETR"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: RETR <SP> File-name <CRLF>.\n"
               "214 Begins transmission of a file from the remote host\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "RESM"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: RESM <SP> File-name <CRLF>.\n"
               "214 Resumes transmission of a file from the remote host\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "STOR"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: STOR <SP> File-name <CRLF>.\n"
               "214 Begins transmission of a file to the remote site\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "MKD"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: MKD <SP> Directory-name <CRLF>.\n"
               "214 Creates the named directory on the remote host.\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "PWD"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: PWD <CRLF>.\n"
               "214 Returns the name of the current directory on the remote host.\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "LIST"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: LIST <CRLF>.\n"
               "214 Sends information about each file in the current directory\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "HELP"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: HELP [<SP> Command] <CRLF>.\n"
               "214-If a command is given, returns help on that command\n"
               "214 Otherwise, returns general help for the FTP server\r\n");
        write_message(controlSocket, tempStr);
    }
    else if (!strcasecmp(cmd, "NOOP"))
    {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "214-Syntax: NOOP <CRLF>.\n"
               "214 Does nothing except return a response\r\n");
        write_message(controlSocket, tempStr);
    }
    else
    {
        char tempStr[MAX_SIZE];
        snprintf(tempStr, MAX_SIZE - 1, "502 Unknown command %s.\r\n", cmd);
        write_message(controlSocket, tempStr);
    }
}

void print_failed_to_open_file(controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "550 Failed to open file.\r\n");
    write_message(controlSocket, tempStr);
}

void print_file_unavailable(controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "550 File / Dir unavailable, not found, not accessible.\r\n");
    write_message(controlSocket, tempStr);
}

void print_command_not_okay_permission(int controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "550 Permission Denied / Requested action not taken.\r\n");
    write_message(controlSocket, tempStr);
}

void print_cant_open_data_cnx(int controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "425 Cannot open data connection.\r\n");
    write_message(controlSocket, tempStr);
}

void print_command_okay(int controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "200 Command okay.\r\n");
    write_message(controlSocket, tempStr);
}

void print_command_successful(int controlSocket, char *cmd) {
    char tempStr[MAX_SIZE];
    snprintf(tempStr, MAX_SIZE - 1, "250 %s Command successful.\r\n", cmd);
    write_message(controlSocket, tempStr);
}

void print_connection_established(int controlSocket) {
    char tempStr[MAX_SIZE];
    snprintf(tempStr, MAX_SIZE - 1, "220 Connecting To FTP Server\r\n");
    write_message(controlSocket, tempStr);
}

void print_command_not_implemented_superfluous(int controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "202 Command not implemented, superfluous at this site.\r\n");
    write_message(controlSocket, tempStr);
}

void print_no_files(int controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "550 No files found.\r\n");
    write_message(controlSocket, tempStr);
}

void print_unknown_command(int controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "502 Command not implemented.\r\n");
    write_message(controlSocket, tempStr);
}

void print_no_command(controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "502 No command given.\r\n");
    write_message(controlSocket, tempStr);
}

void print_switch_pasv(int controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "500 Syntax error, command unrecognized,Try switching to passive mode.\r\n");
    write_message(controlSocket, tempStr);
}


/* PURPOSE: The following functions detail each command what to do , cmd_xxxx.
 */


void cmd_user(char *username, enum states *ptrState, int controlSocket) {
    char tempStr[MAX_SIZE];

    if (checkUser(username, &LoggedUsername))
    {
        strcpy(tempStr, "331 User name okay, need a password.\r\n");
        write_message(controlSocket, tempStr);
        *ptrState = STATE_WAITING_FOR_PASS;
    }
    else
    {
        if (!strcmp(username, "anonymous")) {
            strcpy(tempStr, "230 Anonymous access granted, no need a password.\r\n");
            write_message(controlSocket, tempStr);
            *ptrState = STATE_LOGGED_IN;
        }
        else {
            print_command_not_okay_permission(controlSocket);
        }
    }
}

void cmd_pass(char *password, enum states *ptrState, int controlSocket) {
    char tempStr[MAX_SIZE];

    if (*ptrState == STATE_WAITING_FOR_PASS)
    {
        if (checkPass(LoggedUsername, password))
        {
            strcpy(tempStr, "230 User access granted, restrictions apply.\r\n");
            write_message(controlSocket, tempStr);
            *ptrState = STATE_LOGGED_IN;
        }
        else
        {
            strcpy(tempStr, "530 Incorrect password.\r\n");
            write_message(controlSocket, tempStr);
        }

    }
    else if (*ptrState == STATE_CONNECTED)
    {
        print_not_logged_in(controlSocket);
    }
}


void cmd_pasv(int controlSocket) {
    /* Tells the server to enter "passive mode". In passive mode */
    struct sockaddr_in servaddr;
    char tempStr[MAX_SIZE];
    unsigned int ip;
    unsigned short port;
    uint16_t portNo;
    int len;

    // Data Socket
    dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSocket == 1)
    {
        perror("Error calling socket().");
        exit(EXIT_FAILURE);
    }


    srand(time(NULL));
    portNo = (uint16_t)(rand() % (65530 - 1024) + 1024);

    memset(&servaddr, 0, sizeof(servaddr));
    len = sizeof(servaddr);

    getsockname(controlSocket, (struct sockaddr*)&servaddr, &len);
    servaddr.sin_port = htons(portNo);;

    if (bind(dataSocket, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("bind() for passive socket FAILED: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    getsockname(dataSocket, (struct sockaddr*)&servaddr, &len);

    if (listen(dataSocket, SERV_BACKLOG) < 0)
    {
        perror("Error calling listen().");
        exit(EXIT_FAILURE);
    }
    ip = ntohl(servaddr.sin_addr.s_addr);
    port = ntohs(servaddr.sin_port);

    /* Syntax: PORT a1,a2,a3,a4,p1,p2
     * Specifies the host and port to which the server should connect for the next file transfer.
     * This is interpreted as IP address a1.a2.a3.a4, port p1*256+p2.
     */
    snprintf(tempStr, MAX_SIZE - 1, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n",
             (unsigned int)((ip >> 24) & 0xff),
             (unsigned int)((ip >> 16) & 0xff),
             (unsigned int)((ip >> 8) & 0xff),
             (unsigned int)(ip & 0xff),
             (unsigned int)(port >> 8),
             (unsigned int)(port & 0xff));


    printf("Opened passive connection : %s:%d\n", inet_ntoa(servaddr.sin_addr), port);

    write_message(controlSocket, tempStr);
}

void cmd_pwd(int controlSocket) {
    long size;
    char *buf, *ptr, tempStr[MAX_SIZE];
    size = pathconf(".", _PC_PATH_MAX);
    if ((buf = (char *)malloc((size_t)size)) != NULL)
        ptr = getcwd(buf, (size_t)size);

    snprintf(tempStr, MAX_SIZE - 1, "257 \"%s\" is the current directory.\r\n", ptr);
    write_message(controlSocket, tempStr);
}

void cmd_cwd(int controlSocket, char *destination) {
    char cwd[1024];
    if (chdir(destination) == 0) {
        print_command_successful(controlSocket, "CWD");
    }
    else {
        print_file_unavailable(controlSocket);
    }
}

void cmd_cdup(int controlSocket) {
    // The Parent Directory
    char cwd[1024];
    if (chdir("..") == 0) {
        print_command_successful(controlSocket, "CDUP");
    }
    else {
        print_file_unavailable(controlSocket);
    }
}

void cmd_type(int controlSocket, char *type) {
    if (!strncasecmp(type, "I", 1) || !strncasecmp(type, "A", 1)
	||!strncasecmp(type, "ASCII", 5) || !strncasecmp(type, "ASCII non-print", 15))
        print_command_okay(controlSocket);
    else {
        char tempStr[MAX_SIZE];
        strcpy(tempStr, "500 Syntax error.\r\n");
        write_message(controlSocket, tempStr);
    }
}

void cmd_feat(int controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "211 Feature Negotiation.\r\n");
    write_message(controlSocket, tempStr);
}

void cmd_syst(int controlSocket) {
    char tempStr[MAX_SIZE];
    strcpy(tempStr, "215 Linux system.\r\n");
    write_message(controlSocket, tempStr);
}

void cmd_list(int controlSocket) {
    char pathName[MAX_SIZE];
    char tempStr[MAX_SIZE];
    struct stat statBuf;
    struct dirent *dirp;
    DIR *dp;
    int fd;
    intmax_t totalSize = 0;

    if (dataSocket == 0 )
    {
        print_switch_pasv(controlSocket);
        return;

    }
    if (getcwd(pathName, MAX_SIZE - 1) == NULL)
    {
        print_command_not_okay_permission(controlSocket);
        return;
    }
    if ( (dp = opendir(pathName)) == NULL)
    {
        print_command_not_okay_permission(controlSocket);
    }
    else
    {
        print_opening_ASCII_mode(controlSocket, "LIST");
        fd = open_data_connection();
        if (!fd) {
            print_cant_open_data_cnx(controlSocket);
            return;
        }
        while ((dirp = readdir(dp)) != NULL)
        {
            if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
                continue;
            if (lstat(dirp->d_name, &statBuf) == 0)
            {
                if (S_ISREG(statBuf.st_mode) && statBuf.st_size != 0)
                {
                    //snprintf(tempStr, MAX_SIZE - 1, "%s, Size: %jd Bytes\r\n", dirp->d_name, (intmax_t)statBuf.st_size);
                    totalSize += (intmax_t)statBuf.st_size;
                    // write_message(fd, tempStr);
                }

            }
        }

        char control[100];
        sprintf(control, "ls -l -B -h -c %s | tail -n+2 > .tmp.txt", pathName);
        int rs = system(control);
        if (rs < 0) {
            return;
        }
        FILE* f = fopen(".tmp.txt", "r");
        if (!f) {
            printf("Not the File\n");
        }
        char buffer[MAX_SIZE];
        int n;

        do {
            n = fread(buffer, 1, 8190, f);
            send(fd, buffer, n, 0);

        } while (n > 0);

        fclose(f);
        system("rm .tmp.txt");

        closedir(dp);
        close(fd);
        if (totalSize != 0 ) print_transfer_complete(controlSocket, totalSize);
        else print_no_files(controlSocket);

    }
}

void cmd_mkd(int controlSocket, char *dir) {
    int i;
    char dirname[50];
    strcpy(dirname, dir);
    if ((mkdir(dirname, 00777)) == -1) {
        // Equivalent of 00777 = rwxrwxrwx
        print_command_not_okay_permission(controlSocket);
        return;
    }
    else {
        long size;
        char *buf, *ptr, tempStr[MAX_SIZE];
        char path[_PC_PATH_MAX] ;
        snprintf(path, _PC_PATH_MAX , "./%s", dirname);
        size = pathconf(path, _PC_PATH_MAX);
        if ((buf = (char *)malloc((size_t)size)) != NULL)
            ptr = getcwd(buf, (size_t)size);
        snprintf(tempStr, MAX_SIZE - 1, "257 File Created \"%s\".\r\n", ptr);
        write_message(controlSocket, tempStr);
    }
}

void cmd_retr(int controlSocket, char *filename, intmax_t position) {
    int fd;
    char tempStr[MAX_SIZE];
    int n;
    FILE *filefd;
    struct stat sb;

    if (dataSocket == 0 )
    {
        print_switch_pasv(controlSocket);
        return;
    }
    if (stat(filename, &sb) == -1)
    {
        print_file_unavailable(controlSocket);
        return;
    }
    /*
    if (sb.st_mode & S_IFMT != S_IFREG)
    {
        print_command_not_okay_permission(controlSocket);
        return;
    }
    */
    filefd = fopen(filename, "r");
    if (filefd == 0)
    {
        print_failed_to_open_file(controlSocket);
        return;
    }
    else
    {
        print_opening_ASCII_mode(controlSocket, filename);

    }

    fd = open_data_connection();
    if (!fd) {
        print_cant_open_data_cnx(controlSocket);
        return;
    }
    fseek(filefd, position, SEEK_SET);
    while ((n = fread(tempStr, 1, MAX_SIZE, filefd)) > 0)
    {
        write(fd, tempStr, n);
    }

    if (n == 0)
    {
        close(fd);
        fclose(filefd);
        if (!position )print_transfer_complete(controlSocket, sb.st_size);
        else print_transfer_complete(controlSocket, sb.st_size - position - 1);
    }
    else if (n < 0)
    {
        fprintf(stderr, "Error reading file.\n");
        exit(EXIT_FAILURE);
    }
}

void cmd_resm(int controlSocket, char *filedetails) {

    intmax_t j;
    int base = 10;
    char *nptr, *endptr, *bytesRead;
    bytesRead = strtok(filedetails, " ");
    printf("bytesRead -%s-\n", bytesRead);
    char *filename = strtok(NULL, " ");
    printf("File Name -%s-\n", filename);
    j = strtoimax(bytesRead, &endptr, base);
    cmd_retr(controlSocket, filename, j - 1);
}

void cmd_stor(int controlSocket, char *filename) {
    int filefd, fd;
    int bytesReceived = 0;
    char tempStr[MAX_SIZE];
    int n;

    if (dataSocket == 0 )
    {
        print_switch_pasv(controlSocket);
        return;

    }
    filefd = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
    if (filefd == -1)
    {
        print_command_not_okay_permission(controlSocket);
        return;
    }
    else
    {

        print_opening_ASCII_mode(controlSocket, filename);

    }

    fd = open_data_connection();

    if (!fd) {
        print_cant_open_data_cnx(controlSocket);
        return;
    }

    n = read(fd, tempStr, MAX_SIZE);

    while (n > 0)
    {
        bytesReceived += n;

        write(filefd, tempStr, n);
        n = read(fd, tempStr, MAX_SIZE);
    }

    if (n == 0)
    {
        close(fd);
        close(filefd);
        print_transfer_complete(controlSocket, bytesReceived);

    }
    else if (n < 0)
    {
        fprintf(stderr, "Error reading from socket.\n");
        exit(EXIT_FAILURE);
    }
}

void cmd_noop(int controlSocket) {
    /* Does nothing except return a response */
    print_command_okay(controlSocket);
}

void cmd_help(int controlSocket, char *cmd) {
    /* Help Messages */
    print_help(controlSocket, cmd);
}

void cmd_quit(int controlSocket, enum states *ptrState) {
    char tempStr[MAX_SIZE];
    char *name = LoggedUsername;
    if (name == NULL) name = "Anonymous";
    snprintf(tempStr, MAX_SIZE - 1, "221 Goodbye %s\r\n", name);
    write_message(controlSocket, tempStr);
    *ptrState = STATE_NOT_CONNECTED;
}

/* PURPOSE: The Process of The Server.
 */
int doProcessing(int controlSocket, enum states *ptrState) {
    int datafd;
    char cmdstring[MAX_SIZE];
    char *word, *filenm;
    char command[MAX_SIZE];
    char arg[MAX_SIZE];
    int n;

    for ( ; ; )
    {
        /*read command*/
        n = read(controlSocket, cmdstring, MAX_SIZE - 1); /*technically I should read until I see \r\n*/

        if (n == 0)
        {
            fprintf(stderr, "Client disconnected.\n");
            close(controlSocket);
            return 0;

        }
        cmdstring[n] = '\0';
        fprintf(stderr, "Client: %s", cmdstring);

        /*parse*/
        word = strtok(cmdstring, SEPCHARS);
        if (word == NULL)
        {
            fprintf(stderr, "Error, no command given\n");
            print_no_command(controlSocket);
            continue;
        }

        strncpy(command, word, MAX_SIZE);

        int v = 0;
        if (!strncasecmp(command, "RESM", 4)) v = 1;
        word = strtok(NULL, SEPCHARS);
        if (word == NULL)
        {
            strncpy(arg, "", MAX_SIZE);
        }
        else
        {
            strncpy(arg, word, MAX_SIZE);
        }
        if (v) {
            filenm = (char *)malloc(sizeof(char) * MAX_SIZE);
            word = strtok(NULL, SEPCHARS);
            strncpy(filenm , word, MAX_SIZE);
        }
        fprintf(stderr, "command:%s,length:%d\n", command, strlen(command));
        fprintf(stderr, "arg:%s,length:%d\n", arg, strlen(arg));
        if (v) {
            fprintf(stderr, "filename:%s,length:%d\n\n", filenm, strlen(filenm));
            char *arg2 = (char *)malloc(sizeof(char) * MAX_SIZE);
            snprintf(arg2, MAX_SIZE, "%s %s", arg, filenm);
            strcpy(arg, arg2);
        }
        else printf("\n");


        if (strncasecmp("QUIT", command, 4) == 0)
        {
            cmd_quit(controlSocket, ptrState);
            close(controlSocket);
            return 0;
        }

        switch (*ptrState)
        {
        case STATE_CONNECTED :
            if ((strlen(command) == 4) && (strncasecmp("USER", command, 4) == 0)) //command is USER
            {
                cmd_user(arg, ptrState, controlSocket);
            }
            else if ((strlen(command) == 4) && (strncasecmp("PASS", command, 4) == 0)) //command is PASS
            {
                cmd_pass(arg, ptrState, controlSocket);
            }
            else /*no other command is viable*/
            {
                print_not_logged_in(controlSocket);

            }

            break;

        case STATE_WAITING_FOR_PASS:
            if ((strlen(command) == 4) && (strncasecmp("PASS", command, 4) == 0)) //command is PASS
            {
                cmd_pass(arg, ptrState, controlSocket);
            }
            else if ((strlen(command) == 4) && (strncasecmp("USER", command, 4) == 0)) //command is USER
            {
                cmd_user(arg, ptrState, controlSocket);
            }
            else
            {
                print_not_logged_in(controlSocket);
            }

            break;

        case STATE_LOGGED_IN:
            if      ((strlen(command) == 4) && (strncasecmp("LIST", command, 4) == 0) )
            {
                cmd_list(controlSocket);

            }
            else if ((strlen(command) == 3) && (strncasecmp("PWD", command, 3) == 0))
            {
                cmd_pwd(controlSocket);

            }
            else if ((strlen(command) == 4) && (strncasecmp("TYPE", command, 4) == 0))
            {
                cmd_type(controlSocket, arg);

            }
            else if ((strlen(command) == 4) && (strncasecmp("SYST", command, 4) == 0))
            {
                cmd_syst(controlSocket);

            }
            else if ((strlen(command) == 4) && (strncasecmp("FEAT", command, 4) == 0))
            {
                cmd_feat(controlSocket);

            }
            else if ((strlen(command) == 3) && (strncasecmp("MKD", command, 3) == 0))
            {
                cmd_mkd(controlSocket, arg);

            }
            else if ((strlen(command) == 3) && (strncasecmp("CWD", command, 3) == 0))
            {
                cmd_cwd(controlSocket, arg);

            }
            else if ((strlen(command) == 4) && (strncasecmp("RESM", command, 4) == 0))
            {

                cmd_resm(controlSocket, arg);

            }
            else if ((strlen(command) == 4) && (strncasecmp("CDUP", command, 4) == 0))
            {
                cmd_cdup(controlSocket);

            }
            else if ((strlen(command) == 4) && (strncasecmp("RETR", command, 4) == 0))
            {
                cmd_retr(controlSocket, arg , 0);

            }
            else if ((strlen(command) == 4) && strncasecmp("STOR", command, 4) == 0)
            {
                cmd_stor(controlSocket, arg);
            }
            else if ((strlen(command) == 4) && strncasecmp("PASV", command, 4) == 0)
            {
                cmd_pasv(controlSocket);
            }
            else if ((strlen(command) == 4) && strncasecmp("NOOP", command, 4) == 0)
            {
                cmd_noop(controlSocket);
            }
            else if ((strlen(command) == 4) && strncasecmp("HELP", command, 4) == 0)
            {
                cmd_help(controlSocket, arg);
            }
            else if ((strlen(command) == 4) && ((strncasecmp("PASS", command, 4) == 0) ||
                                                (strncasecmp("USER", command, 4) == 0)))
            {
                print_command_not_implemented_superfluous(controlSocket);
            }
            else
            {
                print_unknown_command(controlSocket);

            }

        } /*switch*/

    }
}
