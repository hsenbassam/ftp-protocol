#include "includes.h"
#include "ftpClientFunctions.h"

/* Declarations and Definitions */

enum BOOLEAN {
	FALSE = 0,
	TRUE = 1
};

typedef enum BOOLEAN boolean;

char *dataPort;
char *bytesRead;
/* Functions */

/* PURPOSE: Check if there are just 2 arguments in the command line.
 */
void checkParameters(int argc)
{
	if (argc != 2)
	{
		fprintf(stderr, "USAGE: ./client <ip_address>\n");
		exit(1);
	}
}

/* PURPOSE: Read a resonse from the server.
 */
void read_response(int controlSocket, char response[MAX_LENGTH])
{
	int n;
	//read response from server
	if ((n = read(controlSocket, response, MAX_LENGTH - 1))) {
		response[n] = '\0';
		fprintf(stderr, "%s", response);
	}
	if (n < 0) {
		printf("READ ERROR !!%s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (n == 0) {
		printf("READ ERROR !!%s.\n", strerror(errno));
		exit(EXIT_SUCCESS);
	}
}

/* PURPOSE: Create The Socket Client.
 */
int CreateSocketClient(char *port, char *ipAdd)
{
	int controlSocket;
	struct sockaddr_in address;
	//create a tcp socket
	if ((controlSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {

		perror("\nError in socket()");
		exit(1);
	}

	//Construct the struct
	memset(&address, 0, sizeof(address));       	/* Clear struct */
	address.sin_family = AF_INET;                  	/* Internet/IP */
	address.sin_addr.s_addr = inet_addr(ipAdd);  	/* IP address */
	address.sin_port = htons(atoi(port));       	/* server port */

	char *ip = inet_ntoa(address.sin_addr);

	printf("\nTrying %s ...\n", ip);
	//Connect to the server
	if (connect(controlSocket, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("\nError in connect()");
		exit(1);
	}
	//printf("220 Connecting To FTP Server [%s]\n\n", ip);
	char response[MAX_LENGTH];
	//Read response from server
	read_response(controlSocket, response);
	return controlSocket;
}

/* PURPOSE: This is to get the filename that is specified in a command.
 */
void get_filename(char command[], char filename[])
{

	char *word;

	//parse
	word = strtok(command, SEPCHARS);
	if (word == NULL) {
		perror("\nError in STOR command");
		exit(1);
	}
	//get the filename
	word = strtok(NULL, SEPCHARS);
	if (word == NULL) {
		strncpy(filename, "", MAX_LENGTH);
	}
	else {
		if (!strncasecmp(command, "RESM", 4)) {
			word = strtok(NULL, SEPCHARS);
		}
		strncpy(filename, word, MAX_LENGTH);
	}
}

/* PURPOSE: This is for checking if a file really is in the client directory.
 *          This is done by actually trying to open the file. If we can't open
 *          the file then we know it doesn't exist.
 */
boolean check_directory(char command[])
{

	int file;
	char filename[MAX_LENGTH];
	boolean has_access = TRUE;

	get_filename(command, filename);

	//read the file
	file = open(filename, O_RDONLY);
	if (file == -1) {
		perror("ERROR> Requested action not taken ");
		close(file);
		has_access = FALSE;

	}

	return has_access;
}

/* PURPOSE: Process the LIST command entered by client. Server will send us a list
 *		    of it's current directory.
 */
void process_list(int sock_data)
{

	//read the data from the server
	char list[MAX_LENGTH];
	int n;
	while ((n = read(sock_data, list, MAX_LENGTH - 1))) {

		list[n] = '\0';
		fprintf(stderr, "%s", list);
	}
}

/* PURPOSE: From the passive mode response Syntax: (a1,a2,a3,a4,p1,p2)
 *          IP address a1.a2.a3.a4, port p1*256+p2.
 */
void process_pasv(char *response)
{

	int port;
	char *temp;
	temp = strrchr(response, ',');      // temp = the last occurrence of ',' in reply
	port = atoi(temp + 1);              // port number receive the last number (p2) sent
	*temp = '\0';
	temp = strrchr(response, ',');      // temp will now point to (p1)
	port += atoi(temp + 1) * 256;       // port_num will now be equal to the sent port number from the server
	dataPort = (char *)malloc(sizeof(char) * MAX_LENGTH);
	sprintf(dataPort, "%d", port);
	size_t len = strlen(dataPort);
}

/* PURPOSE: Process the RETR command entered by client. This will make a new
 *          file and put it in our directory.
 */
void process_retr(char command[], int sock_data)
{

	int file;
	int bytes_received = 0;
	char temp[MAX_LENGTH];
	char filename[MAX_LENGTH];
	int n;

	get_filename(command, filename);
	//Create new file
	if (strncasecmp(command, "RESM", 4)) file = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
	else {
		//Resume retrieving
		setenv("SOMEVAR", filename, 1); // Create environment variable
		system("perl -pi -e 'chomp if eof' $SOMEVAR"); // Pass this variable in this command - Concatenate
		file = open(filename, O_APPEND | O_RDWR);
		unsetenv("SOMEVAR"); // Clear that variable (optional)
	}

	if (file == -1) {
		perror("Error in open file");
	}

	//read data in from server and write these into new file
	n = read(sock_data, temp, MAX_LENGTH);
	while (n > 0) {
		bytes_received += n;
		write(file, temp, n);
		n = read(sock_data, temp, MAX_LENGTH);
	}
	if (n == 0) {
		close(file);
		close(sock_data);
	}
	else if (n < 0) {
		fprintf(stderr, "Error in read()");
	}
}

/* PURPOSE: Process the STOR command entered by the user.
 */
void process_stor(char command[], int sock_data)
{

	int file;
	char filename[MAX_LENGTH];
	char temp[MAX_LENGTH];

	get_filename(command, filename);

	//read the file
	file = open(filename, O_RDONLY);
	if (file == -1) {
		perror("ERROR> Requested action not taken ");
		exit(1);
	}


	//read the file line by line, send each line to the server
	int byte = read(file, temp, MAX_LENGTH);
	while (byte > 0) {

		//fprintf(stderr, "%s", temp);	//if we want to see what we send
		write(sock_data, temp, byte);
		byte = read(file, temp, MAX_LENGTH);
	}

	//the client will close the connection once it has transferred all the data
	//(sent all the lines)
	if (byte == 0) {
		close(file);
		close(sock_data);
		//fprintf(stderr, "\nDone transferring file. Closed data connection.\n\n");

	}
	else if (byte < 0) {
		perror("\nError in read file");
		exit(1);
	}
}

/* PURPOSE: Process the RESM command entered by the user.
 */
char *process_resm(char *filenm) {
	FILE *file;
	char *filename = strtok(filenm, " \r\n");
	file = fopen(filename, "r");
	if ( file == 0 )  {
		perror ("Error opening file");
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	intmax_t len = ftell(file);
	char *r = (char *)malloc(sizeof(char) * MAX_LENGTH);
	sprintf(r, "%jd %s\n", len, filename);
	fclose(file);
	return r;
}

/* PURPOSE: Creates a data connection whenever a LIST, RETR or STOR command
 *		    is entered by client.
 */
void create_data_connect(char command[], char ip[])
{

	int sock_data, rv;
	struct addrinfo hints, *servinfo, *p;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(ip, dataPort, &hints, &servinfo)) != 0) {
		printf("error getaddrinfo\n");
		exit(0);
	}


	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sock_data = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			printf("socket\n");
			continue;
		}

		if (connect(sock_data, p->ai_addr, p->ai_addrlen) == -1) {
			close(sock_data);
			perror("Cant Connect");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "Talker: failed to create socket\n");
		exit(0);
	}

	//if we issued a STOR command
	if (strncasecmp("STOR", command, 4) == 0) {

		process_stor(command, sock_data);

	}

	else if ((strncasecmp("RETR", command, 4) == 0) || (strncasecmp("RESM", command, 4) == 0) ) {

		process_retr(command, sock_data);
	}

	//we issued a LIST command
	else if (strncasecmp("LIST", command, 4) == 0) {

		process_list(sock_data);
	}

	else
		fprintf(stderr, "\nNot a valid command.\n\n");

}
