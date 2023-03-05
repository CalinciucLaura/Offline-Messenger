#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

#define errno;
int PORT ;
  int sock;

char message_to_server[500], message_from_server[500];


void menu()
{
  printf("Salut!\n Bine ai venit la Offline Messenger!\n");
  printf("Introdu o comanda\n");
  printf("register <username> <parola>\n");
  printf("login <username> <parola>\n");
  printf("logout\n");
  printf("send message to <username> \n");
  printf("want to reply \n"); 
  printf("exit\n");
  printf("history\n");
  printf("Scrie 'menu' ca sa te intorci inapoi la pagina principala\n");
  printf("Introduceti o comanda:\n");
}


int spatii_endline(char* message_to_server)
{
  for(int i=0; i<strlen(message_to_server);i++)
  if(message_to_server[i] != ' ' && message_to_server[i] != '\n' && message_to_server[i] != 0)
  {//printf("Am gasit litera!, CUV=%s\n", message_to_server);
  return 1; //daca exista caracter diferit de spatiu sau newline
  }

  //printf("DOAR SPATII!, CUV=%s\n", message_to_server);
  return 0;
}


//intra in main, face read, apoi intra in myThread, face citirea
void *myThreadFun(void* arg)
{

		pthread_detach(pthread_self());

    printf("Client-->");
    while(1)
    {   
        strcpy(message_to_server,"");
        fgets(message_to_server,100,stdin); //3 
   
        if(spatii_endline(message_to_server) == 0)
            strcpy(message_to_server,"SPATIU");

        else if(strstr(message_to_server, "exit") != 0)
        {
            printf("Iesire din aplicatie\n");
            exit(0);
        }
        else if (strstr(message_to_server, "menu") != 0)
       {
        printf("\n");
        menu();
        printf("\n");
        }
  
       
        // trimiterea mesajului la server 
        if (write(sock, &message_to_server, sizeof(message_to_server) ) == -1)  //trimite mesajul, se intoarce in while //5
            {
            perror ("[client]Eroare la trimiterea mesajului spre server.\n");
            exit(1);
            }
        
    }
}

int main (int argc, char *argv[])
{
  menu();
  pthread_t thread_id;
  
  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  PORT = atoi (argv[2]);
  if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  struct sockaddr_in server_addr;	
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(argv[1]);
  server_addr.sin_port = htons (PORT);
  
  /* ne conectam la server */
  if (connect (sock, (struct sockaddr *) &server_addr, sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      exit(1);
    }
  
    pthread_create(&thread_id, NULL, &myThreadFun, NULL);
 
 while(1){
  
  memset(message_from_server,'\0',sizeof(message_from_server));
  
    /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
  
    int c_read= read (sock, &message_from_server, sizeof(message_from_server));
    if (c_read == -1)
    {
      perror ("[client]Eroare la primirea mesajului de la server.\n");
      exit(1);
    }
    
    if (c_read == 0)
    {
      printf("Serverul a inchis fortat conexiunea\n");
      return 0;
    }
    
    if(strlen(message_from_server) == 0){
     
      printf("Client--> ");
      fflush(stdout);
    }
    else{ 
     printf("Raspuns--> %s \n", message_from_server, strlen(message_from_server));
    }
}
  close (sock);
  return 0;
}