//---------------------------------------------------------------------------
/***************************************************************************
FILE........: main.c
AUTHOR......: Zhuravel Mykola
DESCRIPTION.: File contents main function of simple web server main.c
FUNCTION....: - 
NOTES.......: 
COPYRIGHT...: All rights reserved by Zuravel Mykola
HYSTORY.....: DATE            COMMENT
              ----------------------------------------------
              14-02-2014      Created-Zhuravel
              15-02-2014      adding simple handling of requests
*****************************************************************************/
/*------------------------IMPORT DECLARATION--------------------------------*/
#include <stdio.h> 
#include <stdlib.h> 
#include <dlfcn.h> 
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h> 
#include <string.h> 
/*---------------------PUBLIC DECLARATION----------------------------------*/
#define PORT "3456" // the port users will be connecting to

#define BACKLOG 10  // how many pending connections queue will hold

//---------------------------------------------------------------------------

/*------------------------PUBLIC FUNCTION----------------------------------*/
int main(int argc, void **argv)
{
  char *error; 
  char clientIpAddress[16]; 
  memset(clientIpAddress, '\0', 16);
  void *libHandle;  

  int iret_value, 
      isockId,
      new_id, 
      yes=1, 
      flag = 1,
      clientStructSize =0;

  struct addrinfo *hints, *servinfo, *struct_point;
  struct sockaddr_in *client;
  socklen_t sin_size;

  void (*log_print) (char*); 
 
  libHandle = dlopen("./lib.so", RTLD_NOW); 

  if (NULL == libHandle) 
  { 
    fprintf(stderr, "%s\n", dlerror()); 
    exit(EXIT_FAILURE); 
  } 

  *(void **) (&log_print) = dlsym(libHandle, "log_print"); 
  if ((error = dlerror()) != NULL) 
  {
    fprintf(stderr, "%s\n", error); 
    exit(EXIT_FAILURE); 
  } 
  
  (*log_print)("Server was started");

  hints = (struct addrinfo*) calloc(1, sizeof(struct addrinfo));
  hints->ai_family = AF_INET;
  hints->ai_socktype = SOCK_STREAM;
  hints->ai_protocol = 6;
  hints->ai_flags = AI_PASSIVE;

  if((iret_value = getaddrinfo(NULL, PORT, hints, &servinfo)) != 0)
  {
    fprintf(stderr, "Error getting addres: %s\n", gai_strerror(iret_value));
    free(hints);
    dlclose(libHandle); 
    exit(EXIT_FAILURE);
  }

  // loop through all the results and bind to the first we can
  for(struct_point = servinfo; struct_point != NULL; struct_point = struct_point->ai_next) 
  {
    if ((isockId = socket(struct_point->ai_family, struct_point->ai_socktype,
         struct_point->ai_protocol)) == -1)
    {
      fprintf(stderr, "Error creadting socket: %s\n", strerror(errno));
      continue;
    }
    if (setsockopt(isockId, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) 
    {
      fprintf(stderr, "Error socket options: %s\n", strerror(errno));
      free(hints);
      dlclose(libHandle);
      exit(EXIT_FAILURE);
    }
    if (bind(isockId, struct_point->ai_addr, struct_point->ai_addrlen) == -1) 
    {
      close(isockId);
      fprintf(stderr, "Error connecting socket to addres: %s\n", strerror(errno));
      continue;
    }
    break;
  }

  if (struct_point == NULL)  
  {
    fprintf(stderr, "server: failed to bind\n");
    free(hints);
    dlclose(libHandle); 
    return 2;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (listen(isockId, BACKLOG) == -1) 
  {
    fprintf(stderr, "server: failed to listen\n");
    free(hints); 
    close(isockId);
    dlclose(libHandle); 
    exit(EXIT_FAILURE);
  }
  
  //TODO: add reap all dead processes

  (*log_print)("Server waiting for connections...");
  while(flag)
  {
    client = (struct sockaddr_in*) calloc(1, sizeof(struct sockaddr_in)); 
    clientStructSize = sizeof(struct sockaddr_in); 

    new_id = accept(isockId, (struct sockaddr *)client, &clientStructSize);
    if (new_id == -1) 
    {
      fprintf(stderr, "server: failed to accept\n");
      continue;
    }
    inet_ntop(AF_INET, &(client->sin_addr), clientIpAddress, sizeof(clientIpAddress)); 
    fprintf( stdout, "Connection from client [%s:%d]\n", clientIpAddress, 
    ntohs(client->sin_port) ); 
    //flag = 0;
  }

  free(hints);
  (*log_print)("Server was stoped");
  dlclose(libHandle); 
  exit(EXIT_SUCCESS); 
}
//---------------------------------------------------------------------------
//End of file main.c: