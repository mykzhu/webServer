//---------------------------------------------------------------------------
/***************************************************************************
FILE........: main.c
AUTHOR......: Zhuravel Mykola
DESCRIPTION.: File contents main function of simple web server main.c
FUNCTION....: - int main(int argc, void **argv)
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
#include <sys/socket.h> /*  socket definitions        */
#include <errno.h> 
#include <string.h> 
/*---------------------PUBLIC DECLARATION----------------------------------*/
#define PORT "8080" // the port users will be connecting to

#define BACKLOG 10  // how many pending connections queue will hold

//---------------------------------------------------------------------------

/*------------------------PUBLIC FUNCTION----------------------------------*/
/****************************************************************************
FUNCTION.......: main
DESCRIPTION....: Function which open library of functions and 
                 create process which will handle connection.
ARGUMENTS......:
RETURNS........: 
****************************************************************************/
int main(int argc, void **argv)
{
  char *error; 
  char clientIpAddress[16]; 
  char msg[255];
  memset(clientIpAddress, '\0', 16);
  void *libHandle;  

  int iret_value, 
      isockId,
      new_id, 
      yes=1, 
      flag = 1,
      clientStructSize =0,
      pid;

  struct addrinfo *hints, *servinfo, *struct_point;
  struct sockaddr_in *client;
  socklen_t sin_size;

  void (*log_print) (char*); 
  int (*connection) (int);
 
  libHandle = dlopen("./lib.so", RTLD_NOW); 

  if (NULL == libHandle) 
  { 
    fprintf(stderr, "Error during opening lib %s\n", dlerror()); 
    exit(EXIT_FAILURE); 
  } 

  *(void **) (&log_print) = dlsym(libHandle, "log_print"); 
  if ((error = dlerror()) != NULL) 
  {
    fprintf(stderr, "Error during call log_print %s\n", error); 
    exit(EXIT_FAILURE); 
  } 

  *(int **) (&connection) = dlsym(libHandle, "connection"); 
  if ((error = dlerror()) != NULL) 
  {
    fprintf(stderr, "Error during call connection %s\n", error); 
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
    sprintf(msg, "Error getting addres: %s\n", gai_strerror(iret_value));
    (*log_print)(msg);
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
      sprintf(msg, "Error creadting socket: %s\n", strerror(errno));
      (*log_print)(msg);
      continue;
    }
    if (setsockopt(isockId, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) 
    {
      sprintf(msg, "Error socket options: %s\n", strerror(errno));
      (*log_print)(msg);
      free(hints);
      dlclose(libHandle);
      exit(EXIT_FAILURE);
    }
    if (bind(isockId, struct_point->ai_addr, struct_point->ai_addrlen) == -1) 
    {
      close(isockId);
      sprintf(msg, "Error connecting socket to addres: %s\n", strerror(errno));
      (*log_print)(msg);
      continue;
    }
    break;
  }

  if (struct_point == NULL)  
  {
    sprintf(msg, "server: failed to bind\n");
    (*log_print)(msg);
    free(hints);
    dlclose(libHandle); 
    return 2;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (listen(isockId, BACKLOG) == -1) 
  {
    sprintf(msg, "server: failed to listen\n");
    (*log_print)(msg);
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
      sprintf(msg, "server: failed to accept\n");
      (*log_print)(msg);
      continue;
    }
    else
    {
      inet_ntop(AF_INET, &(client->sin_addr), clientIpAddress, sizeof(clientIpAddress)); 
      sprintf( msg, "Connection from client [%s:%d]\n", clientIpAddress, 
      ntohs(client->sin_port) ); 
      (*log_print)(msg);

      pid = fork();
      if(0 == pid)
      { // this is child's context
        close(isockId);//child doesn't need the listener
        if((*connection)(new_id) == -1)
        {
          free(hints);
          close(new_id);
          (*log_print)("Error in connection function!");
          dlclose(libHandle);
          exit(EXIT_FAILURE); 
        };
        free(hints);
        close(new_id);
        exit(EXIT_SUCCESS);
      }
      else if (0 < pid)
      { // this is parent's context
        sprintf(msg, "New child %d was created.\n",pid);
        (*log_print)(msg);
        close(new_id);
      }
      else
      {
        sprintf(msg, "server: failed to fork\n");
        (*log_print)(msg);
        continue; 
      }
    }
    //flag = 0;
  }

  free(hints);
  (*log_print)("Server was stoped");
  dlclose(libHandle); 
  exit(EXIT_FAILURE); 
}
//---------------------------------------------------------------------------
//End of file main.c: