# ftp-protocol

Implementation of FTP Protocol according to RFC 959 Using Sockets in C Language

TCP Server and Client 

Server Side :

Compile it : gcc -o server server.c users.c ftpServerFunctions.c

Type in the command line : ./server




Client Side :

Compile it : gcc -o client client.c ftpClientFunctions.c

Type in the command line : ./client <ip-address of the server>




hello.txt is just to try the RESM command, Type PASV then Type RESM hello.txt, and the unreceived character will be received.
You can see number of remaining bytes in the response. 
And you can try this command with any large file you want , Just RETR a large file and click CTRL+C to abort the connection before retreiving the entire file , then try RESM command.

