/*  $Id: pdi2iso.c, 17/01/05 10.52

    Copyright (C) 2005 Salvatore Santagati <salvatore.santagati@gmail.com>   
  
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
    This program is distributed in the hope that it will be useful,       
    but WITHOUT ANY WARRANTY; without even the implied warranty of        
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
    GNU General Public License for more details.                          
                                                                          
    You should have received a copy of the GNU General Public License     
    along with this program; if not, write to the                         
    Free Software Foundation, Inc.,                                       
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.        
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "0.1"
#define DUMP_BIT_PDI 304
#define _FILE_OFFSET_BITS 64

void cuesheets (char *destfilename)
     {
      char destfilecue[1024],destfilebin[1024];
      FILE *fcue;                 
                      
      strcpy(destfilecue,destfilename);  
      strcpy(destfilebin,destfilename);  
                      
      strcpy(destfilecue+strlen(destfilename)-4, ".cue");
      strcpy(destfilebin+strlen(destfilename)-4, ".bin");
      fcue = fopen(destfilecue,"w");                  
                      
      fprintf(fcue,"FILE \"%s\" BINARY\n",destfilebin);
      fprintf(fcue,"TRACK 1 MODE1/2352\n");
      fprintf(fcue,"INDEX 1 00:00:00\n");
                      
      rename(destfilename,destfilebin);     
                                    
      printf("Create Cuesheets : %s\n", destfilecue);
      fclose(fcue);                      
     }

void main_percent(int percent_bar){
    
    int progress_bar,progress_space;
     
     printf("%d%% [:",percent_bar);  
     for(progress_bar=1; progress_bar<=(int)(percent_bar/5); progress_bar++) printf("=");
     printf(">");     
     for(progress_space=0; progress_space<(20-progress_bar); progress_space++) printf(" ");
     printf(":]\r");   
     }    

void usage(){
    printf("pdi2iso v%s by Salvatore Santagati\n", VERSION);
    printf("Web     : http//pdi2iso.berlios.de\n");
    printf("Email   : salvatore.santagati@gmail.com\n");
    printf("Irc     : irc.freenode.net #ignus\n");
    printf("License : released under the GNU GPL v2 or later\n\n");

    printf("Usage :\n");
    printf("pdi2iso [OPTION] [BASENAME.pdi]\n\n");
    printf("OPTION\n");
    printf("\t--cue  Generate cue file\n");
    printf("\t--help display this notice\n\n");
    }

int main( int argc, char **argv )
{
	int   seek_ecc, sector_size,seek_head,sector_data;
	int   cue=0,cue_mode=0,sub=1;
	double size_iso,write_iso;
	long  percent=0;
	long  i, source_length,progressbar;
	char  buf[2448], destfilename[2354];
	FILE  *fdest, *fsource;
    
    const char SYNC_HEADER_64[16] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x3F, 0x3F, 0x3F, 0x7F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x7F, 0x3F, 0x7F };
    const char SYNC_HEADER_16[16] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x82, 0x00, 0x61 };
    const char SYNC_HEADER[16] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x02, 0x00, 0x01 };
    const char ISO_9660[8] = {0x01, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01 , 0x00};
    const char PDI_IMG[16]={0x54, 0x54, 0x41, 0x46, 0x4D, 0x50, 0x56, 0x4F, 0x42, 0x43, 0x44, 0x49, 0x4D, 0x47, 0x01, 0x00};

if (argc < 2) 
    {
    usage();
    exit(EXIT_FAILURE);
    }  
        else     
            {
            for (i=0; i < argc; i++)
            {
            if  (!strcmp(argv[i],"--help"))
                     {
                     usage();
                     exit(EXIT_SUCCESS);
                     }  
                     
            if  (!strcmp(argv[i],"--cue")) cue = 1;    
            }
            
            if ((cue == 1)&&( argc <= 2 ))  
            {
            usage();
            exit(EXIT_FAILURE);
            }                          
            
            if (argc >= (3+cue)) strcpy(destfilename, argv[2+cue]);
               
                   else
                       {
                       strcpy(destfilename, argv[1+cue]);
                       if (strlen(argv[1+cue]) < 5 || strcmp(destfilename+strlen(argv[1+cue])-4, ".pdi"))
                                              strcpy(destfilename+strlen(argv[1+cue]), ".iso");
   	                                                          
                           else strcpy(destfilename+strlen(argv[1+cue])-4, ".iso");
                       }



        if (fsource = fopen(argv[1+cue],"rb"))
        {
        fseek(fsource, 32768, SEEK_CUR);
        fread(buf, sizeof(char), 8, fsource);

                if (memcmp(ISO_9660, buf, 8)) 
                     {
                     fseek(fsource, 0L, SEEK_SET);
                     fdest = fopen(destfilename,"wb"); 
                     fread(buf, sizeof(char), 16, fsource); 
                     
                     if(!memcmp(PDI_IMG, buf,16))
                     {
                     fseek(fsource, 0L, SEEK_END);
                     fseek(fsource, DUMP_BIT_PDI, SEEK_SET);
                     fread(buf, sizeof(char), 16, fsource); 
                     
                     if(!memcmp(SYNC_HEADER_16, buf, 16))   
                     {
                     printf("This release don't support RAW16 and RAW96, scuse me =(\n");
                     exit(EXIT_FAILURE);
                     }
                      
                     else if(!memcmp(SYNC_HEADER, buf, 16))
                      {
                      if (cue == 1){
                                    cue_mode = 1;
                                    /* RAW TO BIN*/
                                    seek_ecc = 0;
                                    sector_size = 2352;
                                    sector_data = 2352;
                                    seek_head = 0;
                                   
									}  
                                     else 
                                     {          
                                     /* RAW */  
                                     seek_ecc = 288;
                                     sector_size = 2352;
                                     sector_data = 2048;
                                     seek_head = 16 ;  
                                    
									}
                      } 
                      
                      else
                          {
                      		/* DATI UTENTE */
                      		seek_ecc = 0;
                      		sector_size = 2048;
                      		sector_data = 2048;
                      		seek_head = 0; 
		      				} 
                                         
                     fseek(fsource, 0L, SEEK_END);
                   	 source_length = (ftell(fsource)-DUMP_BIT_PDI)/sector_size ;
                     size_iso = (int)(source_length  * sector_data);   
                     fseek(fsource, DUMP_BIT_PDI, SEEK_SET);
                     
                      for(i = 0; i < source_length; i++)
                      {  
                        fseek(fsource, seek_head, SEEK_CUR); 
						 
                        fread(buf, sizeof(char), sector_data, fsource);  
						fwrite(buf, sizeof(char),sector_data, fdest);   
						fseek(fsource, seek_ecc, SEEK_CUR);
                        write_iso =(int)( sector_data * i );
                        if (i!=0)percent =(int)(write_iso * 100 / size_iso);                        
                        main_percent(percent);
                      }
					  
                      printf("100%%[:====================:]\n");     
                      if (cue == 1) cuesheets(destfilename);
                         else  printf("Create iso9660 : %s\n",destfilename);

		      			fclose(fdest);
		      			fclose(fsource);
  
                      exit(EXIT_SUCCESS);
                      }
                      
                      printf("Sorry I don't know this format :(\n");
                      fclose(fsource);
                      exit(EXIT_FAILURE);
                     }
                    

                 else printf("This is file iso9660 ;)\n");
                 fclose(fsource);
                 exit(EXIT_SUCCESS);
                  
            }    
            else printf("%s : No such file\n",argv[1+cue]);  
            exit(EXIT_FAILURE);
   }
}
