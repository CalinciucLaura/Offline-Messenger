#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>

sqlite3 *db; //database handle
char *ErrMsg = 0;
int rc;

int getID(char* username);
int islogged(char* username);

int callback(int* loggedIN, int argc, char **argv, 
                    char **azColName) {
    
    *loggedIN = 0;

    if(argc == 1)
      *loggedIN = 1;

    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
       return 0;
}

int callback2(int* id, int argc, char **argv, 
                    char **azColName) {

    for (int i = 0; i < argc; i++) {
        *id = atoi(argv[i]);
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
       
    }
       return 0;
}


// o sa fie utila si pt vizualizare istoric
int callback3(char msg[20000] , int argc, char **argv, 
                    char **azColName) {

    char temporary[30000]="";
    sprintf(temporary, " Message from %s: %s\n", argv[0], argv[1]);  
    strcat(msg, temporary);

    return 0;
}

int callback4(char msg[20000] , int argc, char **argv, 
                    char **azColName) {

    char temporary[30000]="";
    sprintf(temporary, " %s . Message from %s: %s\n", argv[0], argv[1], argv[2]);  
    strcat(msg, temporary);

    return 0;
}


void registerrr(char *v, char* message_to_client)
{  
  //vezi daca utilizatorul exista deja
  
  char vectorCuv[5][11];
  char *cuv = strtok(v, " ");
  int total = 0;
  while(cuv != NULL)
  {
    strcpy(vectorCuv[total], cuv);
    total++;
    cuv = strtok(NULL, " ");
 }

 if(getID(vectorCuv[1]) != 0)
 strcpy(message_to_client, "Nume de utilizator deja existent. Incearca altceva :)");
 else
 { 
 char sql[150];
 sprintf(sql, "INSERT INTO MESSENGER (username,password,isLogged) VALUES ('%s','%s', %d);", vectorCuv[1], vectorCuv[2], 0);

 rc = sqlite3_exec(db, sql, 0, 0, &ErrMsg);

  strcpy(message_to_client, "User creat cu succes! Te rugam sa te loghezi 'login <username> <parola>");
 }
}


int islogged(char* username)
{
  int loggedIN = 0;

 char sql[100];
 sprintf( sql, "SELECT isLogged from Messenger where username='%s';", username);

 int rc = sqlite3_exec(db, sql, callback, &loggedIN, &ErrMsg); 
     
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);

        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
    } 

printf("Sunt logat = %d\n", loggedIN);
 return loggedIN; 
}

int exists_username_and_password(char* username, char* password)
{
 int id = 0;
 char sql[100];
 sprintf( sql, "SELECT ID from Messenger where username='%s' and password='%s';", username, password);

 int rc = sqlite3_exec(db, sql, callback2, &id, &ErrMsg);
     
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);

        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
    } 
 return id;
}

int e_mesaj(char* username)
{
 int ok = 0;
 char sql[100];
 sprintf( sql, "SELECT COUNT(seen) from Chat where id_destinatie=%d and seen = 0;", getID(username));

 int rc = sqlite3_exec(db, sql, callback2, &ok, &ErrMsg);

    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);

        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
    } 
 
 if(ok == 0)
 return 0;
 else
 return 1;
}

void login(char v[100], char* message_to_client, int descriptor_sursa, char* username)  // caut sa vad daca este in baza de date
{
  
  char vectorCuv[5][11];
  char *cuv = strtok(v, " ");
  int total = 0;


  while(cuv != NULL)
  {
    strcpy(vectorCuv[total], cuv);
    total++;
    cuv = strtok(NULL, " ");
 }

 //de facut select daca exista cineva cu username-ul si parola respectiva
 if ( exists_username_and_password(vectorCuv[1], vectorCuv[2]) == 0)
 {
    strcpy(message_to_client, "Verifica din nou username-ul si parola ori te rugam sa te autentifici!");
    return;
 }


 char sql[150];
  sprintf( sql, "UPDATE Messenger set isLogged = 1, descriptor = %d where username='%s' and password='%s'; ", descriptor_sursa, vectorCuv[1], vectorCuv[2] );
 int rc = sqlite3_exec(db, sql, NULL, 0, &ErrMsg);

    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);

        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
    } 

      
    if(e_mesaj(vectorCuv[1]) != 0) //am primit mesaje cand nu eram conectati
    sprintf( message_to_client, "Logat cu succes! \n Ai mesaje necitite. \n Introdu comanda 'mesaje_offline' pentru a vizualiza mesajele. Introdu comanda 'send message to <username> pentru a trimite mesaje.");
    else
    sprintf(message_to_client, "Logat cu succes!");
    
    strcpy(username, vectorCuv[1]);

}

void logout(int descriptor_sursa)
{ 
  
 char sql[100];
 sprintf( sql, "UPDATE Messenger set isLogged=0 , descriptor = 0 where descriptor = %d;", descriptor_sursa);
 rc = sqlite3_exec(db, sql, callback, 0, &ErrMsg);

    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);

        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
    } 
}

int getID(char* username)
{
 int id = 0;
 char sql[100];
 sprintf( sql, "SELECT ID from Messenger where username='%s';", username);

 int rc = sqlite3_exec(db, sql, callback2, &id, &ErrMsg);

 printf("Mesah %s\n", sql);
 printf("ID-ul este %d\n", id);
     
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
       fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);

        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
    } 
 return id;
}

void send_message_to(char v[100], char* message_to_client, char* username,  char* user_destination, int *am_introdus_comanda_send_before)
{ //send message to <username>
  
  char vectorCuv[5][11];
  char *cuv = strtok(v, " ");
  int total = 0;

  while(cuv != NULL)
  {
    strcpy(vectorCuv[total], cuv);
    total++;
    cuv = strtok(NULL, " ");
 }
 
 strcpy(user_destination, vectorCuv[3]);
 
 if(getID(user_destination) == 0)
 strcpy(message_to_client, "Utilizator inexistent");

 else{
      if(islogged(username) == 0)
        strcpy(message_to_client, "Te rugam sa te loghezi mai intai!");
      else
      {
        strcpy(message_to_client, "[Incepem conversatia]"); 
        *am_introdus_comanda_send_before = 1;

      }
 }
 
}

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
  int id_userBD;
	int cl; //descriptorul intors de accept
}thData;



int getDescriptor(int id)
{
  int descriptor = 0;
 char sql[100];
 sprintf( sql, "SELECT descriptor from Messenger where ID=%d;",  id);

 int rc = sqlite3_exec(db, sql, callback2, &descriptor, &ErrMsg);

 printf("Descriptorul este %d\n", descriptor);
     
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);

        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
    } 
 return descriptor;
}

void during_conversation(char* message_from_client, char* message_to_client, char* username, char* user_destination) //avem datale necesare, acum doar dam insert la mesaje in tabel
{
 int id = getID(username);
 int id2 = getID(user_destination);

 char sql[300];
 
 char buf[200];
 strcpy(buf, message_from_client);


 sprintf( sql, "INSERT into Chat (id_sursa, id_destinatie, mesaj, seen) values (%d, %d, '%s', %d);", id, id2, buf, 0) ;
 rc = sqlite3_exec(db, sql, callback, 0, &ErrMsg);

 if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);

        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
  } 

  int descriptor = getDescriptor(id2); //unde scriu

  if(descriptor != 0)
  {
     if (write (descriptor, &buf, sizeof(buf)) == -1)
		{
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
  }
 }


void make_seen_1(char* username)
{
 char sql[100];  //id_destinatie = username-ul 
 sprintf( sql, "UPDATE Chat set seen=%d where id_destinatie=%d;",1, getID(username));
 rc = sqlite3_exec(db, sql, callback, 0, &ErrMsg);

    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);

        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
    } 
}

void history(char* message_to_client, char* username)
{ //afisez toate mesajele 

 if(islogged(username) == 0)
 strcpy(message_to_client, "Te rugam sa te loghezi mai intai!");
   
 else
 {
 
   char sql[500];
  char msg[20000]="";

  sprintf( sql, 
  "SELECT id_Mesaj , username, mesaj FROM Chat INNER JOIN Messenger ON id_sursa = id where id_destinatie=%d ;", getID(username));
   
   int rc = sqlite3_exec(db, sql, callback4, &msg, &ErrMsg);
   
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);

        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
  } 
   

        strcpy(message_to_client, msg);
        //seen =1;
        make_seen_1(username);
 }
}

void see_offline_messages(char* message_to_client, char* username)
{
  char sql[500];
  char msg[20000]="";

  sprintf( sql, 
  "SELECT username, mesaj FROM Chat INNER JOIN Messenger ON id_sursa = id where id_destinatie=%d and seen = 0;", getID(username));
   printf("mesaj %s\n", sql);
   int rc = sqlite3_exec(db, sql, callback3, &msg, &ErrMsg);
   
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);
       
        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
  } 
   
   strcpy(message_to_client, msg);

   //seen =1;
   make_seen_1(username);
}

void getMessage (char* message_from_client, char* message_to_client, char* username)
{
  if(islogged(username) == 0)
 strcpy(message_to_client, "Te rugam sa te loghezi mai intai!");
   
 else
 {
  char vectorCuv[5][11];
  char *cuv = strtok(message_from_client, " ");
  int total = 0;

  while(cuv != NULL)
  {
    strcpy(vectorCuv[total], cuv);
    total++;
    cuv = strtok(NULL, " ");
 }
  
  //reply to x ( unde x este id-ul mesajului la care vreau sa raspund in mod explicit) x=vectorCuv[2]
  
  //voi extrage mesajul in functie de id-ul x din tabelul Chat.
  char sql[500];
  char msg[20000]="";

  int nr = atoi(vectorCuv[2]);

  sprintf( sql, 
  "SELECT username, mesaj FROM Chat INNER JOIN Messenger ON id_sursa = id where id_destinatie=%d and id_Mesaj=%d;", getID(username) , nr);
   printf("mesaj %s\n", sql);
   int rc = sqlite3_exec(db, sql, callback3, &msg, &ErrMsg);
   
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", ErrMsg, sql);
       
        sqlite3_free(ErrMsg);
        sqlite3_close(db);
        exit(1);
  } 
   
   sprintf(message_to_client, "REPLY TO %s ", msg );  
 }
}


void delete_my_descriptor(int *am_introdus_comanda_send_before)
{
    *am_introdus_comanda_send_before = 0;
}

int reply(void *arg, char* username, char* user_destination, int *am_introdus_comanda_send_before)
{ 
  int i = 0 ; //nr de utilizatori;
	struct thData td_client; 
	td_client= *((struct thData*)arg);

   
  char message_from_client[1000], message_to_client[1000];
  int descriptor_sursa = td_client.cl;

  // Clean buffers:
    memset(message_from_client,'\0',sizeof(message_from_client));
    memset(message_to_client,'\0',sizeof(message_to_client));
  
  int c_read = read(td_client.cl, &message_from_client, sizeof(message_from_client));
	if (c_read == -1)
    {
      printf("[Thread %d]\n",td_client.idThread);
      printf ("Eroare la read() de la client.\n");
    }
    if(c_read == 0) //cand clientul inchide conexiunea, se trimite EOF = 0.
    {
      printf("Un client a inchis fortat conexiunea.\n");
      return 0;
    }

  printf("SERVERUL A PRIMIT MESAJUL: %s\n", message_from_client);

  if(message_from_client[strlen(message_from_client)-1] == '\n' )
  message_from_client[strlen(message_from_client)-1] = '\0';

  printf("Client-->%s\n", message_from_client);
	printf ("[Thread %d]Mesajul a fost receptionat...%s\n",td_client.idThread, message_from_client);

  if(strstr(message_from_client, "SPATIU") != 0 )
  strcpy(message_to_client, "Ai trimis un mesaj gol"); 
 
  else  if(strstr(message_from_client, "register") != 0 )
  registerrr(message_from_client, message_to_client);  
   
  else if (strstr(message_from_client, "login") != 0 )
    login(message_from_client, message_to_client, descriptor_sursa, username); 

  else if (strstr(message_from_client, "logout") != 0 ) 
   { 
     logout(descriptor_sursa);  
     *am_introdus_comanda_send_before = 0;
     strcpy(username, "");
     strcpy(user_destination, "");

     strcpy(message_to_client, "Delogat cu succes!");
   }

   else if (strstr(message_from_client, "history") != 0 )
   { 
     history(message_to_client, username);
   }

   else if (strstr(message_from_client, "want to reply") != 0 )
   { 
     history(message_to_client, username);
     if(strstr(message_to_client, "intai") == 0)
     strcat(message_to_client," Introdu comanda 'reply to <numar>'.");
   }
 
   else if (strstr(message_from_client, "reply to") != 0 )
   { getMessage (message_from_client,message_to_client, username);
     
   }
   else if (strstr(message_from_client, "mesaje_offline") != 0 )
   see_offline_messages(message_to_client, username);

   else if (strstr(message_from_client, "menu") != 0 )
   delete_my_descriptor(am_introdus_comanda_send_before);

   else if (strstr(message_from_client, "send message to") != 0)
    {
      send_message_to(message_from_client, message_to_client, username, user_destination, am_introdus_comanda_send_before); 
    }

  else{ 
       if( *am_introdus_comanda_send_before == 1 )
          {
            
            during_conversation(message_from_client, message_to_client, username, user_destination);
          }
       else
       strcpy(message_to_client, "Te rugam sa introduci comanda send message to <username>");
       }
  
		      /*pregatim mesajul de raspuns */
  printf("Utilizatorul %s cu id-ul %d are thread-ul %d\n", username, getID(username), td_client.idThread);
  printf("[Server]");
	printf("[Thread %d] Trimitem mesajul inapoi...%s\n",td_client.idThread, message_to_client);

		      /* returnam mesajul clientului */

	if (strlen(message_to_client) != 0)
    if(write (td_client.cl, &message_to_client, sizeof(message_to_client)) == -1)
		{
		  perror ("[Thread]Eroare la write() catre client.\n");
		}

    return 1;
}

void* server_thread( void * arg) //in arg* poate sa fie orice si server_thread poate returna orice
{
    struct thData MyThread;
		MyThread = *((struct thData*)arg);	
    
    //lucrez cu tdl

    printf ("[thread %d] Asteptam mesajul...\n", MyThread.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());

    char username[100], user_destination[100];
    int am_introdus_comanda_send_before = 0;

    while(1){

		int raspuns_status = reply((struct thData*)arg, username, user_destination, &am_introdus_comanda_send_before);
    //functie si verific comanda si in functie de comanda, apelez o alta comanda.
    if(raspuns_status == 0)
      break;
    }
    
		/* am terminat cu acest client, inchidem conexiunea */


     am_introdus_comanda_send_before = 0;
     strcpy(username, "");
     strcpy(user_destination, "");

    logout(MyThread.cl);
		close ((intptr_t)arg);
		return(NULL);	
  
}

int main()
{ 
  
    rc = sqlite3_open("test.db", &db); 

    char *sql="UPDATE Messenger set isLogged = 0, descriptor = 0;";
    rc = sqlite3_exec(db, sql, callback, 0, &ErrMsg);

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int i=0;

    int server_socket;

    pthread_t th[100]; //identificatorii thread-urilor care se vor crea;

    /* utilizarea optiunii SO_REUSEADDR */
    int on=1;
    setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
   
    if ((server_socket = socket(AF_INET, SOCK_STREAM,  0)) == -1)
   {
    perror("Eroare la creare socket [server]\n");
    exit(1);
   }
    printf("Server socket a fost creat cu succes!\n");

    server_addr.sin_family = AF_INET;	
    server_addr.sin_addr.s_addr =  htonl (INADDR_ANY);
    server_addr.sin_port = htons (2001);

    int optval = 1;
    setsockopt (server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    //bind() --> atasam socketul
    if (bind( server_socket, (struct sockaddr * ) &server_addr, sizeof(server_addr)) == -1)
    {
    perror("Eroare la bind[server]\n");
    exit(1);
    }

  //listen() --> asteptam conexiuni
   if(listen(server_socket, 100) == -1)
  { 
    perror("Eroare la listen[server]\n");
    exit(1);
  } 

  while(1)
  {
    int client_socket;
    thData * td; //parametru functia executata de thread    
    int length = sizeof(client_addr);

    if ( (client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &length )) == -1)
    {
    perror("Eroare la acceptarea conexiunii de la client[server]\n");
    exit(1);
    }

    	td=(struct thData*)malloc(sizeof(struct thData));	
	    td->idThread=i++;
    	td->cl=client_socket;
  

	  pthread_create(&th[i], NULL, &server_thread, td);	      
				
	}

    sqlite3_close(db);
    return 0;
}


