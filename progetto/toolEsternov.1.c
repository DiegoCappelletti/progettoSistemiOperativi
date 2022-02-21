/*
 * tool esterno 
 * 
 */


//LIBRERIE
//funzioni di libreria standard
#include <stdlib.h>
#include <stdio.h>
//gestione stringhe
#include <string.h>
//gestione fork e exec
#include <unistd.h>
//gestione pipe
#include <sys/types.h>
#include <fcntl.h>
//gestione segnali
#include <signal.h>
//gestione tempo sistema
#include <sys/stat.h>
#include <time.h>
//gestione semafori
#include <semaphore.h>

void separa(char* cmd, char* comando, char* parametro1, char* parametro2, char* parametro3);

void handlerEsterno(int signum);


int main(int argc, char **argv)
{
	//queste variabili contengono un immagine dei dispositivi connessi al sistema
	int numeroDispositivi=0; //numero dei dispositivi nel sistema inizialmente a 0
	int *vettorePid=(pid_t *)malloc(numeroDispositivi*sizeof(pid_t));//contiene il pid del dispositivo
	int *vettoreId=(int *)malloc(numeroDispositivi*sizeof(int));//contiene l'id del dispositivo
	char *tipologia=(int *)malloc(numeroDispositivi*sizeof(char)); //contiene la tipologia di dispositivo
	
	
	//apertura fifo per ottenere pid e id e tipologia dei dispositivi
	char* fifoIdEsterno="./fifoIdEsterno";
	int fdFifoIdEsterno;//quando apri la fifo la metti qui
	fdFifoIdEsterno=open(fifoIdEsterno, O_RDONLY);
	fcntl(fdFifoIdEsterno, F_SETFL, O_NONBLOCK);  
	if(fdFifoIdEsterno==-1)
    {
		printf("ERRORE APERTURA FIFO ID ESTERNO DA PARTE DI TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE IL SUO UTILIZZO \n");
		perror("");
		getchar();
			
	 }
	  
	
	//creazione e apertura fifo per la diffusione comandi esterni ai dispositivi
	char* fifoTool="./fifoTool";
	int fdFifoTool;
	if(mkfifo(fifoTool, 0666)==-1)
	{
		printf("ERRORE CREAZIONE FIFO TOOL ESTERNO IN TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE CON I DISPOSITIVI DAL TOOL ESTERNO  \n");
	    perror("");
	    getchar();
	}
	fdFifoTool=open(fifoTool, O_RDWR);

	if(fdFifoTool==-1)
    {
		printf("ERRORE APERTURA FIFO TOOL ESTERNO DA PARTE DI TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE IL SUO UTILIZZO \n");
	    perror("");
		getchar();
			
	}
	
	
	//come per la centralina cmd contiene il comando inserito dall'utente
	//i suoi componenti vengono separati dalla funzione separa nei seguenti sottovettori
    //cmd
    char cmd[100];
    //contiene il comando vero e proprio
    char comando[100];
    //contiene il primo parametro del comando
    char parametro1[100];
    //contiene il secondo parametro del comando
    char parametro2[100];
    //contiene il terzo parametro del comando 
    char parametro3[100];
	


    int ext=0;//variabile che indica se uscire dal ciclo di lettura dei comandi testuali al  tool esterno

    do//il tool esterno esegue cicli di lettura
    {
	   

		//variabile che memorizza un carattere letto dalla fifo che passa le info dei dispositivi
		char carattere[1];
		
		while(read(fdFifoIdEsterno, carattere, 1)!=-1)//legge finche si può
		{
			char comand=carattere[0]; //leggo il comando . se 0 aggiungo se 1 tolgo.
					    
		    read(fdFifoIdEsterno, carattere, 1);//leggo la X
		
		    
		    read(fdFifoIdEsterno, carattere, 1);//leggo il tipo di dispositivo si veda sotto per una loro legenda
		    char tipo;
		    tipo=carattere[0];
		   
		    
		    read(fdFifoIdEsterno, carattere, 1);//leggo la X
		   
		    
		    read(fdFifoIdEsterno, carattere, 1);//leggo l'id del dispositivo
		    char id[4];
		    int limite=1;//limite ci aiuta a capire la lunghezza dell'id
		    id[0]=carattere[0];
		    
		    read(fdFifoIdEsterno, carattere, 1);
		    if(carattere[0]!='X')
		    {
				id[1]=carattere[0];
				limite=limite+1;
			}
		    
		    read(fdFifoIdEsterno, carattere, 1);
		    if(carattere[0]!='X')
		    {
				id[2]=carattere[0];
				limite=limite+1;
			}
			
		    
		    limite=0;//ripristino limite
			
			read(fdFifoIdEsterno, carattere, 1);//leggo la X
		    		
			//reperisco il pid
			char pid[50];//vettore che contiene il pid in formato stringa
			read(fdFifoIdEsterno, carattere, 1);	
			
			while(carattere[0]!='|')//finche non trovo il carattere pipe leggo un nuovo carattere e lo inserisco nel vettore che contiene il pid
			{
				pid[limite]=carattere[0];
				limite=limite+1;
				read(fdFifoIdEsterno, carattere, 1);
			}
		
				
	
			
			//converto i valori trovati in intero a eccezione del tipo
			int idIntero=atoi(id);
			int pidIntero=atoi(pid);
			
		
				
			if(comand=='0')//bisogna aggiungere il dispositivo
			{
			
			
				numeroDispositivi=numeroDispositivi+1;//incremento il numero di dispositivi nel ssitema
			
		
			    //la meccanica è la stessa della centralina e del processo iniziale per aumentare i vettori dinamici
				int *vettoreId1=(int *)malloc((numeroDispositivi)*sizeof(int));
						   						   
						   
					   
				for(int i=0; i<numeroDispositivi-1; i++)
				{
					 vettoreId1[i]=vettoreId[i];
				}
						   
						   
				free(vettoreId);
					   
				vettoreId=vettoreId1;
				
				vettoreId[numeroDispositivi-1]=idIntero;
				
				int *vettorePid1=(pid_t *)malloc((numeroDispositivi)*sizeof(pid_t));
						   
				for(int i=0; i<numeroDispositivi-1; i++)
				{
					vettorePid1[i]=vettorePid[i];
				}
						   
						   
				free(vettorePid);
						 				
				vettorePid=vettorePid1;
						   
				vettorePid[numeroDispositivi-1]=pidIntero;
						   
				int *tipologia1=(char *)malloc((numeroDispositivi-1)*sizeof(char));
						   
				for(int i=0; i<numeroDispositivi-1; i++)
				{
					tipologia1[i]=tipologia[i];
				}
						
						   
				free(tipologia);
						   
				tipologia=tipologia1;
						   
				tipologia[numeroDispositivi-1]=tipo;
				
				
				
				
			}else//bisogna togliere il dispositivo dalla lista dei dispositivi del sistema e liberare il suo spazio allocato nel vettore 
			{
				int indiceRimuovere;
				
				
				for(int i=0; i<numeroDispositivi; i++)
				{
					if(vettoreId[i]==idIntero)
					{
						indiceRimuovere=i;
					}
				}
				
				int *vettoreId1=(int *)malloc((numeroDispositivi-1)*sizeof(int));
						   
				for(int i=0; i<indiceRimuovere; i++)
				{
				   vettoreId1[i]=vettoreId[i];
				}
				for(int i=(indiceRimuovere+1); i<numeroDispositivi; i++)
			    {
					vettoreId1[i-1]=vettoreId[i];
				}
						   
				free(vettoreId);
						   
				vettoreId=vettoreId1;
				
				
				
				pid_t *vettorePid1=(pid_t *)malloc((numeroDispositivi-1)*sizeof(pid_t));
						   
				for(int i=0; i<indiceRimuovere; i++)
				{
						vettorePid1[i]=vettorePid[i];
				}
				for(int i=(indiceRimuovere+1); i<numeroDispositivi; i++)
				{
						vettorePid1[i-1]=vettorePid[i];
				}
						   
				free(vettorePid);
						   
				vettorePid=vettorePid1;
						   
						   
						   
				char *tipologia1=(char *)malloc((numeroDispositivi-1)*sizeof(char));
						   
				for(int i=0; i<indiceRimuovere; i++)
				{
					tipologia1[i]=tipologia[i];
				}
				for(int i=(indiceRimuovere+1); i<numeroDispositivi; i++)
				{
					tipologia1[i-1]=tipologia[i];
				}
						   
				free(tipologia);
						   
				tipologia=tipologia1;
				
				
				numeroDispositivi=numeroDispositivi-1;
				
				
			}
		}
		
		
		printf("TOOL ESTERNO \n");
    	printf("di seguito sono mostrati la lista dei dispositivi disponibili e i loro id \n");
	    printf("premi invio per ricaricaricare e mostrare nuovi eventuali dispositivi aggiunti\n");
	
	    
	    printf("dispositivi disponibili \n");
	    
	    //ciclo che stampa tutti i dispositivi presenti nel sistema
	    for(int i=0; i<numeroDispositivi; i++)//qui ci sono anche i codici associati alla tipologia di dispositivi
	    {
			char tipo[15];
			
			if(tipologia[i]=='0')
			{
				strcpy(tipo, "bulb");
				
			}else if(tipologia[i]=='1')
			{
				strcpy(tipo, "window");
				
			}else if(tipologia[i]=='2')
			{
				strcpy(tipo, "fridge");
				
			}else if(tipologia[i]=='3')
			{
				strcpy(tipo, "tent");
				
			}else if(tipologia[i]=='4')
			{
				strcpy(tipo, "thermostat");
				
			}else if(tipologia[i]=='5')
			{
				strcpy(tipo, "timer");
				
			}else if(tipologia[i]=='6')
			{
				strcpy(tipo, "hub");
			}
			
			
			printf("id: %d tipologia: %s \n", vettoreId[i], tipo);
			
			
		}
		
		
		printf("inserire info per avere una descrizione dei comandi disponibili. inserire exit per uscire. \n");
		
		
        //leggo dallo stdin
        scanf("%[^\n]s", cmd);
        
       //separo il comando nei suoi costituenti con la funzione separa
       separa(cmd, comando, parametro1, parametro2, parametro3);
       
      //a questo punto individuiamo quale comando eseguire
       
       if(strcmp(comando, "increase_tent")==0)
       {		   
		  
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			  //se il dispositivo non è una tenda non si può eseguire
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="11";
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
		   
		   
	   }else if(strcmp(comando, "decrease_tent")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			  //se il dispositivo non è una tenda non si può eseguire
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="12";
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "switch")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  
				   
				   
				   char comando[]="13";
				   sprintf(comando, "%sX", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, strlen(comando))==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
			   
		   }
			   
	   }else if(strcmp(comando, "set_thermostat_temp")==0)
       {		   
		  
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='4')
			   {
				   
				   	sprintf(comando, "%sX%s", comando, parametro1);
				   char comando[]="40";
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
		   
		   
	   }else if(strcmp(comando, "set_thermostat_extern")==0)
       {		   
		  
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='4')
			   {
				   
				   	sprintf(comando, "%sX%s", comando, parametro1);
				   char comando[]="41";
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
		   
		   
	   }else if(strcmp(comando, "set_bulb_year")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='0')
			   {
				   
				   
				   char comando[]="14";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_bulb_month")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='0')
			   {
				   
				   
				   char comando[]="15";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_bulb_day")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='0')
			   {
				   
				   
				   char comando[]="16";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_bulb_hour")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='0')
			   {
				   
				   
				   char comando[]="17";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_bulb_minute")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='0')
			   {
				   
				   
				   char comando[]="18";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_bulb_second")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='0')
			   {
				   
				   
				   char comando[]="19";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_window_year")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='1')
			   {
				   
				   
				   char comando[]="20";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_window_month")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='1')
			   {
				   
				   
				   char comando[]="21";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_window_day")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='1')
			   {
				   
				   
				   char comando[]="22";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_window_hour")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='1')
			   {
				   
				   
				   char comando[]="23";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_window_minute")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='1')
			   {
				   
				   
				   char comando[]="24";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_window_second")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='1')
			   {
				   
				   
				   char comando[]="25";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_fridge_year")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="27";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_fridge_month")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="28";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_fridge_day")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="29";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_fridge_hour")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="30";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_fridge_minute")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="31";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_fridge_second")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="32";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_fridge_perc")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="34";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_fridge_delay")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="33";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_fridge_temp")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="35";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "insert_food")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="26";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "set_fridge_temp")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='3')
			   {
				   
				   
				   char comando[]="35";
				   sprintf(comando, "%sX%s", comando, parametro1);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "clean_hub_mirroring")==0)
	   {
			   //ricerca id tra quelli disponibili
			  int idIntero=atoi(parametro1);
			  int indice=-1;//contiene l'indice del dispositivo nei vettori memorizzati, inzialmente a -1
			  for(int i=0; i<numeroDispositivi; i++)
			  {
				  if(vettoreId[i]==idIntero)
				  {
					  indice=i; //se trovo l'indice lo salvo
				  }
			  }
			  
			  if(indice=-1)//se indice è a -1 non ho trovato l'id printo un errore se no provo eseguire
			  {
				  printf("id inesistente \n");
			  }else
			  {
			  
			  //controllo se il comando è valido
			   if(tipologia[indice]=='6')
			   {
				   
				   
				   char comando[]="50";
				   sprintf(comando, "%s", comando);
				   
				   if(write(fdFifoTool, comando, 2)==-1)
				   {
					   printf("ERRORE DI SCRITTURA FIFO ESTERNO DA PARTE DEL TOOL ESTERNO POTREBBE ESSER IMPOSSIBILE COMUNICARE COI DISPOSTIVII \n");
					   getchar();
				   }			 
				   
				   
				   if(kill( vettorePid[indice], SIGURG)==-1)//avviso il dispositivo corrispondente di leggere la fifo  
				   {
					   printf("ERRORE INVIO SEGNALE DA TOOL ESTERNO POTREBBE NON ESSER POSSIBILE COMUNICARE COI DISPOSITIVI\n");
					   perror("");
					   getchar();
				
				   } 
				   
				
				   
			   }else
			   {
				   printf("comando non disponibile per questa tipologia di dispositivo \n");
			   }
			   
		   }
			   
	   }else if(strcmp(comando, "info")==0)//identificazione comandi
       {
		   //questi sono tutti i comandi che si devono implementare
		   //a ciascuno di questi è associato numero e codice del comando da scrivere su pipe(segnato di fianco al comando come commento)
		   printf("elenco dei comandi disponibili per ogni tipologia di dispositivo \n");
		   printf("switch <id>  cambia l'interruttore");//13X0 o 13X1
		   printf("\nlampadina \n");
		   printf("set_bulb_year <id> <value>  setta il valore anno del tempo di accensione\n");//14X<valore inserito utente>
		   printf("set_bulb_month <id> <value> setta il valore mese del tempo di accensione\n");//15X<valore inserito utente>
		   printf("set_bulb_day <id> <value> setta il valore giorno del tempo di accensione\n");//16X<valore inserito utente>
		   printf("set_bulb_day <id> <value> setta il valore ore del tempo di accensione\n");//17X<valore inserito utente>
		   printf("set_bulb_minute <id> <value> setta il valore minuti del tempo di accensione\n");//18X<valore inserito utente>
		   printf("set_bulb_second <id> <value> setta il valore secondi del tempo di accensione\n");//19X<valore inserito utente>
		   printf("\nfinestra \n");
		   printf("set_window_year <id> <value>  setta il valore anno del tempo di accensione\n");//20X<valore inserito utente>
		   printf("set_window_month <id> <value> setta il valore mese del tempo di accensione\n");//21X<valore inserito utente>
		   printf("set_window_day <id> <value> setta il valore ore del tempo di accensione\n");//22X<valore inserito utente>
		   printf("set_window_hour <id> <value> setta il valore ore del tempo di accensione\n");//23X<valore inserito utente>
		   printf("set_window_minute <id> <value> setta il valore minuti del tempo di accensione\n");//24X<valore inserito utente>
		   printf("set_window_second <id> <value> setta il valore secondi del tempo di accensione\n");//25X<valore inserito utente>
		   printf("\nfrigorifero \n");
		   printf("insert_food <id> <quantity>  inserisce la quantità di cibo\n"); //26X<valore inserito utente>
		   printf("set_fridge_year <id> <value>  setta il valore anno del tempo di accensione\n");//27X<valore inserito utente> 
		   printf("set_fridge_month <id> <value>  setta il valore mese del tempo di accensione\n");//28X<valore inserito utente>
		   printf("set_fridge_day <id> <value> setta il valore giorno del tempo di accensione\n");//29X<valore inserito utente>
		   printf("set_fridge_hour <id> <value> setta il valore ore del tempo di accensione\n");//30X<valore inserito utente>
		   printf("set_fridge_minute <id> <value> setta il valore minuti del tempo di accensione\n");//31X<valore inserito utente>
		   printf("set_fridge_second <id> <value> setta il valore secondi del tempo di accensione\n");//32X<valore inserito utente>
	 	   printf("set_fridge_delay <id> <value> setta il valore di delay\n");//33X<valore inserito utente>
	 	   printf("set_fridge_perc <id> <value> setta il valore percentuale tempo riempimento\n");//34X<valore inserito utente>
	 	   printf("set_fridge_temp <id> <value> setta il valore temperatura \n");//35X<valore inserito utente>
	 	   printf("\ntenda \n");
		   printf("increase_tent <id>  accende l'interruttore e incrementa tenda\n"); //codice comando scritto su pipe: 11 
		   printf("decrease_tent <id>  accende l'interruttore e decrementa tenda\n");//codice comando scritto su pipe: 12 
		   printf("\ntermostato \n");
           printf("set_thermostat_temp <id> <value> setta il valore temperatura del termostato\n");//40X<valore inserito utente>
           printf("set_thermostat_extern <id> <value> setta il valore di temperatura ambiente del termostato\n");//41X<valore inserito utente>
           printf("\nhub \n");
           printf("clean_hub_mirroring <id>  pulisce il mirroring dell'hub\n");//50


	 
	   }else if(strcmp(comando, "exit")==0)//se exit allora imposto ext a 1 e il prossimo ciclo non sarà eseguito portando alla terminazione del programma
	   {
		   ext=1;
		   
 	   }else //in alternativa il comando non è riconosciuto
	   {
		   printf("comando non riconosciuto \n");
	   }
        
        
      
    
    
      
	
	}while(ext!=1);
	
    unlink(fifoTool);
    unlink(fifoIdEsterno);

	return 0;
}

//implementazione della funzione separa
void separa(char* cmd, char* comando, char* parametro1, char* parametro2, char* parametro3)
{
	
//nota che l implementazione della funzione si ripete uguale su comando parametro 1 parametro 2 e parametro 3
//commento solo la prima parte	
	
//individuo il primo spazio
char* p;
p = strchr (cmd, ' '); // Cerca lo spazio
   
if (!p)//se non lo trovo
{	
  strcpy (comando, cmd);//sposto il contenuto di cmd in comando
}
else
{
   *p = '\0'; // Sostituisco lo spazio col terminatore, separando le parole
    p++; //incremento il puntatore p per puntare alla successiva lettera al terminatore
    strcpy (comando, cmd); //copio la prima parte della stringa di cmd in comando
    strcpy (cmd, p); //sovrascrivo cmd con l'altra sottostringa
}
 
//primo parametro
   
p = strchr (cmd, ' '); // Cerca lo spazio

if (!p)
{
  strcpy(parametro1, cmd);
}
else 
{
   *p = '\0'; // Sostituisco lo spazio col terminatore, separando le parole
    p++;
    strcpy (parametro1, cmd);
    strcpy (cmd, p);
}
   
//secondo parametro
   
p = strchr (cmd, ' '); // Cerca lo spazio

if (!p)
{
   strcpy(parametro2, cmd);
}
else
{
   *p = '\0'; // Sostituisco lo spazio col terminatore, separando le parole
   p++;
   strcpy (parametro2, cmd);
   strcpy (cmd, p);
   strcpy (parametro3, cmd);
}

//verifico se nel secondo parametro è ppresente un to
if (strcmp(parametro2, "to")==0)
{
	//copio il parametro 3 nel 2
	strcpy(parametro2, parametro3);
	//nel parametro tre in prima posizione inserisco il terminatore per ogni evenienza
	parametro3[0]='\0';
}
   
  

}



