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

/*------------------------PUBLIC FUNCTION----------------------------------*/
void log_print(char* msg) 
{
	printf("%s\n",msg);
}
//---------------------------------------------------------------------------
//End of file lib.c: