/*
* Jan 10, 2017  is The Last Update
* server.c
*
* PURPOSE: An ftp server.
* INSTRUCTIONS:
*
*       There will be 1 argument in the command line
*       ./server
*
*/

#include "includes.h"
#include "ftpServerFunctions.h"

enum states {STATE_NOT_CONNECTED, STATE_CONNECTED, STATE_WAITING_FOR_PASS, STATE_LOGGED_IN}; /* Status of The User */


/* The Main Process */
int main(int argc, char **argv) {
    int listenfd, controlSocket;    /*Sockets*/
    uint16_t portNo = atoi(PORT);
    enum states state;

    checkParameters(argc);

    listenfd = createSocketServer(portNo);
    if (!listenfd) {
        exit(1);
    }

    printf("\nServer is Waiting a Client\n\n");


    for ( ; ; )
    {

        int pid;

        state = STATE_NOT_CONNECTED;

        if ((controlSocket = accept(listenfd, (struct sockaddr*) NULL, NULL)) < 0)
        {
            perror("Error calling accept().");
            exit(EXIT_FAILURE);
        }
        print_connection_established(controlSocket);
        state = STATE_CONNECTED;

        pid = fork();

        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        /* This is the client process */
        if (pid == 0) {
            close(listenfd);
            doProcessing(controlSocket, &state);
            exit(0);
        }

        else close(controlSocket);

    }

    close(dataSocket);
    close(listenfd);
    return 0;
}


