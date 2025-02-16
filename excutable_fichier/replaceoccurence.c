
#include <stdio.h>
#include  <stdlib.h>
#include <time.h>
#include  <string.h>
#include  <math.h>
#include  <stdarg.h>
#define MAX_COMMAND_LENGTH 2560  <<  2 
#define MAX_OUTPUT_LENGTH 1024 << 2 
#define MAX_BUFFER_INPUT 2048 << 2


int excuteCommand ( const char  *command  ,  char *output  , int handle  )



{
   
    printf("%d %d %d " , MAX_COMMAND_LENGTH ,  MAX_OUTPUT_LENGTH, MAX_BUFFER_INPUT  ) ;
   
   if (handle == -1 )  return  -1 ;

   FILE *fp ; 
   
    char  buffer  [MAX_BUFFER_INPUT] ;


     fp  = popen (command  , "r" ) ;

      if (!fp )  { 
          perror ("error  open prompt " ) ;
	  return  -1;

      }

      output [0] = '\0' ; 

      while (fgets (buffer  ,sizeof (buffer  )  , fp ) !=NULL ) {

        strncat  (output  , buffer , MAX_OUTPUT_LENGTH  - strlen (output ) -1 );


   

      }

      if(pclose (fp) == -1 ) {
        perror ("failed close prompt  ") ;
	return  -1 ;
      
      }


      return  0 ; 

  
      

 }




int main (int argc , const   char  *argv[] ) 



{


                      
	  char command[MAX_COMMAND_LENGTH] ;
	  char output  [MAX_OUTPUT_LENGTH] ;

                   
	  while(1){
          
	   printf ("> ") ;
	   if(fgets (command ,  sizeof (command ) ,  stdin ) == NULL )

	   {

                  perror ("fgets error " ) ;
		  break ;
	   }


	   command [strcspn(command , "\n")]  = '\0' ;
	   if(strcmp  (command , "exit " ) == 0 ) 
		    {

                          printf("exit ok  .... \n" ) ;
			  break ; 

		    }

               if (excuteCommand (command  ,output , 0 ) == 0 ) {
                   
		   time_t time_ =time (NULL ) ;
		   char mytime   [1024 ] = {"0"} ; 
		   sprintf(mytime,"%s", asctime ( localtime (&time_ ))) ;

                   FILE *out_  =NULL  ; 
		  out_ =  fopen("commandeoutput.txt" ,  "a+" ) ;
                    if (out_ !=NULL)
                       if(strcmp (output  , "" ) !=0 )
		   fprintf(out_,"time ; %s , output :   %s" , mytime ,  output ) ;
		    printf("%d "  ,  strcmp(output, "" )) ;
		  printf(" time_excute  : %s ,  command output  %s" , mytime ,output );
                    fclose (out_ ) ;
		    
	       }else printf("error  excution"  );
                  
	  } 


	 return EXIT_SUCCESS ;

}






