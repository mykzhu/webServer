//---------------------------------------------------------------------------
/***************************************************************************
FILE........: lib.h
AUTHOR......: Zhuravel Mykola
DESCRIPTION.: File contents main import and public declaration
FUNCTION....: 
NOTES.......: 
COPYRIGHT...: All rights reserved by Zuravel Mykola
HYSTORY.....: DATE            COMMENT
              ----------------------------------------------
              23-02-2014      Created-Zhuravel
*****************************************************************************/
#ifndef LIB_FILE
#define LIB_FILE

/*---------------------PUBLIC DECLARATION----------------------------------*/

#define BACKLOG 10

#define _portDigits 6
#define _addressLength 16
#define _dirLength 32
#define _confStringLength 128

#define SERVER "Server: SimpleWeb\n"
#define CONTENT "Content-Type: text/html\r\n"
#define CONTENT2 "Content-Type: image/jpeg\r\n"
//---------------------------------------------------------------------------

struct parameters {
  char listenPort[_portDigits]; 
  char listenIpAddress[_addressLength]; // IP for client connections
  char workDir[_dirLength];
};

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
        
#endif
//---------------------------------------------------------------------------
//End of file lib.h