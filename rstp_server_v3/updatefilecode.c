
      #include<stdio.h>
      #include<stdlib.h>
      #include<string.h>

       

       int main( int argc , const char *argv[])
	 {

             if (argc >1 )
	     {
                char *filename= "file_code.txt";
		FILE * f= fopen(filename,"w+");
		fprintf(f,"%s",argv[1]);

		fclose(f);

	     }
            else perror("usage : ./updatefilecode <val>");
           return 1;
         }
       
