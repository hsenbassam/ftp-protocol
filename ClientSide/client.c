/*
* Jan 10, 2017  is The Last Update
* client.c
*
* PURPOSE: An ftp client.
* INSTRUCTIONS:
*
*		There will be 2 arguments in the command line
*		./client <ip_address>
*
*		Choose ip_address to be The Ip Address Of The Server
*
*/

#include "includes.h"
#include "ftpClientFunctions.h"

/* Declarations and Definitions */

enum BOOLEAN {
	FALSE = 0,
	TRUE = 1
};

typedef enum BOOLEAN boolean;


/* The Main Process */
int main(int argc, char *argv[])
{

	int controlSocket;
	char *ip = argv[1];
	boolean has_quit = 0;


	checkParameters(argc);

	controlSocket = CreateSocketClient(PORT, ip);

	//Keep getting input from client until they enter QUIT command
	//These inputs from client will be sent as commands to server
	while (has_quit == FALSE) {

		char command[MAX_LENGTH];
		char command_temp[MAX_LENGTH];
		printf("ftp> ");
		fgets(command, MAX_LENGTH, stdin);	//get input from user
		strcat(command, "\r\n");
		strcpy(command_temp, command);
		boolean has_access = TRUE;

		/* if doing a STOR command, check if the file in your directory does in fact exist
		 * if the file does not exists on our (client) directory, do not bother sending
		 * this STOR command to the server
		 * (since on a STOR, the server actually makes an empty file in its directory assuming that
		 * it is going to get to STOR in it)
		*/

		if (strncmp("STOR", command, 4) == 0) {
			has_access = check_directory(command_temp);
		}


		if (has_access == TRUE) {

			if (!strncasecmp("RESM", command, 4)) {
				char *filenm = strtok(command, " ");
				filenm = strtok(NULL, " ");
				char *filedetails = process_resm(filenm);
				if (filedetails) sprintf(command, "RESM %s\r\n", filedetails);
				else continue;
			}

			//Send command to server
			if (write(controlSocket, command, strlen(command)) < 0) {
				perror("\nError in write()");
				exit(1);
			}


			char response[MAX_LENGTH];
			//Read response from server
			read_response(controlSocket, response);

			//If user receive a accepted response from Passive mode
			if (strncmp("227", response, 3) == 0) process_pasv(response);

			//If user enters RETR, LIST or STOR
			//will not make a data connection if a response is anything other
			//than 150 Command okay
			if (!strncmp("150", response, 3) && (!strncasecmp("LIST", command, 4) || !strncasecmp("STOR", command, 4)
			                                     || !strncasecmp("RETR", command, 4) || !strncasecmp("RESM", command, 4) )) {

				//If the command is ok, the client will connect to the server to form a tcp data connection
				create_data_connect(command, ip);
				//Read response from server
				read_response(controlSocket, response);


			}

			if (strncasecmp("QUIT", command, 4) == 0) {
				has_quit = TRUE;
			}


		}
	}


//When client enters QUIT we quit here
	printf("\n\nProcessing complete\n\n");
	exit(0);
}


