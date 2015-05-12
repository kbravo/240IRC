#include <time.h>
//#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>


#include <stdio.h>
#include <gtk/gtk.h>

char * host = (char *) malloc(1000*sizeof(char));

char * user = (char*) malloc(1000*sizeof(char));
char * password = (char*) malloc(1000*sizeof(char));
char * current_room_selection = (char*) malloc(1000*sizeof(char));
char * current_room_posted = (char*) malloc(1000*sizeof(char));
char * sport;

static int RoomCount = 0;
GtkWidget *fixed;
GtkWidget *combo;

GtkWidget *fixed1;
GtkWidget *combo1;

GtkWidget *messages;
GtkWidget *myMessage;

static char * argument;

GtkWidget *entry1;
GtkWidget *entry2;

GtkWidget *entryRoom;
GtkWidget *msgs;


int port;

#define MAX_MESSAGES 100
#define MAX_MESSAGE_LEN 300
#define MAX_RESPONSE (20 * 1024)

int lastMessage = 0;

//GtkListStore * list_rooms;
GtkListStore * list_names;
GtkListStore * message_list;

GtkListStore * list_current_room;

int open_client_socket(char * host, int port) {
  // Initialize socket address structure
  struct  sockaddr_in socketAddress;

  strcpy(host, "localhost");
  port = 1200;
  // Clear sockaddr structure
  memset((char *)&socketAddress,0,sizeof(socketAddress));
  
  // Set family to Internet 
  socketAddress.sin_family = AF_INET;
  
  // Set port
  socketAddress.sin_port = htons((u_short)port);
  
  // Get host table entry for this host
  struct  hostent  *ptrh = gethostbyname(host);
  if ( ptrh == NULL ) {
    perror("gethostbyname");
    exit(1);
  }
  
  // Copy the host ip address to socket address structure
  memcpy(&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);
  
  // Get TCP transport protocol entry
  struct  protoent *ptrp = getprotobyname("tcp");
  if ( ptrp == NULL ) {
    perror("getprotobyname");
    exit(1);
  }
  
  // Create a tcp socket
  int sock = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
  if (sock < 0) {
    perror("socket");
    exit(1);
  }
  
  // Connect the socket to the specified server
  if (connect(sock, (struct sockaddr *)&socketAddress,
        sizeof(socketAddress)) < 0) {
    perror("connect");
    exit(1);
  }
  
  return sock;
}

int sendCommand(char * host, int port, char * command, char * user,
    char * password, char * args, char * response) {
  int sock = open_client_socket( host, port);

  // Send command
  write(sock, command, strlen(command));
  write(sock, " ", 1);
  write(sock, user, strlen(user));
  write(sock, " ", 1);
  write(sock, password, strlen(password));
  write(sock, " ", 1);
  write(sock, args, strlen(args));
  write(sock, "\r\n",2);

  // Keep reading until connection is closed or MAX_REPONSE
  int n = 0;
  int len = 0;
  while ((n=read(sock, response+len, MAX_RESPONSE - len))>0) {
    len += n;
  }

  //printf("response:%s\n", response);

  close(sock);
}

void printUsage()
{
  printf("Usage: talk-client host port user password\n");
  exit(1);
}

void add_user(GtkWidget *widget, gpointer data) {
  // Try first to add user in case it does not exist.
  strcpy(user, "");
  strcpy(password, "");

  strcpy(user, gtk_entry_get_text (GTK_ENTRY(entry1)));
  strcpy(password, gtk_entry_get_text (GTK_ENTRY(entry2)));
  
  //printf("%s\n", user);
  //printf("%s\n", password);
  
  char response[MAX_RESPONSE];
  sendCommand(host, port, "ADD-USER", user, password, "\0", response);
  
  if (strcmp(response,"OK\r\n") == 0) {
    printf("User %s added\n", user);
  }
 
  
}


void 
enter_room(GtkWidget *widget, gpointer data) {
  char * args = (char*) malloc(10000*sizeof(char));
  strcpy(args, current_room_selection);
  
  //printf("%s\n", current_room_selection );
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo1), current_room_selection);
  
  char response[MAX_RESPONSE];
  sendCommand(host, port, "ENTER-ROOM", user, password, args, response);
  
  if (strcmp(response,"OK\r\n") == 0) {
    printf("User %s entered %s\n", args);
  }
  

}

void room_Creation(GtkWidget *button, gpointer data)
{
  char * args = (char*) malloc(10000*sizeof(char));

  strcpy(args, gtk_entry_get_text (GTK_ENTRY(entryRoom)));

  char response[MAX_RESPONSE];
  sendCommand(host, port, "CREATE-ROOM", user, password, args, response);
  RoomCount++;
  if (strcmp(response,"OK\r\n") == 0) {
    printf("Room made%s\n", args);
    
  }

}

void create_room(GtkWidget *button, gpointer data) {
    GtkWidget *window;
    GtkWidget *table;
    GtkWidget *label1;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "Room signup");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    GtkWidget *create_button = gtk_button_new_with_label ("Create!");

    table = gtk_table_new(2, 3, FALSE);
    gtk_container_add(GTK_CONTAINER(window), table);

    label1 = gtk_label_new("Room name?");
    gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 1, 0, 1);
    
    entryRoom = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), entryRoom, 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE (table), create_button, 0, 1, 2, 3);
    
    g_signal_connect(G_OBJECT(create_button), "clicked", G_CALLBACK(room_Creation), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main), NULL);

    gtk_widget_show(table);

    gtk_widget_show(label1);

    gtk_widget_show(entry1);

    gtk_widget_show_all(window);
    
}




void leave_room(GtkWidget *widget, gpointer data) {

  char * args = (char*) malloc(10000*sizeof(char));
  if(gtk_combo_box_get_active (GTK_COMBO_BOX(combo1)) == -1) {
    return;
  }
  //if(gtk_combo_box_get_has_entry (GTK_COMBO_BOX(combo1)) == true) {
  strcpy(args, gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo1)));
  //}
  //strcpy(args, current_room_posted);
  //if(gtk_combo_box_get_has_entry (GTK_COMBO_BOX(combo1)) == true) {
  gtk_combo_box_remove_text (GTK_COMBO_BOX(combo1), gtk_combo_box_get_active (GTK_COMBO_BOX(combo1)));
  //}
  char response[MAX_RESPONSE];

  sendCommand(host, port, "LEAVE-ROOM", user, password, args, response);
  
  if (strcmp(response,"OK\r\n") == 0) {
    printf("User left room %s\n", args);
  }
  
  
}

void
get_messages() {

  char * args = (char*) malloc(10000*sizeof(char));
  if(gtk_combo_box_get_active (GTK_COMBO_BOX(combo1)) == -1) {
    return;
  }
  
  gchar *msg;

  strcpy(args, "0 ");
  strcat(args, gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo1)));

  char response[MAX_RESPONSE];
  sendCommand(host, port, "GET-MESSAGES", user, password, args, response);    
  
  GtkTreeIter iter;
  int i;
    
  if( *response == '\0') {
    return;
  }

  char * pch;
  pch = strtok(response, "\r\n");
  
  if(strcmp(response, "DENIED\r\n") != 0 || strcmp(response, "ERROR (User not in room)\r\n") != 0) {
    
    gtk_list_store_clear(GTK_LIST_STORE(message_list));
    
    while(pch != NULL) {
      
      msg = g_strdup_printf (pch);
      gtk_list_store_append (GTK_LIST_STORE (message_list), &iter);
      gtk_list_store_set (GTK_LIST_STORE (message_list), &iter, 0, msg,-1);
      g_free(msg);
   


      pch = strtok (NULL, "\r\n");

      if(pch == NULL) {
        break;
      }
    
    }
  }

  
}

void send_message(GtkWidget *widget, gpointer data) {

    char * args = (char*) malloc(10000*sizeof(char));
    if(gtk_combo_box_get_active (GTK_COMBO_BOX(combo1)) == -1) {
      return;
    }
    strcpy(args, gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo1)));
    strcat(args, " ");
    strcat(args, gtk_entry_get_text (GTK_ENTRY(myMessage)));
    
  char response[MAX_RESPONSE];
  sendCommand(host, port, "SEND-MESSAGE", user, password, args, response);
  
  if (strcmp(response,"OK\r\n") == 0) {
    printf("Message sent %s\n", user);
  }
  
  
  

}

void print_users_in_room() {

  char * args = (char*) malloc(10000*sizeof(char));
  if(gtk_combo_box_get_active (GTK_COMBO_BOX(combo1)) == -1) {
    return;
  }

  strcpy(args, gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo1)));

  char response[MAX_RESPONSE];
  sendCommand(host, port, "GET-USERS-IN-ROOM", user, password, args, response);

  if( *response == '\0') {
    return;
  }

  GtkTreeIter iter;
    int i;
  
  gchar *msg;
  char * pch;

  pch = strtok(response, "\r\n");
  
  if(strcmp(response, "ERROR (Wrong password)\r\n") != 0) {
    gtk_list_store_clear(GTK_LIST_STORE(list_names));
    while(pch != NULL) {
      
      msg = g_strdup_printf (pch);

          gtk_list_store_append (GTK_LIST_STORE (list_names), &iter);
          gtk_list_store_set (GTK_LIST_STORE (list_names), &iter, 0, msg,-1);
          g_free(msg);
       
      
      pch = strtok (NULL, "\r\n");

      if(pch == NULL) {
        break;
      }
    }
  }

  

  
}

void print_users() {

}

void printPrompt() {
  printf("talk> ");
  fflush(stdout);
}

void printHelp() {
  printf("Commands:\n");
  printf(" -who   - Gets users in room\n");
  printf(" -users - Prints all registered users\n");
  printf(" -help  - Prints this help\n");
  printf(" -quit  - Leaves the room\n");
  printf("Anything that does not start with \"-\" will be a message to the chat room\n");
}

/*void * getMessagesThread(void * arg) {
  // This code will be executed simultaneously with main()
  // Get messages to get last message number. Discard the initial Messages
  
  while (1) {
    // Get messages after last message number received.

    // Print messages

    // Sleep for ten seconds
    usleep(2*1000*1000);
  }
}

void startGetMessageThread()
{
  pthread_create(NULL, NULL, getMessagesThread, NULL);
}*/

void combo_selected(GtkWidget *widget, gpointer window)
{ 
  gchar *text =  gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
  strcpy(current_room_selection, text);
}

void
refreshCOMBO() {
  for(int i = 0; i < RoomCount; i++) {
    gtk_combo_box_remove_text (GTK_COMBO_BOX(combo), i);
  }
    //combo = gtk_combo_box_new_text();
}

void update_list_rooms() {

  int i = 1;

  char response[MAX_RESPONSE];
  
  sendCommand(host, port, "LIST-ROOMS", user, password, "\0", response);
  
  if( *response == '\0' ) {
    return;
  }

  char * pch;
  
  pch = strtok(response, "\r\n");

  if(strcmp(response, "ERROR (Wrong password)\r\n") != 0) {
   
    while(pch != NULL) {

        if(strcmp(pch, "ERROR (Wrong password)") == 0) {

        } else {

          gtk_combo_box_append_text(GTK_COMBO_BOX(combo), pch);
        }

      pch = strtok (NULL, "\r\n");
      
      if(pch == NULL) {
        break;
      }
    }
    
  }
  
      /*gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Ubuntu");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Mandriva");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Fedora");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Mint");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Gentoo");
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Debian");*/
}

static gboolean
time_handler(GtkWidget *widget)
{
  if (widget->window == NULL) return FALSE;

  //gtk_widget_queue_draw(widget);

  fprintf(stderr, "Hi\n");
 
  get_messages();
  print_users_in_room();

  //argument = get_messages();
  

  return TRUE;
}

/* Create the list of "messages" */
static GtkWidget *create_list( const char * titleColumn, GtkListStore *model )
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    //GtkListStore *model;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

    int i;
   
    /* Create a new scrolled window, with scrollbars only if needed */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
            GTK_POLICY_AUTOMATIC, 
            GTK_POLICY_AUTOMATIC);
   
    //model = gtk_list_store_new (1, G_TYPE_STRING);
    tree_view = gtk_tree_view_new ();
    gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
    gtk_widget_show (tree_view);
   
    cell = gtk_cell_renderer_text_new ();

    column = gtk_tree_view_column_new_with_attributes (titleColumn,
                                                       cell,
                                                       "text", 0,
                                                       NULL);
  
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
                 GTK_TREE_VIEW_COLUMN (column));

    return scrolled_window;
}
   
/* Add some text to our text widget - this is a callback that is invoked
when our window is realized. We could also force our window to be
realized with gtk_widget_realize, but it would have to be part of
a hierarchy first */

static void insert_text( GtkTextBuffer *buffer, const char * initialText )
{
   GtkTextIter iter;
 
   gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
   gtk_text_buffer_insert (buffer, &iter, initialText,-1);
}
   
/* Create a scrolled text area that displays a "message" */
static GtkWidget *create_text( const char * initialText )
{
   GtkWidget *scrolled_window;
   GtkWidget *view;
   GtkTextBuffer *buffer;

   view = gtk_text_view_new ();
   buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

   scrolled_window = gtk_scrolled_window_new (NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                   GTK_POLICY_AUTOMATIC,
           GTK_POLICY_AUTOMATIC);

   gtk_container_add (GTK_CONTAINER (scrolled_window), view);
   insert_text (buffer, initialText);

   gtk_widget_show_all (scrolled_window);

   return scrolled_window;
}

void sign_up_procedure(GtkWidget *button, gpointer data) {
  GtkWidget *window;
  GtkWidget *table;

  GtkWidget *label1;
  GtkWidget *label2;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(window), "New User signup");
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);

  GtkWidget *submit_button = gtk_button_new_with_label ("Sign up!");
  GtkWidget *room_button = gtk_button_new_with_label ("Create Room?");

  table = gtk_table_new(3, 3, FALSE);
  gtk_container_add(GTK_CONTAINER(window), table);

  label1 = gtk_label_new("Username");
  label2 = gtk_label_new("Password");


  gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(table), label2, 0, 1, 1, 2);

  entry1 = gtk_entry_new();
  entry2 = gtk_entry_new();
  gtk_entry_set_visibility (GTK_ENTRY(entry2), FALSE);
  gtk_table_attach_defaults(GTK_TABLE(table), entry1, 1, 2, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(table), entry2, 1, 2, 1, 2);

  gtk_table_attach_defaults(GTK_TABLE (table), submit_button, 2, 3, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE (table), room_button, 0, 2, 2, 3 );
  g_signal_connect(G_OBJECT(submit_button), "clicked", G_CALLBACK(add_user), NULL);
  g_signal_connect(G_OBJECT(room_button), "clicked", G_CALLBACK(create_room), NULL); 
  gtk_widget_show (submit_button);

  gtk_widget_show(table);

  gtk_widget_show(label1);
  gtk_widget_show(label2);

  gtk_widget_show(entry1);
  gtk_widget_show(entry2);

  gtk_widget_show_all(window);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main), NULL);

}

int main( int   argc,
          char *argv[] )
{
    char * msgs = (char *) malloc(10000*sizeof(char));
    
    GtkWidget *window;

    GtkWidget *list2;
    GtkWidget *list3;
    
    GtkWidget *label;
    GtkWidget *label1;

    gtk_init (&argc, &argv);
   
    //WINDOW SETUP
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "240 Internet Relay Chat");
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_widget_set_size_request (GTK_WIDGET (window), 800, 600 );

    //GRID SETUP
    // Create a table to place the widgets. Use a 7x4 Grid (7 rows x 4 columns)
    GtkWidget *table = gtk_table_new (7, 4, TRUE);
    gtk_container_add (GTK_CONTAINER (window), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);

    //COMBO BOX FOR ALL ROOMS
    // Add list of rooms. Use columns 0 to 4 (exclusive) and rows 0 to 4 (exclusive)
    //list_rooms = gtk_list_store_nYkew (1, G_TYPE_STRING);
    
    fixed = gtk_fixed_new();
    combo = gtk_combo_box_new_text();

    update_list_rooms();
    
    gtk_fixed_put(GTK_FIXED(fixed), combo, 50, 50);
    label = gtk_label_new("Rooms");
    gtk_fixed_put(GTK_FIXED(fixed), label, 50, 30 );
    gtk_table_attach_defaults (GTK_TABLE (table), fixed, 2, 3, 0, 1);
    g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_selected), (gpointer) label);

    GtkWidget *refresh_button = gtk_button_new_with_label ("Refresh");
    gtk_table_attach_defaults(GTK_TABLE (table), refresh_button, 2, 3, 1, 2);
    g_signal_connect(G_OBJECT(refresh_button), "clicked", G_CALLBACK(update_list_rooms), NULL); 
    gtk_widget_show (refresh_button);
    
    //COMBO BOX FOR CURRENT ROOMS
    fixed1 = gtk_fixed_new();
    combo1 = gtk_combo_box_new_text();
    gtk_fixed_put(GTK_FIXED(fixed1), combo1, 50, 50);
    label1 = gtk_label_new("Currently in rooms");
    gtk_fixed_put(GTK_FIXED(fixed1), label1, 50, 30 );
    gtk_table_attach_defaults (GTK_TABLE (table), fixed1, 3, 4, 0, 2);
    
    //FOR USERS IN A ROOM
    list_names = gtk_list_store_new (1, G_TYPE_STRING);

    print_users_in_room();
    
    list2 = create_list ("Users", list_names);
    gtk_table_attach_defaults (GTK_TABLE (table), list2, 0, 2, 0, 2);
    gtk_widget_show (list2);
    
    //GET MESSAGES IN A ROOM
    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive) 
    message_list = gtk_list_store_new (1, G_TYPE_STRING);

    get_messages();
    
    list3 = create_list ("Messages", message_list);
    gtk_table_attach_defaults (GTK_TABLE (table), list3, 0, 4, 2, 6);
    gtk_widget_show (list3);
    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive) 
    
    //SENDING STUFF
    myMessage = gtk_entry_new();
    gtk_table_attach_defaults (GTK_TABLE (table), myMessage, 0, 4, 6, 7);
    gtk_widget_show (myMessage);
    
    // Add send button. Use columns 0 to 1 (exclusive) and rows 4 to 7 (exclusive)
    GtkWidget *send_button = gtk_button_new_with_label ("Send");
    gtk_table_attach_defaults(GTK_TABLE (table), send_button, 0, 1, 7, 8);
    g_signal_connect(G_OBJECT(send_button), "clicked", G_CALLBACK(send_message), NULL); 
    gtk_widget_show (send_button);
    
    GtkWidget *sign_up_button = gtk_button_new_with_label ("Sign Up");
    gtk_table_attach_defaults(GTK_TABLE (table), sign_up_button, 3, 4, 7, 8);
    g_signal_connect(G_OBJECT(sign_up_button), "clicked", G_CALLBACK(sign_up_procedure), NULL);
    gtk_widget_show (sign_up_button);
    
    GtkWidget *enter_room_button = gtk_button_new_with_label ("Enter Room");
    gtk_table_attach_defaults(GTK_TABLE (table), enter_room_button, 1, 2, 7, 8);
    g_signal_connect(G_OBJECT(enter_room_button), "clicked", G_CALLBACK(enter_room), NULL);
    
    GtkWidget *leave_room_button = gtk_button_new_with_label ("Leave Room");
    gtk_table_attach_defaults(GTK_TABLE (table), leave_room_button, 2, 3, 7, 8);
    g_signal_connect(G_OBJECT(leave_room_button), "clicked", G_CALLBACK(leave_room), NULL);
    
    gtk_widget_show_all (table);
    gtk_widget_show (window);
    
    g_timeout_add(2500, (GSourceFunc) time_handler, (gpointer) window);
    
    gtk_main ();
    

    return 0;
}

