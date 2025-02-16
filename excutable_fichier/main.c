

    #include  <stdio.h>
    #include  <stdlib.h>
    #include  <stdarg.h>
    #include <time.h>
    #include <gst/gst.h>

   

    int main (int argc , const char *argv[] )

     {

        
         
       gst_init (&argc , &argv) ;
      
      GMainLoop *main_loop =  g_main_loop_new (NULL  ,  FALSE ) ;
      GstElement *pipeline  = gst_pipeline_new("webrtc-pipeline") ;
      GstElement *source  = gst_element_factory_make ("videotestsrc", "source");
      GstElement *slink = gst_element_factory_new ("autovideosink","sink") ;
      
       if(!pipeline || !source   || !slink  {

         g_printerr("Failed to  create  elements\n") ;
         return  -1;
       } 
         
    //   gst_main_loop_run (main_loop) ;

     
    //  gst_deinit() ;

    





    gst_bin_add_many(GST_BIN(pipeline), source, sink, NULL);
    if (gst_element_link(source, sink) != TRUE) {
        g_printerr("Failed to link elements\n");
        gst_object_unref(pipeline);
        return -1;
    }
    
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    
    g_main_loop_run(main_loop);
    
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(main_loop);
    gst_deinit();

  

    
  	 return  0;



    } 


       
    
