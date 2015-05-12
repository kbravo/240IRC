
struct users {
public:
	char * username;
	char * password;

	struct users * next;
};

typedef struct users users;

struct usr_list {
public:
	users * head;
};

typedef struct usr_list usr_list;

struct rooms{
public:
	char * name;
	char ** users;
	
	int mssg_count = 0;
	int usr_count = 0;
	
	char ** messages;
	char ** owners;

	struct rooms * next;
};

typedef struct rooms rooms;


struct room_list {
public:
	rooms * head;
};

typedef struct room_list room_list;
