char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#include "IRCServer.h"

int QueueLength = 5;
room_list rm_list;
usr_list u_list;

int
IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);
  
	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the same port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			     (char *) &optval, sizeof( int ) );
	
	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			  (struct sockaddr *)&serverIPAddress,
			  sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}
	
	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

void
IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);

	initialize();
	
	while ( 1 ) {
		
		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
					  (struct sockaddr *)&clientIPAddress,
					  (socklen_t*)&alen);
		
		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}
		
		// Process request.
		processRequest( slaveSocket );		
	}
}

int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}
	
	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);
	
}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

void
IRCServer::initialize() {
	rm_list.head = NULL;
	u_list.head = NULL;
}

void
IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;
	int i = 1;
	
	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;
	
	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
		read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}
		
		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}
	
	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
    commandLine[ commandLineLength ] = '\0';

	printf("RECEIVED: %s\n", commandLine);

	printf("The commandLine has the following format:\n");
	printf("COMMAND <user> <password> <arguments>. See below.\n");
	printf("You need to separate the commandLine into those components\n");
	printf("For now, command, user, and password are hardwired.\n");
	
	char * str = strtok(commandLine, " ");

	char * command = (char *) malloc(1000*sizeof(char));
	char * user = (char *) malloc(1000*sizeof(char));
	char * password = (char *) malloc(1000*sizeof(char));
	char * args = (char *) malloc(11000*sizeof(char));
	
	while( str != NULL ) {
			if(i == 1) {
				
				strcpy(command, str);
				i++;

			} else if( i == 2 ) {
				
				strcpy(user, str);
				i++;

			} else if( i == 3 ) {
				
				strcpy(password, str);
				i++;

			} else if( i > 3) {
				
				strcat(args, str);
				strcat(args, " ");
				i++;
			}
			str = strtok (NULL, " ");

			if(str == NULL) {
				break;
			}
	}

	args[strlen(args)-1] = '\0';	

	char * msg = (char *) malloc(50*sizeof(char));
	/*char * command = "ADD-USER";
	char * user = "peter";
	char * password = "spider";
	char * args = "";*/
	
	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf( "password=%s\n", password );
	printf("args=%s\n", args);

	if (strcmp(command, "ADD-USER") == 0) {
		addUser(fd, user, password, "");

	} else if(strcmp(command, "CREATE-ROOM") == 0) {
		createRoom(fd, user,password, args);
	}
	else if (strcmp(command, "ENTER-ROOM") == 0) {
		enterRoom(fd, user, password, args);
	}
	else if (strcmp(command, "LEAVE-ROOM") == 0) {
		leaveRoom(fd, user, password, args);
	}
	else if (strcmp(command, "LIST-ROOMS") == 0) {
		listRooms(fd, user, password, "");
	} 
	else if (strcmp(command, "SEND-MESSAGE") == 0) {
		sendMessage(fd, user, password, args);
	}
	else if (strcmp(command, "GET-MESSAGES") == 0) {
		getMessages(fd, user, password, args);
	}
	else if (strcmp(command, "GET-USERS-IN-ROOM") == 0) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (strcmp(command, "GET-ALL-USERS") == 0) {
		getAllUsers(fd, user, password, "");
	}
	else {
		strcpy(msg, "UNKNOWN COMMAND\r\n");
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);	
}



bool
IRCServer::checkPassword(int fd, char * user, char * password) {
	
	users * ptr = u_list.head;

	if(u_list.head == NULL) {
		return true;
	}

	while( ptr != NULL ) {

		if( (strcmp(user, ptr->username) == 0) && 
			(strcmp(password, ptr->password) == 0) ) {
			return true;
		}
		ptr = ptr->next;
	}
	
	return false;
}

void
IRCServer::addUser(int fd, char * user, char * password, char * args)
{
	char * msg = (char *) malloc(50*sizeof(char));

	/*if(checkPassword(fd, user, password) == false) {
		strcpy(msg,"DENIED\r\n");
		write(fd, msg, strlen(msg));

		return;

	} else {*/

		users * ptr1 = u_list.head;
 		while( ptr1 != NULL) {
 			if(strcmp(ptr1->username, user) == 0) {
 				strcpy(msg, "DENIED\r\n");
 				write(fd, msg, strlen(msg));
 				return;
 			}
 			ptr1 = ptr1->next;
 		} 
	
		users * ptr = (users *) malloc(sizeof(users));

		ptr->username = (char *) malloc(1000 * sizeof(char));
		ptr->password = (char *) malloc(1000 * sizeof(char));
		
		strcpy(ptr->username, user);
		strcpy(ptr->password, password);

		ptr->next = u_list.head;
		u_list.head = ptr;

		strcpy(msg,"OK\r\n");
		write(fd, msg, strlen(msg));

		return;	
	//}	
}

void 
IRCServer::createRoom(int fd, char * user, char * password, char * args) {
	char * msg = (char *) malloc(50*sizeof(char));

	if(checkPassword(fd, user, password) == false) {
		strcpy(msg,"ERROR (Wrong password)\r\n");
		write(fd, msg, strlen(msg));

		return;

 	} else {

 		rooms * ptr1 = rm_list.head;

 		while( ptr1 != NULL) {
 			if(strcmp(ptr1->name, args) == 0) {
 				strcpy(msg, "DENIED\r\n");
 				write(fd, msg, strlen(msg));
 				return;
 			}
 			ptr1 = ptr1->next;
 		}

		rooms * ptr = (rooms *) malloc(sizeof(rooms));
		
		ptr->mssg_count = 0;
		ptr->usr_count = 0;
		
		ptr->name = (char*) malloc(1000 * sizeof(char));
		
		ptr->users = (char**) malloc(100 * sizeof(char*));
		ptr->messages = (char**)malloc(100 * sizeof(char*));
		ptr->owners = (char**)malloc(100 * sizeof(char*));

		for (int i = 0; i < 100; i++) {
			ptr->users[i] = (char*) malloc(1000 * sizeof(char));
			ptr->messages[i] = (char*) malloc(10000 * sizeof(char));
			ptr->owners[i] = (char*) malloc(1000 * sizeof(char));
		}
		
		strcpy(ptr->name, args);

		ptr->next = rm_list.head;
		rm_list.head = ptr;

		strcpy(msg,"OK\r\n");
		write(fd, msg, strlen(msg));

		return;
	}

}

bool
IRCServer::userInList(char ** users, char * user, int count) {
	int i = 0;

	for(i = 0; i < count; i++) {
		if(strcmp(user, users[i]) == 0) {
			return true;
		}
	}
	return false;
}

void
IRCServer::enterRoom(int fd, char * user, char * password, char * args)
{
	char * msg = (char *) malloc(50*sizeof(char));

	if(checkPassword(fd, user, password) == false) {
		strcpy(msg,"ERROR (Wrong password)\r\n");
		write(fd, msg, strlen(msg));
		return;

	} else {

		rooms * ptr = rm_list.head;

		while(ptr != NULL) {
			
			if(strcmp(ptr->name, args) == 0) {

				if(userInList(ptr->users, user, ptr->usr_count) == false) {
					
					strcpy(ptr->users[ptr->usr_count], user);
					ptr->usr_count++;

					strcpy(msg,"OK\r\n");
					write(fd, msg, strlen(msg));
					
					return;

				} else {
					
					char * err = (char *) malloc(100*sizeof(char));
					strcpy(err,"OK\r\n");
					write(fd, err, strlen(err));
					return;
				}
			}
			ptr = ptr->next;
		}
		
		char * err2 = (char *) malloc(100*sizeof(char));
		strcpy(err2,"ERROR (No room)\r\n");
		write(fd,err2,strlen(err2));
		return;
	}
}

//testing needed
void
IRCServer::leaveRoom(int fd, char * user, char * password, char * args)
{
	char * msg = (char *) malloc(50*sizeof(char));
	int i,j, pos;
	if(checkPassword(fd, user, password) == false) {
		strcpy(msg,"ERROR (Wrong password)\r\n");
		write(fd, msg, strlen(msg));

		return;

	} else {
		
		rooms * ptr = rm_list.head;

		while(ptr != NULL) {

			if(strcmp(ptr->name, args) == 0) {
				if(userInList(ptr->users, user, ptr->usr_count) == true) {
					i =0;
					j = 0;
					for(i = 0; i < ptr->usr_count; i++) {
						
						if(strcmp(ptr->users[i], user) == 0) {
							pos = i;
							for(j = pos; j < ptr->usr_count; j++) {
								ptr->users[j] = ptr->users[j+1];
							}
							ptr->usr_count--;

							strcpy(msg,"OK\r\n");
							write(fd,msg,strlen(msg));
							return;
						}
					}
				} else {

					char * err = (char *) malloc(100*sizeof(char));
					strcpy(err,"ERROR (No user in room)\r\n");
					write(fd,err,strlen(err));
					return;
				}
			}
		}
		char * err2 = (char *) malloc(100*sizeof(char));
		strcpy(err2,"ERROR (No user in room)\r\n");
		write(fd,err2,strlen(err2));
		return;
	}
}

void
IRCServer::sendMessage(int fd, char * user, char * password, char * args)
{
	char * msg = (char *) malloc(50*sizeof(char)); 

	if(checkPassword(fd, user, password) == false) {
		strcpy(msg,"ERROR (Wrong password)\r\n");
		write(fd, msg, strlen(msg));

		return;

	} else {
		
		char * room_name = (char *) malloc (1000*sizeof(char));
		char * message = (char *) malloc (10000*sizeof(char));

		char * str = strtok(args, " ");
		int i = 1;

		while( str != NULL ) {
			if(i == 1) {
				strcpy(room_name, str);
				i++;
			} else if(i > 1) {
				strcat(message, str);
				strcat(message," ");
			}
			str = strtok (NULL, " ");
			if( str == NULL ) {
				break;
			}
		}
		bool flag = false;
		message[strlen(message)-1] = '\0';

		rooms * ptr = rm_list.head;

		while(ptr != NULL) {
			if(strcmp(ptr->name, room_name) == 0) {
				
				for(int p = 0; p < ptr->usr_count; p++) {
					if(strcmp(ptr->users[p], user) == 0) {
						flag = true;
						break;
					}
				}

				if(flag == false) {
					strcpy(msg, "ERROR (user not in room)\r\n");
					write(fd, msg, strlen(msg));
					return;
				}

				strcpy(ptr->messages[ptr->mssg_count], message);
				strcpy(ptr->owners[ptr->mssg_count], user);
				ptr->mssg_count++;
				
				strcpy(msg,"OK\r\n");
				write(fd, msg, strlen(msg));
				return;
			}
			ptr = ptr->next;
		}

		strcpy(msg,"ERROR (user not in room)\r\n");
		write(fd, msg, strlen(msg));
		return;
	}
}

void
IRCServer::getMessages(int fd, char * user, char * password, char * args)
{
	char * msg = (char *) malloc(1000*sizeof(char));
	if(checkPassword(fd, user, password) == false) {
		
		strcpy(msg,"ERROR (Wrong password)\r\n");
		write(fd, msg, strlen(msg));

		return;
	
	} else {

		char * room_name = (char *) malloc (1000*sizeof(char));
		char * last_message = (char *) malloc (10*sizeof(char));

		char * str = strtok(args, " ");
		int i = 1;
		
		
		while( str != NULL ) {
			if(i == 1) {
				strcpy(last_message, str);
				i++;
			} else {
				strcpy(room_name, str);
			}
			
			str = strtok (NULL, " ");
			
			if(str == NULL) {
				break;
			}
		}

		rooms * ptr = rm_list.head;

		bool flag = false;
		int p = 0;

		i = 0;

		int lm = atoi(last_message);

		

		while(ptr != NULL) {
			
			if(strcmp(ptr->name, room_name) == 0) {

				for(int p = 0; p < ptr->usr_count; p++) {
					if(strcmp(ptr->users[p], user) == 0) {
						flag = true;
						break;
					}
				}

				if(flag == false) {
					strcpy(msg, "ERROR (User not in room)\r\n");
					write(fd, msg, strlen(msg));
					return;
				}
				
				//testing needed
				if(lm >= ptr->mssg_count) {
			
					strcpy(msg,"NO-NEW-MESSAGES");
					write(fd, msg, strlen(msg));
				}


				char mychar[3];
				for(i = lm; i < ptr->mssg_count; i++) {
					sprintf(mychar, "%d", i);
					strcat(msg, mychar);
					strcat(msg, " ");
					strcat(msg, ptr->owners[i]);
					strcat(msg, " ");
					strcat(msg, ptr->messages[i]);
					strcat(msg, "\r\n");
					write(fd, msg, strlen(msg));
					strcpy(msg, "");
				}
				write(fd, "\r\n", 2);
				return;
			}
			ptr = ptr->next;

		}

		strcpy(msg,"DENIED\r\n");
		write(fd, msg, strlen(msg));

	}
}

void IRCServer::listRooms(int fd, char *user, char *password, char *args) {
	if (checkPassword(fd, user, password) == false) {
		const char *msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
	} else {
		rooms * ptr = rm_list.head;

		char * buffer = (char *) malloc(100*sizeof(char));
		char ** array = (char **) malloc(100*sizeof(char*));
		for(int z = 0; z < 100; z++) {
			array[z] = (char*) malloc(1000*sizeof(char));
		}

		int h = 0;
		
		while (ptr != NULL) {
			array[h] = ptr->name;
			h++;
			ptr = ptr->next;
		}

		char * temp = (char *) malloc(1000* sizeof(char));
		
		for (int i = 0; i < h; i++) {
			for (int j = i + 1; j < h; j++) {
				if (strcmp(array[i], array[j]) > 0) {
					temp = array[i];
					array[i] = array[j];
					array[j] = temp;
				}
			}
		}
		for (int i = 0; i < h; i++) {
			snprintf(buffer, 100, "%s\r\n", array[i]);
			write(fd, buffer, strlen(buffer));
			write(fd, "\r\n", 2);
		}
	}
}


void
IRCServer::getUsersInRoom(int fd, char * user, char * password, char * args)
{
	char * msg = (char *) malloc(50*sizeof(char));

	if(checkPassword(fd, user, password) == false) {
		strcpy(msg,"ERROR (Wrong password)\r\n");
		write(fd, msg, strlen(msg));
		return;
	} else {
		char ** array = (char **) malloc(100*sizeof(char*));
		for(int z = 0; z < 100; z++) {
			array[z] = (char*) malloc(1000*sizeof(char));
		}

		int h = 0;
		
		int g = 0;

		char * buffer = (char *) malloc(100*sizeof(char));
		
		int i = 0;

		rooms * ptr = rm_list.head;
		
		while(ptr != NULL) {
			if(strcmp(ptr->name, args) == 0) {
				for(i = 0; i < ptr->usr_count; i++) {
					array[h] = ptr->users[i];
					h++;
				}
				break;
			}
			ptr = ptr->next;
		}
		char * temp = (char *) malloc(1000* sizeof(char));

		for (int i = 0; i < h; i++) {
			for (int j = i + 1; j < h; j++) {
				if (strcmp(array[i], array[j]) > 0) {
					temp = array[i];
					array[i] = array[j];
					array[j] = temp;
				}
			}
		}
		for (int i = 0; i < h; i++) {
			snprintf(buffer, 100, "%s\r\n", array[i]);
			write(fd, buffer, strlen(buffer));
		}
		write(fd, "\r\n", 2);
		return;
	}
}

void
IRCServer::getAllUsers(int fd, char * user, char * password, char * args)
{
	char * msg = (char *) malloc(50*sizeof(char));

	if(checkPassword(fd, user, password) == false) {
		strcpy(msg,"ERROR (Wrong password)\r\n");
		write(fd, msg, strlen(msg));

		return;

	} else {

		char ** array = (char **) malloc(100*sizeof(char*));
		for(int z = 0; z < 100; z++) {
			array[z] = (char*) malloc(1000*sizeof(char));
		}

		int h = 0;
		char * buffer = (char *) malloc(100*sizeof(char));

		users * ptr = u_list.head;

		while(ptr != NULL) {
			array[h] = ptr->username;
			h++;
			ptr = ptr->next;
		}

		char * temp = (char *) malloc(1000* sizeof(char));

		for (int i = 0; i < h; i++) {
			for (int j = i + 1; j < h; j++) {
				if (strcmp(array[i], array[j]) > 0) {
					temp = array[i];
					array[i] = array[j];
					array[j] = temp;
				}
			}
		}

		for (int i = 0; i < h; i++) {
			snprintf(buffer, 100, "%s\r\n", array[i]);
			write(fd, buffer, strlen(buffer));
		}
		write(fd, "\r\n", 2);
	}
}
