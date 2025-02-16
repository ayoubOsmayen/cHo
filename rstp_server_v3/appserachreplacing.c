    #include<stdio.h>
    #include<string.h>
    #include<stdlib.h>
     

     #define MAX_LENGTH_LINE 1024 
     

      #define shifft (x) x>>w


     int replace_all(char *line , const chae *src, const char *repl)
{
      

	  char buffer[MAX_LENGTH_LINE];
          char *pos;
          
           int src_len = strlen(src);
           int repl_len = strlen(repl);
            

            while ( (pos=  strstr( line,src))!=NULL   )

	       {

		        strncat(buffer,line,pos-line);
			strcat(buffer,replace);
			line = pos+src_len;


	       }	       



}
