
#ifndef IRC_SERVER
#define IRC_SERVER
#include "IRCLists.h"
#define PASSWORD_FILE "password.txt"


class IRCServer {


private:
	int open_server_socket(int port);

public:
	void initialize();
	bool checkPassword(int fd,  char * user,  char * password);
	void processRequest( int socket );
	void createRoom(int fd,  char * user, char * password,  char * args);
	void listRooms(int fd,  char * user,  char * password,  char * args);
	void addUser(int fd,  char * user,  char * password,  char * args);
	void enterRoom(int fd,  char * user,  char * password,  char * args);
	void leaveRoom(int fd,  char * user,  char * password,  char * args);
	void sendMessage(int fd,  char * user,  char * password,  char * args);
	void getMessages(int fd,  char * user,  char * password,  char * args);
	void getUsersInRoom(int fd,  char * user,  char * password,  char * args);
	void getAllUsers(int fd,  char * user,  char * password,  char * args);
	void runServer(int port);
	bool userInList(char ** users, char * user, int count);
};

#endif
