     #include <stdio.h>
     #include <stdlib.h>
     #include <string.h>



         

     typedef struct Config  Config;
      
       struct Config {

           char video_url[256]; 
	   char rtsp_mount[256];
	   int rstp_port;
	   char display_text[256];

       };
        

         Config read_config(const char *file_config)
	  {
               Config conf;

	       FILE * file = fopen(file_config,"r");
	       if (!file)
		      {
                         perror("error open file");
			 exit(-1);

		      }
                    

	  fgets(conf.video_url,sizeof(conf.video_url),file)	 ;
		 conf.video_url[strcspn(conf.video_url,"\n")]=0;


 fgets(conf.rtsp_mount,sizeof(conf.rtsp_mount),file);
  conf.rtsp_mount[strcspn(conf.rtsp_mount,"\n")]=0;
      fscanf(file,"%d\n",&conf.rstp_port);

       fgets(conf.display_text,sizeof(conf.display_text),file);
      conf.display_text[strcspn(conf.display_text,"\n")]=0;
       fclose (file);
       return conf;      

	  }


    int main (int argc , const char *argv[] ) 



{
       

	  int b;
	   char * se = "";
	   
           if(argc >1)
		    {

                       char filename[1024];
		       strcpy(filename,argv[1]);

		       Config cnf= read_config(filename);
		       printf("%s %s", cnf.video_url,cnf.display_text);
		           b= 1;
			   return b;
		    }
	   else {
               perror("number argument insuffisant ");
	       perror("usage ./cnf <filename> ");
	       b=-1; 
	       return b;

	   }
	     

}
