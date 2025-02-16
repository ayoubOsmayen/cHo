  




      #include  <stdio.h>

      #include <stdlib.h>

     

      #include  <string.h>




      #define MAX_LEN_DATA 1024 >> 3
        typedef struct Data Data   ;

        typedef  struct Node Node  ; 
	typedef int   INTEGER  ;

	 enum ALPHA  { OK , ERROR ,  STATUS  };


        struct  Data {

        char  video_url [ 1024] ; 
	char mount_url [256 ];
	int vl ; 
	char _t  ;

       } ;

      typedef struct Init_Data Init_Data  ;

        struct Init_Data  {

        int val ;
	int status  ;
      };

       struct Node  {
	   int _st ;
	   int node_id ;
	   Data data ;
           Init_Data  ;
	   Node *next , *previous ;



       };
       int _checkData (const char  *data  ,  const char *filename )  ;


         void insertNode (  Node  **node  ,  Data _data , Init_Data  ,  int check , int handle   ) ;





     int main (int argc ,  const char *argv[] ) 


     {


           char  *user  [1024]  ; 


            int m = 2 >> 3 ;
            int o  =  5 << 3 ; //     0000 0101        0010   1000  = 8 + 32  = 40  
            printf("m = %d o = %d" ,  m,o ) ;




          return  1 ; 



    }







