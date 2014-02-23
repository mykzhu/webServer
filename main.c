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
#include <unistd.h> 
#include "lib.h"

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
  char *error, *variable, *value; 
  char clientIpAddress[16]; 
  char *buf = (char*) calloc(1, _confStringLength);
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

  FILE *file;

  struct addrinfo *hints, *servinfo, *struct_point;
  struct sockaddr_in *client;
  socklen_t sin_size;

  struct parameters *params = (struct parameters *) calloc(1, sizeof(struct parameters));
  //params->listenIpAddress = (char*) calloc(1, _addressLength);
  //params->workDir = (char*) calloc(1, _dirLength);

  void (*log_print) (char*); 
  int (*connection) (int, struct parameters *);
 
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
  //------------------------------------------------------------------
  // reading config file
  //------------------------------------------------------------------
  file = fopen("./config", "r");
  if (NULL != file)
  {
    (*log_print)("Reading config file");
     while (NULL != fgets(buf, _confStringLength, file)) 
     {
        if ('\n' == buf[strlen(buf)-1]) 
        {
          buf[strlen(buf)-1] = '\0';
        }
        if (('#' == buf[0]) || ('=' == buf[0])) 
        {
          continue;
        }
        if ((2 < strlen(buf)) && ('=' != buf[strlen(buf)-1])) 
        {
          if (NULL != strchr(buf, '=')) 
          {
            variable = strtok(buf, "=");
            value = strtok(NULL, "=");

            if (0 == strncmp("port", variable, 4)) 
            {
              strncpy(params->listenPort, (value), strlen(value));
            }
            else if (0 == strncmp("address", variable, 7)) 
            {
              strncpy(params->listenIpAddress, value, strlen(value));
            }
            else if (0 == strncmp("workDir", variable, 7)) 
            {
              strncpy(params->workDir, value, strlen(value));
            }
          }
        }
      };

    sprintf(msg, " port = [%s]\n address = [%s]\n work directory = [%s]\n", 
        params->listenPort,
        params->listenIpAddress,
        params->workDir
    );
    fclose(file);
    (*log_print)(msg);
  }
  else
  {
    (*log_print)("Culd'n open config file");
    strncpy(params->listenPort, "8080", 4);
    strncpy(params->listenIpAddress, "127.0.0.1", 9);
    strncpy(params->workDir,"site/",5);

    sprintf(msg, " port = [%s]\n address = [%s]\n work directory = [%s]\n", 
        params->listenPort,
        params->listenIpAddress,
        params->workDir
    );
    (*log_print)(msg);
  }
  //------------------------------------------------------------------

  (*log_print)("Server was started");

  hints = (struct addrinfo*) calloc(1, sizeof(struct addrinfo));
  hints->ai_family = AF_INET;
  hints->ai_socktype = SOCK_STREAM;
  hints->ai_protocol = 6;
  hints->ai_flags = AI_PASSIVE;

  if((iret_value = getaddrinfo(params->listenIpAddress, params->listenPort, 
                               hints, &servinfo)) != 0)
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
        if((*connection)(new_id, params) == -1)
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