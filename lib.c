//---------------------------------------------------------------------------
/***************************************************************************
FILE........: lib.c
AUTHOR......: Zhuravel Mykola
DESCRIPTION.: File contents main function for simple web server main.c
FUNCTION....: - void log_print(char* msg)
NOTES.......: 
COPYRIGHT...: All rights reserved by Zuravel Mykola
HYSTORY.....: DATE            COMMENT
              ----------------------------------------------
              14-02-2014      Created-Zhuravel
              14-02-2014      Added log_print-Zuravel
*****************************************************************************/
/*------------------------IMPORT DECLARATION--------------------------------*/
#include <stdio.h> 
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>        /*  For select()  */
#include <unistd.h>          /*  for ssize_t data type  */
#include <errno.h> 
#include <fcntl.h>
#include <time.h>

/*---------------------PUBLIC DECLARATION----------------------------------*/
#define SERVER "Server: SimpleWeb\n"
#define CONTENT "Content-Type: text/html\r\n"
#define CONTENT2 "Content-Type: image/jpeg\r\n"



struct {
        char *ext;
        char *filetype;
} extensions [] = {
        {"gif", "image/gif" },  
        {"jpg", "image/jpeg"}, 
        {"jpeg","image/jpeg"},
        {"png", "image/png" },  
        {"zip", "image/zip" },  
        {"gz",  "image/gz"  },  
        {"tar", "image/tar" },  
        {"htm", "text/html" },  
        {"html","text/html" },  
        {"css", "text/css"  },
        {0,0} };

/*------------------------PUBLIC FUNCTION----------------------------------*/
/****************************************************************************
FUNCTION.......: get_time
DESCRIPTION....: Function which returns current time with us or without.
ARGUMENTS......: - msg  -string which will contain time value
                 - flag - if eq. 1-time with us 
RETURNS........: None
****************************************************************************/
void get_time(char *msg, short flag)
{
  time_t now;
  struct tm *tm;
  struct timeval  tv;

  now = time(0);
  if ((tm = localtime (&now)) == NULL) 
  {
    sprintf(msg,"Error extracting time stuff");
  }
  else
  {
    if(flag == 1)
    {
      gettimeofday(&tv, NULL);
      sprintf (msg,"[%04d-%02d-%02d %02d:%02d:%02d.%06u]",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec,(int)tv.tv_usec);
    }
    else
    {
      sprintf (msg,"%04d-%02d-%02d %02d:%02d:%02d",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
  }
}

/****************************************************************************
FUNCTION.......: log_print
DESCRIPTION....: Function which writes log messages to file.
ARGUMENTS......: - msg  -string which will contain log message. 
RETURNS........: None
****************************************************************************/
void log_print(char* msg) 
{
	int fd;
  char time_msg[40];

	if((fd = open("nweb.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0)
	{
    get_time(time_msg, 1);
		(void)write(fd,time_msg,strlen(time_msg));
		(void)write(fd,msg,strlen(msg)); 
		(void)write(fd,"\n",1);      
		(void)close(fd);
	}
}

/****************************************************************************
FUNCTION.......: notGet
DESCRIPTION....: Function which sends HTTP response 501 if type of request 
                 is not GET
ARGUMENTS......: - sock - socket ID
RETURNS........: None
****************************************************************************/
void notGet(int sock)
{  
	char buffer[1024];
	   
	//Send HTTP Response line by line
  strcpy(buffer, "HTTP/1.0 501 Method Not Implemented\r\n");
  write(sock, buffer, strlen(buffer));
	strcpy(buffer, SERVER);
  write(sock, buffer, strlen(buffer));
	strcpy(buffer, CONTENT);
	write(sock, buffer, strlen(buffer));
	strcpy(buffer, "\r\n");
	write(sock, buffer, strlen(buffer));
	strcpy(buffer, "<html>\n<head>\n<title>Method Not Implemented</title>\n</head>\r\n");
	write(sock, buffer, strlen(buffer));
	strcpy(buffer, "<body>\n<p>501 HTTP request method not supported.</p>\n</body>\n</html>\r\n");
	write(sock, buffer, strlen(buffer));
}

/****************************************************************************
FUNCTION.......: notFound
DESCRIPTION....: Function which sends HTTP response 404 if requested page is not found
ARGUMENTS......: - sock - socket ID
RETURNS........: None
****************************************************************************/
void notFound(int sock)
 {  
  char buffer[1024]; 
  long ret;
  int file_fd;

  //Send HTTP Response line by line
  strcpy(buffer, "HTTP/1.0 404 Not Found\r\n");
  write(sock, buffer, strlen(buffer));
  strcpy(buffer, SERVER);
  write(sock, buffer, strlen(buffer));
  if(( file_fd = open("source/404.jpg",O_RDONLY)) == -1) 
  {
    log_print("failed to open 404 file");
    strcpy(buffer, CONTENT);
    write(sock, buffer, strlen(buffer));
    strcpy(buffer, "\r\n");
    write(sock, buffer, strlen(buffer));
    strcpy(buffer, "<html>\n<head>\n<title>Not Found</title>\n</head>\r\n");
    write(sock, buffer, strlen(buffer));
    strcpy(buffer, "<body>\n<p>404 Request file not found.</p>\n</body>\n</html>\r\n");
    write(sock, buffer, strlen(buffer));
  }
  else
  {
    strcpy(buffer, CONTENT2);
    write(sock, buffer, strlen(buffer));
    strcpy(buffer, "\r\n");
    write(sock, buffer, strlen(buffer));
    while ( (ret = read(file_fd, buffer, 1024)) > 0 ) 
    {
      (void)write(sock,buffer,ret);
    }
  }
}

/****************************************************************************
FUNCTION.......: connection
DESCRIPTION....: Function which receives HTTP requests, parces them and sends
                 requested file or responce with error messages.
ARGUMENTS......: - new_socket - socket ID
RETURNS........: None
****************************************************************************/
int connection(int new_socket)
{
	   int bufsize = 1024,
		i,j,buflen, len, file_fd;    
   	char *buffer = malloc(bufsize);
   	char * fstr; 
   	long ret;
   	char path[] = "site/"; 

   	if(recv(new_socket, buffer, bufsize, 0) == -1)
   	{
   		fprintf(stderr, "Error read socket: %s\n", strerror(errno));
   		return -1;
   	}
   	else
   	{
      log_print(buffer);

    	if(strncmp(buffer,"GET ",4)&& strncmp(buffer,"get ",4))
    	{
			  log_print("Only simple GET operation supported");
			  notGet(new_socket);
			  return 0;
		  }
		  else
		  {
			  for(i=4;i<bufsize;i++) 
			  { /* null terminate after the second space to ignore extra stuff */
				  if(buffer[i] == ' ') 
				  {
            buffer[i] = 0;
            break;
          }
	      }

	      for(j=0;j<i-1;j++)
				  if(buffer[j] == '.' && buffer[j+1] == '.')
				  {
            log_print("Parent directory (..) path names not supported");
            return 0;
          }

          if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) )
            (void)strcpy(buffer,"GET /site/index.html");
          else
          {
            for (i=strlen(buffer);i>=0;i--)
            {
              if (i<5)
            	{
            	  buffer[i+strlen(path)] = path[i];
            	}
            	else
            	{
            		buffer[i+strlen(path)] = buffer[i];
            	}
            }
          }

        	buflen=strlen(buffer);
        	fstr = (char *)0;

        	for(i=0;extensions[i].ext != 0;i++) 
        	{
            len = strlen(extensions[i].ext);
            if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) 
            {
              fstr =extensions[i].filetype;
              break;
            }
        	}

        	if(fstr == 0) 
        	{
        		log_print("file extension type not supported");
        		return 0;
        	}

        	if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) 
        	{
        	 	log_print("failed to open file");
        	 	notFound(new_socket);
        	}
          else
          {
            (void)sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n",fstr);
            (void)write(new_socket,buffer,strlen(buffer));
            while ( (ret = read(file_fd, buffer, bufsize)) > 0 ) 
            {
              (void)write(new_socket,buffer,ret);
            }
          }
          #ifdef LINUX
            sleep(1);       /* to allow socket to drain */
          #endif
		    }
      }  
      return 0;
}
//---------------------------------------------------------------------------
//End of file lib.c: