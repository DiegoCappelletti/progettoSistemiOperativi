/*
 * hub
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



//DICHIARAZIONE FUNZIONI
//handler per il segnale utente  cambia il valore dell'interruttore .
void handlerCambiaInterruttore(int signum);
//gestore per il segnale utente  che permette il continuo dell esecuzione
void handlerLetturaPipe(int signum);
//gestore mirroring
void handlerMirroring(int signum, siginfo_t *siginfo, void *context);
//gestore per il segnale di delete e altri segnali che richiedono un ack
void handlerAck(int signum, siginfo_t *siginfo, void *context);

//VARIABILI GLOBALI
//interruttore
//file descriptor della pipe usata per comunicare il valore a cui si vuole settare un interruttore
int fdPadreInterruttore;
//variabile che rappresenta un interruttore
char interruttore[]="OFF";
//di servizio per l'esecuzione delle operazioni
//variabile intera che indica se connesso
int connected;
//pipe padre per passare informazioni al padre
int pipePadre;
//se settato indica la lettura
int lettura=0;
//checkvalue
int checkValue=0;
//per il sender
int pidSender;
//per il mirroring
//indice dei dispositivi che hanno registrato un cambiamento
int checkValueMirroring=0;
//puntatore che conterrà un vettore con tutti i dispositivi che hanno registrato un cambiamento manuale
pid_t *pidSenderMirroring;
int checkValue1=0;//sempre per il mirroring
//comunicazione tool esterno
//flag che indica se leggere dalla fifo per il controllo esterno
int flagEsterno=0;	
	
	
int main(int argc, char **argv)
{
	//pipe comandi
	int pipeComandi=atoi(argv[5]);
	
	printf("HUB ARRIVATO INIZIO \n");
	
	//inseriamo qui la inizializzazione di sto affare se no da errore
	pidSenderMirroring=(pid_t *)malloc(checkValueMirroring*sizeof(pid_t));
	
	//pid del linker
    pid_t pidLinker=atoi(argv[3]);
    
	//id del dispositivo
	int id;
	
	//variabile che rappresenta stato
int stato;

printf("HUB ARRIBVATO PRIMA DEI SUOI DISPOSITIVI \n");

    //pid dei dispositivi connessi
   
   //lampadine
	pid_t *vetPidBulb;
    int *fdLampadineMirroringLettura;
    int *fdLampadineComandi;
    int numeroLampadine=1;
	vetPidBulb=(pid_t *)malloc(numeroLampadine*sizeof(pid_t));
	
	fdLampadineMirroringLettura=(int *)malloc(numeroLampadine*sizeof(int));
	fdLampadineComandi=(int *)malloc(numeroLampadine*sizeof(int));
	int* fdLampadineInterruttore=(int *)malloc(numeroLampadine*sizeof(int));
	
	
	printf("ARRIVATO DOPO DICHIARAZIONi \n");
	
	int fdLampadinaMirroringLettura[2];
	if(pipe(fdLampadinaMirroringLettura)==-1)
	{
		printf("ERRORE PIPE \n");
		return EXIT_FAILURE;
	}
	
	int fdLampadinaComandi[2];
	if(pipe(fdLampadinaComandi)==-1)
	{
		printf("ERRORE PIPE \n");
		return EXIT_FAILURE;
	}
	
	fcntl(fdLampadinaComandi[0], F_SETFL, O_NONBLOCK);
	fcntl(fdLampadinaMirroringLettura[0], F_SETFL, O_NONBLOCK);
	
	printf("ARRIVATO DOPO PIPE \n");
	
	char pipeMirroringLettura[100];
	
	char pipeComand[100];//cambiato perche confliggeva

	printf("ARRIVATO DOPO VETTORI STRINGHE \n");
	
	if(sprintf(pipeMirroringLettura, "%d", fdLampadinaMirroringLettura[1])==-1)
	{
		printf("ERRORE \n");
	} 
	if(sprintf(pipeComand, "%d", fdLampadinaComandi[1])==-1)
	{
		printf("ERRORE \n");
	}
	
	printf("ARRIVATO PRIMA DI LETTURA \n");
	
	 char *args[]={"./lampadina", "6", "test1", argv[3], pipeMirroringLettura, pipeComand, NULL};
	
	
	printf("ARRIVATO DOPO VETTORE LAMPADINA \n");
	
	
	fflush(stdout);
	vetPidBulb[0]=fork();
	if(vetPidBulb[0]==-1)
	{
		printf("ERROE FORK LAMPADINA \n");
		return(EXIT_FAILURE);
	}
	
	printf("arrivato prima exec \n");
	
	if(vetPidBulb[0]==0)
	{
		execvp(args[0], args);
	}
	
	printf("arrivato dopo fork \n");
	
	printf("LA LAMPADINA DOVREBBE AVERE %d \n", fdLampadinaComandi[1]);
	
	close(fdLampadinaMirroringLettura[1]);
	
	close(fdLampadinaComandi[1]);
	
	
	fdLampadineMirroringLettura[0]=fdLampadinaMirroringLettura[0];
	
	fdLampadineComandi[0]=fdLampadinaComandi[0];
	
	printf("FD RECUPERATO %d \n", fdLampadineComandi[0]);
	
	printf("lampadina creata \n");
	
	//frigoriferi
	int numeroFrigoriferi=0;
	pid_t *vetPidFridge=(pid_t *)malloc(numeroFrigoriferi*sizeof(pid_t));
    int* fdFrigoriferiComandi=(int *)malloc(numeroFrigoriferi*sizeof(int));
	int* fdFrigoriferiInterruttore=(int *)malloc(numeroFrigoriferi*sizeof(int));
    int *fdFrigoriferiMirroringLettura=(int *)malloc(numeroFrigoriferi*sizeof(int));

	
	//finestre
	int numeroFinestre=0;
	pid_t *vetPidWindow=(pid_t *)malloc(numeroFinestre*sizeof(pid_t));
	int* fdFinestreComandi=(int *)malloc(numeroFinestre*sizeof(int));
	int* fdFinestreInterruttore=(int *)malloc(numeroFinestre*sizeof(int));
    int *fdFinestreMirroringLettura=(int *)malloc(numeroFinestre*sizeof(int));
	
	//hub
	int numeroHub=0;
	pid_t *vetPidHub=(pid_t *)malloc(numeroHub*sizeof(pid_t));
	int* fdHubComandi=(int *)malloc(numeroHub*sizeof(int));
    int* fdHubInterruttore=(int *)malloc(numeroHub*sizeof(int));
    int *fdHubMirroringLettura=(int *)malloc(numeroHub*sizeof(int));

	
	//timer
	int numeroTimer=0;
	pid_t *vetPidTimer=(pid_t *)malloc(numeroTimer*sizeof(pid_t));
    int* fdTimerComandi=(int *)malloc(numeroTimer*sizeof(int));
	int* fdTimerInterruttore=(int *)malloc(numeroTimer*sizeof(int));
    int *fdTimerMirroringLettura=(int *)malloc(numeroTimer*sizeof(int));
	
	//termostati
	int numeroTermostati=0;
	pid_t *vetPidTermostato=(pid_t *)malloc(numeroTermostati*sizeof(pid_t));
    int* fdTermostatiComandi=(int *)malloc(numeroTermostati*sizeof(int));
	int* fdTermostatiInterruttore=(int *)malloc(numeroTermostati*sizeof(int));
    int *fdTermostatiMirroringLettura=(int *)malloc(numeroTermostati*sizeof(int));
	
	
	//tende
	int numeroTende=0;
	pid_t *vetPidTenda=(pid_t *)malloc(numeroTende*sizeof(pid_t));
    int* fdTendeComandi=(int *)malloc(numeroTende*sizeof(int));
	int* fdTendeInterruttore=(int *)malloc(numeroTende*sizeof(int));
	int *fdTendeMirroringLettura=(int *)malloc(numeroTende*sizeof(int));

   
   
   
   
   
   
   
   
   
   
   
   
    //per la gestioone mirroring
   	      int numeroMirroring=0;//numero dei dispositivi con valori anomali
   	   int *vettoreMirroringId=(int *)malloc(numeroMirroring*sizeof(int));//vettore che contiene gli id dei dispositivi con valori anomali
   	int *vettoreMirroringStato=(int *)malloc(numeroMirroring*sizeof(int));//vettore che contiene lo stato dei dispositivo con valori anomali
   	   
    char **vettoreMirroringMessaggio;
   	  vettoreMirroringMessaggio=malloc(numeroMirroring*sizeof(char*));
   	  
   
	
        if(strcmp(argv[2],"test")==0)
	    {
		
		printf("sono un hub appena generato \n");
		
		if(checkValueMirroring>0)//controllo mirroring
		{
		}
		
		
		
		connected=1;
		
		stato=0;
		strcpy(interruttore, "OFF");
		
		printf("hub entra in test \n");
		printf("in argv1 c'e %s \n", argv[1]);
		id=atoi(argv[1]);
		printf("SONO UNA HUB %d \n", id);
		
		/*
		 * qui serve codice di mirroring di sopra
		 * 
		 */ 
		
		pipePadre=atoi(argv[4]);
		
		printf("HUB CONVERTITA PIPE PADRE \n");
		
		fdPadreInterruttore=atoi(argv[6]);
		printf("HUB LA MIA PIPE INTERRUTTORRE %d \n", fdPadreInterruttore);
		
				      //stringa in cui inserisco lo stato del dispositivo da mandare in pipe
		      char messaggio[100];
		      sprintf(messaggio, "%dX%dXdispositivi anomali:", stato, id);
		      
		      if(numeroMirroring==0)
		      {
				  sprintf(messaggio, "%s nessuno", messaggio);
			  }
		      
		      for(int i=0; i<numeroMirroring; i++)
		      {
				  sprintf(messaggio, "%s id: %d stato: %d messaggio:%s", messaggio, vettoreMirroringId[i], vettoreMirroringStato[i], vettoreMirroringMessaggio[i]);
			  }
		      
		      printf("HUB HA SCRITTO QUESTO MESSAGGIO %s IL MIO PID E' %d\n", messaggio, getpid());
		      //scrivo lo stato del dispositivo sulla pipe
		      if(write(pipePadre, messaggio, strlen(messaggio)+1)==-1)
		      {
				  printf("ERRORE\n");
			  }
		      //invio il segnale al padre
		      kill(getppid(), SIGIO);
		
		printf("HO AVVISATO PADRE PER MIRRORING \n");
		
		
	}
    
    //stringa fifo che contiene i comandi
    char* fifoComandi="./fifoComandi";
	int fdFifoComandi; //questo conteine fd della fifo per quando viene aperta
	char communication[]="XXXXXXXXX";
	char comand;
	char pos;
	
	
    //apro o creo il semaforo per la lettura dalla fifo comandi
	sem_t* semaforoFifoComandi=sem_open("IdCm", O_CREAT, 0666, 0);
	if(semaforoFifoComandi==-1)
	{
		printf("ERRORE APERTURA SEMAFORO DA PARTE LAMPADINA \n");
	}
	
	int value;
	sem_getvalue(semaforoFifoComandi, &value);
	printf("valore semaforo apertura comandi %d \n", value);
			  
	
	

	//imposto come handler di default del segnale utente 1 l'handler per la lettura della pipe principale
	signal(SIGUSR1, handlerLetturaPipe);
	
	//handler mirroring
	
		struct sigaction act2;
	memset(&act2, '\0', sizeof(act2));
	act2.sa_sigaction=&handlerCambiaInterruttore;
	act2.sa_flags=SA_RESTART;
	sigaction(SIGINT, &act2, NULL);
	
	
	struct sigaction act1;
	memset(&act1, '\0', sizeof(act1));
	act1.sa_sigaction=&handlerMirroring;
	act1.sa_flags=SA_RESTART|SA_SIGINFO;
	sigaction(SIGIO, &act1, NULL);
	
	
	//handler per la gestione del delete
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_sigaction=&handlerAck;
	act.sa_flags=SA_SIGINFO;
	sigaction(SIGUSR2, &act, NULL);
	
	  //dichiarazione della maschera per l'attesa
    sigset_t maschera;
    //il set viene riempito con tutti i segnali di sistema
    sigfillset(&maschera);
    // con questa funzione tolgo il segnale SIGUSR1 che mi gestisce la lettura nella fifo comandi
    sigdelset(&maschera, SIGUSR1);
    //con questa funzione tolgo il segnale SIGUSR2 che mi gestisce il cambio valore interruttore
    sigdelset(&maschera, SIGUSR2);
     //con questa funzione tolgo il segnale SIGTERM che mi gestisce una terminazione esterna, altrimenti sto processo non potrebbe mai esser chiuso, si veda sotto
    sigdelset(&maschera, SIGTERM);
    //con questa funzione tolgo il segnale SIGKILL che mi gestisce una terminazione fatale, nel caso ci fosse la rivolta delle macchine
    sigdelset(&maschera, SIGKILL);
    //con questa funzione tolgo il segnale SIGUSR2 che mi gestisce il cambio valore interruttore
    sigdelset(&maschera, SIGIO);
      //con questa funzione tolgo il segnale SIGUSR2 che mi gestisce il cambio valore interruttore
    sigdelset(&maschera, SIGINT);
    
    while(1)
    {
		//gestione mirroring
		//è praticamente identico alla centralina se non che non si printa alcun valore
		
		 if(checkValueMirroring>0)//proseguo solo se è avvenuto una variazione di stato in un dispositivo connesso registrata
	      {
	      
	      
	        for(int i=0; i<checkValueMirroring; i++)//per ogni pidsender presente individuo chi lo ha mandato e svolgo alcune operazioni
	        {
				
	           //individuiamo la categoria di dispositivo a cui fa riferimento il pid nel vettore e l'indice in cui si trova
	           
	           int tipo; //contiene il tipo di dispositivo 
	           int indice; //contiene l'indice del dispositivo nel vettore appropriato
	       
	       
	       
	            //lampadine
	            
	            for(int j=0; j<numeroLampadine; j++)
	            {
					if(pidSenderMirroring[i]==vetPidBulb[j])
					{
						indice=j;
						tipo=0;
					}
				}
	       
	            //finestre
	            for(int j=0; j<numeroFinestre; j++)
	            {
					if(pidSenderMirroring[i]==vetPidWindow[j])
					{
						indice=j;
						tipo=1;
					}
				}
	            
	            
	            //frigoriferi
	            for(int j=0; j<numeroFrigoriferi; j++)
	            {
					if(pidSenderMirroring[i]==vetPidFridge[j])
					{
						indice=j;
						tipo=2;
					}
				}
	            
	            
	            //hub
	            for(int j=0; j<numeroHub; j++)
	            {
					if(pidSenderMirroring[i]==vetPidHub[j])
					{
						indice=j;
						tipo=3;
					}
				}
	            
	            //timer
	            for(int j=0; j<numeroTimer; j++)
	            {
					if(pidSenderMirroring[i]==vetPidTimer[j])
					{
						indice=j;
						tipo=4;
					}
				}
				
	            //termostati
	            for(int j=0; j<numeroTermostati; j++)
	            {
					if(pidSenderMirroring[i]==vetPidTermostato[j])
					{
						indice=j;
						tipo=5;
					}
				}
	            
	            
	            //tende
	            for(int j=0; j<numeroTende; j++)
	            {
					if(pidSenderMirroring[i]==vetPidTenda[j])
					{
						indice=j;
						tipo=0;
					}
				}
	       
	            //vettori contenenti le informazioni da reperire dalla pipe mirroring
			     char messaggio[100];//vettore generale
			     char statoReperito[2];//conterrà lo stato reperito
	             char idReperito[4];//conterrà l'id reperito 
	
	             //sulla base della tipologia di dispositivo che abbiamo  individuato si ricerca la sua pipe e si svolgono alcune operazioni
	
                 if(tipo==0)//lampadine
                 {	
					 
	                if(read(fdLampadineMirroringLettura[indice], messaggio, 6)==-1)
	                {
						printf("ERRORE LETTURA PIPE MIRRORING DA PARTE DI CENTRALINA PREMERE UN PULSANTE PER USCIRE\n");
						getchar();
						return EXIT_FAILURE;
					}

	                //separo la stringa letta in pipe
	                
	                //individuo il separatore tra lo stato el'id
                   char* p;
                   p = strchr (messaggio, 'X'); // Cerca la X
   
           
                   *p = '\0'; // Sostituisco lo spazio col terminatore, separando le parole
                    p++; //incremento il puntatore p per puntare alla successiva lettera al terminatore
                    strcpy (statoReperito, messaggio); //copio la prima parte della stringa di messaggio in comando
                    strcpy (idReperito, p); //sovrascrivo idReperito con l'altra sottostringa
               
						
				    int idIntero=atoi(idReperito);//ottengo il valore intero dell'id
				    int statoIntero=atoi(statoReperito);//ottengo il valore intero stato
					     
		            //verifico se il valore non è gia presente nel vettore Mirroring
		               
		            int presente=0;//contiene 0 se non ci sono occorrenze o la posizione dell'occorrenza nel vettore mirroring
		            for(int i=0; i<numeroMirroring; i++)
		            {
						   if(vettoreMirroringId[i]==idIntero)
						   {
							   presente=i;
						   }
					 }
					   
					   
					if(presente==0)//se non presente lo aggiungo
					{
						   //aumento numero mirroring 
	                      numeroMirroring=numeroMirroring+1;
	
	                      //creo un  nuovo vettore dinamico di id
                           int *vettoreMirroringId1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                       {
		                       vettoreMirroringId1[i]=vettoreMirroringId[i];
	                       }
				          
				          if(numeroMirroring>0)
				          {
	                        //libero la memori puntata in precedenza dal vettore mirroring
	                        free(vettoreMirroringId);
					      }
					      
	                      //scambio i riferimenti tra l originale vettore pid  e quell nuovo creato
	                      vettoreMirroringId=vettoreMirroringId1;
				
                          //inserisco l'id reperito nell ultima posizione del vettore
                          vettoreMirroringId[numeroMirroring-1]=idIntero;
                          
                          //eseguo le stesse cose con lo stato
                           int *vettoreMirroringStato1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                      {
		                     vettoreMirroringStato1[i]=vettoreMirroringStato[i];
	                      }
				          
				          if(numeroMirroring>0)
				          {
      	                      free(vettoreMirroringStato);
					      }
					      
	                      vettoreMirroringStato=vettoreMirroringStato1;
				
                          vettoreMirroringStato[numeroMirroring-1]=statoIntero;
                          
                          //una cosa simile viene eseguita per la matrice messaggio
                          
                          char **vettoreMirroringMessaggio1;
                          vettoreMirroringMessaggio1=malloc(numeroMirroring*sizeof(char*));
                         
                          for(int i=0; i<numeroMirroring-1; i++)//ma bisogna prima allocare i vettori dinamici che compongono la matrice nel nuovo vettore dei messaggi
                          {
							  vettoreMirroringMessaggio1[i]=malloc(100*sizeof(char*));
						  }
                         
                         
                          for(int i=0; i<numeroMirroring-1; i++)
                          {
							  strcpy(vettoreMirroringMessaggio1[i], vettoreMirroringMessaggio[i]);
						  }
						  
						  if(numeroMirroring>0)
						  {
						   for(int i=0; i<numeroMirroring-1; i++)
                          {
							  free(vettoreMirroringMessaggio[i]);
						  }
						  
						  free(vettoreMirroringMessaggio);
					      }
					      
                          vettoreMirroringMessaggio=vettoreMirroringMessaggio1;
                          
                          vettoreMirroringMessaggio[numeroMirroring-1]=malloc(100*sizeof(char));
                          strcpy(vettoreMirroringMessaggio[numeroMirroring-1], "n/a");//nel caso di un dispositivo che non puo presentare mirroring il messaggio è fisso e indica la sua mancanza
                          
                          
                         						   
				    }else//se gia presente il pid aggiorno il valore dell'id e dello stato
				    {
							 vettoreMirroringId[presente]=idIntero;
							 vettoreMirroringStato[presente]=statoIntero;
						                          
				    }  
					   
				  
				    
		   
			     }else if(tipo==1)//stessa cosa per le finestre
			     {
					 
	                if(read(fdFinestreMirroringLettura[indice], messaggio, 6)==-1)
	                {
						printf("ERRORE LETTURA PIPE MIRRORING DA PARTE DI CENTRALINA PREMERE UN PULSANTE PER USCIRE\n");
						getchar();
						return EXIT_FAILURE;
					}

	                //separo la stringa letta in pipe
	                
	                //individuo il separatore tra lo stato el'id
                   char* p;
                   p = strchr (messaggio, 'X'); // Cerca la X
   
           
                   *p = '\0'; // Sostituisco lo spazio col terminatore, separando le parole
                    p++; //incremento il puntatore p per puntare alla successiva lettera al terminatore
                    strcpy (statoReperito, messaggio); //copio la prima parte della stringa di messaggio in comando
                    strcpy (idReperito, p); //sovrascrivo idReperito con l'altra sottostringa
               
						
				    int idIntero=atoi(idReperito);//ottengo il valore intero dell'id
				    int statoIntero=atoi(statoReperito);//ottengo il valore intero stato
					     
		            //verifico se il valore non è gia presente nel vettore Mirroring
		               
		            int presente=0;//contiene 0 se non ci sono occorrenze o la posizione dell'occorrenza nel vettore mirroring
		            for(int i=0; i<numeroMirroring; i++)
		            {
						   if(vettoreMirroringId[i]==idIntero)
						   {
							   presente=i;
						   }
					 }
					   
					   
					if(presente==0)//se non presente lo aggiungo
					{
						   //aumento numero mirroring 
	                      numeroMirroring=numeroMirroring+1;
	
	                      //creo un  nuovo vettore dinamico di id
                           int *vettoreMirroringId1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                       {
		                       vettoreMirroringId1[i]=vettoreMirroringId[i];
	                       }
				          
				          if(numeroMirroring>0)
				          {
	                        //libero la memori puntata in precedenza dal vettore mirroring
	                        free(vettoreMirroringId);
					      }
					      
	                      //scambio i riferimenti tra l originale vettore pid  e quell nuovo creato
	                      vettoreMirroringId=vettoreMirroringId1;
				
                          //inserisco l'id reperito nell ultima posizione del vettore
                          vettoreMirroringId[numeroMirroring-1]=idIntero;
                          
                          //eseguo le stesse cose con lo stato
                           int *vettoreMirroringStato1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                      {
		                     vettoreMirroringStato1[i]=vettoreMirroringStato[i];
	                      }
				          
				          if(numeroMirroring>0)
				          {
      	                      free(vettoreMirroringStato);
					      }
					      
	                      vettoreMirroringStato=vettoreMirroringStato1;
				
                          vettoreMirroringStato[numeroMirroring-1]=statoIntero;
                          
                          //una cosa simile viene eseguita per la matrice messaggio
                          
                          char **vettoreMirroringMessaggio1;
                          vettoreMirroringMessaggio1=malloc(numeroMirroring*sizeof(char*));
                         
                          for(int i=0; i<numeroMirroring-1; i++)//ma bisogna prima allocare i vettori dinamici che compongono la matrice nel nuovo vettore dei messaggi
                          {
							  vettoreMirroringMessaggio1[i]=malloc(100*sizeof(char*));
						  }
                         
                         
                          for(int i=0; i<numeroMirroring-1; i++)
                          {
							  strcpy(vettoreMirroringMessaggio1[i], vettoreMirroringMessaggio[i]);
						  }
						  
						  if(numeroMirroring>0)
						  {
						   for(int i=0; i<numeroMirroring-1; i++)
                          {
							  free(vettoreMirroringMessaggio[i]);
						  }
						  
						  free(vettoreMirroringMessaggio);
					      }
					      
                          vettoreMirroringMessaggio=vettoreMirroringMessaggio1;
                          
                          vettoreMirroringMessaggio[numeroMirroring-1]=malloc(100*sizeof(char));
                          strcpy(vettoreMirroringMessaggio[numeroMirroring-1], "n/a");//nel caso di un dispositivo che non puo presentare mirroring il messaggio è fisso e indica la sua mancanza
                          
                          
                         						   
				    }else//se gia presente il pid aggiorno il valore dell'id e dello stato
				    {
							 vettoreMirroringId[presente]=idIntero;
							 vettoreMirroringStato[presente]=statoIntero;
						                          
				    }  
				    
				 }else if(tipo==2)//frigoriferi
				 {
					  
	                if(read(fdFrigoriferiMirroringLettura[indice], messaggio, 6)==-1)
	                {
						printf("ERRORE LETTURA PIPE MIRRORING DA PARTE DI CENTRALINA PREMERE UN PULSANTE PER USCIRE\n");
						getchar();
						return EXIT_FAILURE;
					}

	                //separo la stringa letta in pipe
	                
	                //individuo il separatore tra lo stato el'id
                   char* p;
                   p = strchr (messaggio, 'X'); // Cerca la X
   
           
                   *p = '\0'; // Sostituisco lo spazio col terminatore, separando le parole
                    p++; //incremento il puntatore p per puntare alla successiva lettera al terminatore
                    strcpy (statoReperito, messaggio); //copio la prima parte della stringa di messaggio in comando
                    strcpy (idReperito, p); //sovrascrivo idReperito con l'altra sottostringa
               
						
				    int idIntero=atoi(idReperito);//ottengo il valore intero dell'id
				    int statoIntero=atoi(statoReperito);//ottengo il valore intero stato
					     
		            //verifico se il valore non è gia presente nel vettore Mirroring
		               
		            int presente=0;//contiene 0 se non ci sono occorrenze o la posizione dell'occorrenza nel vettore mirroring
		            for(int i=0; i<numeroMirroring; i++)
		            {
						   if(vettoreMirroringId[i]==idIntero)
						   {
							   presente=i;
						   }
					 }
					   
					   
					if(presente==0)//se non presente lo aggiungo
					{
						   //aumento numero mirroring 
	                      numeroMirroring=numeroMirroring+1;
	
	                      //creo un  nuovo vettore dinamico di id
                           int *vettoreMirroringId1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                       {
		                       vettoreMirroringId1[i]=vettoreMirroringId[i];
	                       }
				          
				          if(numeroMirroring>0)
				          {
	                        //libero la memori puntata in precedenza dal vettore mirroring
	                        free(vettoreMirroringId);
					      }
					      
	                      //scambio i riferimenti tra l originale vettore pid  e quell nuovo creato
	                      vettoreMirroringId=vettoreMirroringId1;
				
                          //inserisco l'id reperito nell ultima posizione del vettore
                          vettoreMirroringId[numeroMirroring-1]=idIntero;
                          
                          //eseguo le stesse cose con lo stato
                           int *vettoreMirroringStato1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                      {
		                     vettoreMirroringStato1[i]=vettoreMirroringStato[i];
	                      }
				          
				          if(numeroMirroring>0)
				          {
      	                      free(vettoreMirroringStato);
					      }
					      
	                      vettoreMirroringStato=vettoreMirroringStato1;
				
                          vettoreMirroringStato[numeroMirroring-1]=statoIntero;
                          
                          //una cosa simile viene eseguita per la matrice messaggio
                          
                          char **vettoreMirroringMessaggio1;
                          vettoreMirroringMessaggio1=malloc(numeroMirroring*sizeof(char*));
                         
                          for(int i=0; i<numeroMirroring-1; i++)//ma bisogna prima allocare i vettori dinamici che compongono la matrice nel nuovo vettore dei messaggi
                          {
							  vettoreMirroringMessaggio1[i]=malloc(100*sizeof(char*));
						  }
                         
                         
                          for(int i=0; i<numeroMirroring-1; i++)
                          {
							  strcpy(vettoreMirroringMessaggio1[i], vettoreMirroringMessaggio[i]);
						  }
						  
						  if(numeroMirroring>0)
						  {
						   for(int i=0; i<numeroMirroring-1; i++)
                          {
							  free(vettoreMirroringMessaggio[i]);
						  }
						  
						  free(vettoreMirroringMessaggio);
					      }
					      
                          vettoreMirroringMessaggio=vettoreMirroringMessaggio1;
                          
                          vettoreMirroringMessaggio[numeroMirroring-1]=malloc(100*sizeof(char));
                          strcpy(vettoreMirroringMessaggio[numeroMirroring-1], "n/a");//nel caso di un dispositivo che non puo presentare mirroring il messaggio è fisso e indica la sua mancanza
                          
                          
                         						   
				    }else//se gia presente il pid aggiorno il valore dell'id e dello stato
				    {
							 vettoreMirroringId[presente]=idIntero;
							 vettoreMirroringStato[presente]=statoIntero;
						                          
				    }  
				 }else if(tipo==3)//hub
				 {
					 
					 
	                if(read(fdHubMirroringLettura[indice], messaggio, 100)==-1)
	                {
						printf("ERRORE LETTURA PIPE MIRRORING DA PARTE DI CENTRALINA PREMERE UN PULSANTE PER TERMINARE\n");
						getchar();
						return EXIT_FAILURE;
					}
	
	                
	                //separo
	                //individuo il separatore tra lo stato el'id
                   char* p;
                   p = strchr (messaggio, 'X'); // Cerca la X
   
           
                   *p = '\0'; // Sostituisco la X col terminatore, separando le parole
                    p++; //incremento il puntatore p per puntare alla successiva lettera al terminatore
                    strcpy (statoReperito, messaggio); //copio la prima parte della stringa di messaggio in comando
     
                    //questa volta bisogna gestire un terzo elemento del messaggio, il messaggio appunto   
                    strcpy(messaggio, p);
    
                    p = strchr (messaggio, 'X'); // Cerca la X che separa stato da messaggio
                    *p = '\0'; // Sostituisco la X col terminatore, separando le parole
                    p++; //incremento il puntatore p per puntare alla successiva lettera al terminatore 

                    strcpy (idReperito, messaggio); //sovrascrivo idReperito con l'altra sottostringa
                    strcpy (messaggio, p); //sovrascrivo messaggio col messaggio vero e proprio

              
						
				   int idIntero=atoi(idReperito);//ottengo il valore intero dell'id
				   int statoIntero=atoi(statoReperito);//ottengo il valore intero stato
					     
		               //verifico se il valore non è gia presente nel vettore Mirroring
		               
		            int presente=0;//contiene il numero occorrenze
		               
		           for(int i=0; i<numeroMirroring; i++)
		           {
						   if(vettoreMirroringId[i]==idIntero)
						   {
							   presente=i;
						   }
					}
					   
					   
				   if(presente==0)//se non presente lo aggiungo
				   {
						   //aumento numero mirroring
	                      numeroMirroring=numeroMirroring+1;
	
	                      //creo un  nuovo vettore dinamico di id
                           int *vettoreMirroringId1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                       {
		                      vettoreMirroringId1[i]=vettoreMirroringId[i];
	                       }
				          
				          if(numeroMirroring>0)
				          {
	                         //libero la memori puntata in precedenza dal vettore mirroring
	                         free(vettoreMirroringId);
					      }
					      
	                      //scambio i riferimenti tra l originale vettore pid  e quell nuovo creato
	                      vettoreMirroringId=vettoreMirroringId1;
				
                          //ottengo il pid dell segnale nell ultimo indice del vettor
                          vettoreMirroringId[numeroMirroring-1]=idIntero;
                          
                          //creo un  nuovo vettore dinamico di id
                          int *vettoreMirroringStato1=(int *)malloc(numeroMirroring*sizeof(int));
	                      for(int i=0; i<(numeroMirroring-1); i++)
	                      {
		                     vettoreMirroringStato1[i]=vettoreMirroringStato[i];
	                      }
				          
				          if(numeroMirroring>0)
				          {
	                         //libero la memori puntata in precedenza dal vettore mirroring
	                         free(vettoreMirroringStato);
					      }
					      
	                      //scambio i riferimenti tra l originale vettore pid  e quell nuovo creato
	                      vettoreMirroringStato=vettoreMirroringStato1;
				
                          //ottengo il pid dell segnale nell ultimo indice del vettor
                          vettoreMirroringStato[numeroMirroring-1]=statoIntero;
                          
                          
                           char **vettoreMirroringMessaggio1;
                          vettoreMirroringMessaggio1=malloc(numeroMirroring*sizeof(char*));
                          
                            for(int i=0; i<numeroMirroring-1; i++)
                          {
							  vettoreMirroringMessaggio1[i]=malloc(100*sizeof(char*));
						  }
                          
                          for(int i=0; i<numeroMirroring-1; i++)
                          {
							                           

							  strcpy(vettoreMirroringMessaggio1[i], vettoreMirroringMessaggio[i]);
							  
							   
						  }
						  
						  
						  
						  if(numeroMirroring>0)
						  {
						   for(int i=0; i<numeroMirroring-1; i++)
                          {
							  free(vettoreMirroringMessaggio[i]);
						  }
						  
						  free(vettoreMirroringMessaggio);
					      }
					      
                          vettoreMirroringMessaggio=vettoreMirroringMessaggio1;
                          
                          vettoreMirroringMessaggio[numeroMirroring-1]=malloc(100*sizeof(char));
                          strcpy(vettoreMirroringMessaggio[numeroMirroring-1], messaggio);//copio il messaggio in questo caso che contiene i dati di mirroring dell hub con problemi di mirroring
                          
                        
						   
						 }else//se presente aggiorno il valore
						 {
							 vettoreMirroringId[presente]=idIntero;
							 vettoreMirroringStato[presente]=statoIntero;
							 strcpy(vettoreMirroringMessaggio[presente], messaggio);//in questo caso bisogna anche aggiornare il messaggio
						 }  
					 
					
                          
					 
				 }else if(tipo==4)//timer, simile a prima
				 {
					 
					 
	                if(read(fdTimerMirroringLettura[indice], messaggio, 100)==-1)
	                {
						printf("ERRORE LETTURA PIPE MIRRORING DA PARTE DI CENTRALINA PREMERE UN PULSANTE PER TERMINARE\n");
						getchar();
						return EXIT_FAILURE;
					}
	
	                
	                //separo
	                //individuo il separatore tra lo stato el'id
                   char* p;
                   p = strchr (messaggio, 'X'); // Cerca la X
   
           
                   *p = '\0'; // Sostituisco la X col terminatore, separando le parole
                    p++; //incremento il puntatore p per puntare alla successiva lettera al terminatore
                    strcpy (statoReperito, messaggio); //copio la prima parte della stringa di messaggio in comando
     
                    //questa volta bisogna gestire un terzo elemento del messaggio, il messaggio appunto   
                    strcpy(messaggio, p);
    
                    p = strchr (messaggio, 'X'); // Cerca la X che separa stato da messaggio
                    *p = '\0'; // Sostituisco la X col terminatore, separando le parole
                    p++; //incremento il puntatore p per puntare alla successiva lettera al terminatore 

                    strcpy (idReperito, messaggio); //sovrascrivo idReperito con l'altra sottostringa
                    strcpy (messaggio, p); //sovrascrivo messaggio col messaggio vero e proprio

              
						
				   int idIntero=atoi(idReperito);//ottengo il valore intero dell'id
				   int statoIntero=atoi(statoReperito);//ottengo il valore intero stato
					     
		               //verifico se il valore non è gia presente nel vettore Mirroring
		               
		            int presente=0;//contiene il numero occorrenze
		               
		           for(int i=0; i<numeroMirroring; i++)
		           {
						   if(vettoreMirroringId[i]==idIntero)
						   {
							   presente=i;
						   }
					}
					   
					   
				   if(presente==0)//se non presente lo aggiungo
				   {
						   //aumento numero mirroring
	                      numeroMirroring=numeroMirroring+1;
	
	                      //creo un  nuovo vettore dinamico di id
                           int *vettoreMirroringId1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                       {
		                      vettoreMirroringId1[i]=vettoreMirroringId[i];
	                       }
				          
				          if(numeroMirroring>0)
				          {
	                         //libero la memori puntata in precedenza dal vettore mirroring
	                         free(vettoreMirroringId);
					      }
					      
	                      //scambio i riferimenti tra l originale vettore pid  e quell nuovo creato
	                      vettoreMirroringId=vettoreMirroringId1;
				
                          //ottengo il pid dell segnale nell ultimo indice del vettor
                          vettoreMirroringId[numeroMirroring-1]=idIntero;
                          
                          //creo un  nuovo vettore dinamico di id
                          int *vettoreMirroringStato1=(int *)malloc(numeroMirroring*sizeof(int));
	                      for(int i=0; i<(numeroMirroring-1); i++)
	                      {
		                     vettoreMirroringStato1[i]=vettoreMirroringStato[i];
	                      }
				          
				          if(numeroMirroring>0)
				          {
	                         //libero la memori puntata in precedenza dal vettore mirroring
	                         free(vettoreMirroringStato);
					      }
					      
	                      //scambio i riferimenti tra l originale vettore pid  e quell nuovo creato
	                      vettoreMirroringStato=vettoreMirroringStato1;
				
                          //ottengo il pid dell segnale nell ultimo indice del vettor
                          vettoreMirroringStato[numeroMirroring-1]=statoIntero;
                          
                          
                           char **vettoreMirroringMessaggio1;
                          vettoreMirroringMessaggio1=malloc(numeroMirroring*sizeof(char*));
                          
                            for(int i=0; i<numeroMirroring-1; i++)
                          {
							  vettoreMirroringMessaggio1[i]=malloc(100*sizeof(char*));
						  }
                          
                          for(int i=0; i<numeroMirroring-1; i++)
                          {
							                           

							  strcpy(vettoreMirroringMessaggio1[i], vettoreMirroringMessaggio[i]);
							  
							   
						  }
						  
						  
						  
						  if(numeroMirroring>0)
						  {
						   for(int i=0; i<numeroMirroring-1; i++)
                          {
							  free(vettoreMirroringMessaggio[i]);
						  }
						  
						  free(vettoreMirroringMessaggio);
					      }
					      
                          vettoreMirroringMessaggio=vettoreMirroringMessaggio1;
                          
                          vettoreMirroringMessaggio[numeroMirroring-1]=malloc(100*sizeof(char));
                          strcpy(vettoreMirroringMessaggio[numeroMirroring-1], messaggio);//copio il messaggio in questo caso che contiene i dati di mirroring dell hub con problemi di mirroring
                          
                        
						   
						 }else//se presente aggiorno il valore
						 {
							 vettoreMirroringId[presente]=idIntero;
							 vettoreMirroringStato[presente]=statoIntero;
							 strcpy(vettoreMirroringMessaggio[presente], messaggio);//in questo caso bisogna anche aggiornare il messaggio
						 }  
					 
					
				 }else if(tipo==5)//termostati
				 {
					 
					   
	                if(read(fdTermostatiMirroringLettura[indice], messaggio, 6)==-1)
	                {
						printf("ERRORE LETTURA PIPE MIRRORING DA PARTE DI CENTRALINA PREMERE UN PULSANTE PER USCIRE\n");
						getchar();
						return EXIT_FAILURE;
					}

	                //separo la stringa letta in pipe
	                
	                //individuo il separatore tra lo stato el'id
                   char* p;
                   p = strchr (messaggio, 'X'); // Cerca la X
   
           
                   *p = '\0'; // Sostituisco lo spazio col terminatore, separando le parole
                    p++; //incremento il puntatore p per puntare alla successiva lettera al terminatore
                    strcpy (statoReperito, messaggio); //copio la prima parte della stringa di messaggio in comando
                    strcpy (idReperito, p); //sovrascrivo idReperito con l'altra sottostringa
               
						
				    int idIntero=atoi(idReperito);//ottengo il valore intero dell'id
				    int statoIntero=atoi(statoReperito);//ottengo il valore intero stato
					     
		            //verifico se il valore non è gia presente nel vettore Mirroring
		               
		            int presente=0;//contiene 0 se non ci sono occorrenze o la posizione dell'occorrenza nel vettore mirroring
		            for(int i=0; i<numeroMirroring; i++)
		            {
						   if(vettoreMirroringId[i]==idIntero)
						   {
							   presente=i;
						   }
					 }
					   
					   
					if(presente==0)//se non presente lo aggiungo
					{
						   //aumento numero mirroring 
	                      numeroMirroring=numeroMirroring+1;
	
	                      //creo un  nuovo vettore dinamico di id
                           int *vettoreMirroringId1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                       {
		                       vettoreMirroringId1[i]=vettoreMirroringId[i];
	                       }
				          
				          if(numeroMirroring>0)
				          {
	                        //libero la memori puntata in precedenza dal vettore mirroring
	                        free(vettoreMirroringId);
					      }
					      
	                      //scambio i riferimenti tra l originale vettore pid  e quell nuovo creato
	                      vettoreMirroringId=vettoreMirroringId1;
				
                          //inserisco l'id reperito nell ultima posizione del vettore
                          vettoreMirroringId[numeroMirroring-1]=idIntero;
                          
                          //eseguo le stesse cose con lo stato
                           int *vettoreMirroringStato1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                      {
		                     vettoreMirroringStato1[i]=vettoreMirroringStato[i];
	                      }
				          
				          if(numeroMirroring>0)
				          {
      	                      free(vettoreMirroringStato);
					      }
					      
	                      vettoreMirroringStato=vettoreMirroringStato1;
				
                          vettoreMirroringStato[numeroMirroring-1]=statoIntero;
                          
                          //una cosa simile viene eseguita per la matrice messaggio
                          
                          char **vettoreMirroringMessaggio1;
                          vettoreMirroringMessaggio1=malloc(numeroMirroring*sizeof(char*));
                         
                          for(int i=0; i<numeroMirroring-1; i++)//ma bisogna prima allocare i vettori dinamici che compongono la matrice nel nuovo vettore dei messaggi
                          {
							  vettoreMirroringMessaggio1[i]=malloc(100*sizeof(char*));
						  }
                         
                         
                          for(int i=0; i<numeroMirroring-1; i++)
                          {
							  strcpy(vettoreMirroringMessaggio1[i], vettoreMirroringMessaggio[i]);
						  }
						  
						  if(numeroMirroring>0)
						  {
						   for(int i=0; i<numeroMirroring-1; i++)
                          {
							  free(vettoreMirroringMessaggio[i]);
						  }
						  
						  free(vettoreMirroringMessaggio);
					      }
					      
                          vettoreMirroringMessaggio=vettoreMirroringMessaggio1;
                          
                          vettoreMirroringMessaggio[numeroMirroring-1]=malloc(100*sizeof(char));
                          strcpy(vettoreMirroringMessaggio[numeroMirroring-1], "n/a");//nel caso di un dispositivo che non puo presentare mirroring il messaggio è fisso e indica la sua mancanza
                          
                          
                         						   
				    }else//se gia presente il pid aggiorno il valore dell'id e dello stato
				    {
							 vettoreMirroringId[presente]=idIntero;
							 vettoreMirroringStato[presente]=statoIntero;
						                          
				    }  
					 
				 }else if(tipo==6)//tende
				 {
					   
	                if(read(fdTendeMirroringLettura[indice], messaggio, 6)==-1)
	                {
						printf("ERRORE LETTURA PIPE MIRRORING DA PARTE DI CENTRALINA PREMERE UN PULSANTE PER USCIRE\n");
						getchar();
						return EXIT_FAILURE;
					}

	                //separo la stringa letta in pipe
	                
	                //individuo il separatore tra lo stato el'id
                   char* p;
                   p = strchr (messaggio, 'X'); // Cerca la X
   
           
                   *p = '\0'; // Sostituisco lo spazio col terminatore, separando le parole
                    p++; //incremento il puntatore p per puntare alla successiva lettera al terminatore
                    strcpy (statoReperito, messaggio); //copio la prima parte della stringa di messaggio in comando
                    strcpy (idReperito, p); //sovrascrivo idReperito con l'altra sottostringa
               
						
				    int idIntero=atoi(idReperito);//ottengo il valore intero dell'id
				    int statoIntero=atoi(statoReperito);//ottengo il valore intero stato
					     
		            //verifico se il valore non è gia presente nel vettore Mirroring
		               
		            int presente=0;//contiene 0 se non ci sono occorrenze o la posizione dell'occorrenza nel vettore mirroring
		            for(int i=0; i<numeroMirroring; i++)
		            {
						   if(vettoreMirroringId[i]==idIntero)
						   {
							   presente=i;
						   }
					 }
					   
					   
					if(presente==0)//se non presente lo aggiungo
					{
						   //aumento numero mirroring 
	                      numeroMirroring=numeroMirroring+1;
	
	                      //creo un  nuovo vettore dinamico di id
                           int *vettoreMirroringId1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                       {
		                       vettoreMirroringId1[i]=vettoreMirroringId[i];
	                       }
				          
				          if(numeroMirroring>0)
				          {
	                        //libero la memori puntata in precedenza dal vettore mirroring
	                        free(vettoreMirroringId);
					      }
					      
	                      //scambio i riferimenti tra l originale vettore pid  e quell nuovo creato
	                      vettoreMirroringId=vettoreMirroringId1;
				
                          //inserisco l'id reperito nell ultima posizione del vettore
                          vettoreMirroringId[numeroMirroring-1]=idIntero;
                          
                          //eseguo le stesse cose con lo stato
                           int *vettoreMirroringStato1=(int *)malloc(numeroMirroring*sizeof(int));
	                       for(int i=0; i<(numeroMirroring-1); i++)
	                      {
		                     vettoreMirroringStato1[i]=vettoreMirroringStato[i];
	                      }
				          
				          if(numeroMirroring>0)
				          {
      	                      free(vettoreMirroringStato);
					      }
					      
	                      vettoreMirroringStato=vettoreMirroringStato1;
				
                          vettoreMirroringStato[numeroMirroring-1]=statoIntero;
                          
                          //una cosa simile viene eseguita per la matrice messaggio
                          
                          char **vettoreMirroringMessaggio1;
                          vettoreMirroringMessaggio1=malloc(numeroMirroring*sizeof(char*));
                         
                          for(int i=0; i<numeroMirroring-1; i++)//ma bisogna prima allocare i vettori dinamici che compongono la matrice nel nuovo vettore dei messaggi
                          {
							  vettoreMirroringMessaggio1[i]=malloc(100*sizeof(char*));
						  }
                         
                         
                          for(int i=0; i<numeroMirroring-1; i++)
                          {
							  strcpy(vettoreMirroringMessaggio1[i], vettoreMirroringMessaggio[i]);
						  }
						  
						  if(numeroMirroring>0)
						  {
						   for(int i=0; i<numeroMirroring-1; i++)
                          {
							  free(vettoreMirroringMessaggio[i]);
						  }
						  
						  free(vettoreMirroringMessaggio);
					      }
					      
                          vettoreMirroringMessaggio=vettoreMirroringMessaggio1;
                          
                          vettoreMirroringMessaggio[numeroMirroring-1]=malloc(100*sizeof(char));
                          strcpy(vettoreMirroringMessaggio[numeroMirroring-1], "n/a");//nel caso di un dispositivo che non puo presentare mirroring il messaggio è fisso e indica la sua mancanza
                          
                          
                         						   
				    }else//se gia presente il pid aggiorno il valore dell'id e dello stato
				    {
							 vettoreMirroringId[presente]=idIntero;
							 vettoreMirroringStato[presente]=statoIntero;
						                          
				    }  
				 }
              }
           
              checkValueMirroring=0; //reimposto il valore di checkvalue mirroring
              
              //libero la memoria del vettre dei pid
              free(pidSenderMirroring);
	       }
		
		
		//non è fra i registri ma ce lo calcoliamo comunque per comodità
		int num=numeroLampadine+numeroHub+numeroTimer+numeroTende+numeroFrigoriferi+numeroTermostati+numeroFinestre;
				
		sigsuspend(&maschera);//solito suspend in attesa di un segnale
		
		if (lettura==1)//controllo del valore di lettura, se a 1 procedi a legger un comando da centralina
		{
			
			printf("hub %d entra in lettura \n", id);
		
		fdFifoComandi=open(fifoComandi,O_RDWR);
		if (fdFifoComandi==-1)
		{
			printf("ERRORE APERTURA PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
			getchar();
			return EXIT_FAILURE;
		}
		
		printf("hub %d attende semaforo \n", id);
		sem_wait(semaforoFifoComandi);
		printf("hub %d esce attesa semaforo \n", id);
		
		if(read(fdFifoComandi, communication, strlen(communication))==-1)
		{
			printf("ERRORE LETTURA PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
			getchar();
			return EXIT_FAILURE;
		}
		
		printf("hub %d ha letto %s \n", id, communication);
		
		//scomposizione del comando
		comand=communication[0];
		
		printf("hub %d ha in command %c \n", id, comand);
		
		pos=communication[8];
		
	   int counter=3;
      
	   for(int i=1; i<4; i++)
	   {
		  if(communication[i]=='X')
		  {
			  counter=counter-1;
		  }
	   }
		
	   char *id1=(char *)malloc(counter*sizeof(char));
	
       for(int i=0; i<counter; i++)
	   {
		id1[i]=communication[i+1];
	   }	
	   
	   counter=3;
      
	   for(int i=5; i<8; i++)
	   {
		  if(communication[i]=='X')
		  {
			  counter=counter-1;
		  }
	   }
		
	   char *id2=(char *)malloc(counter*sizeof(char));
	
       for(int i=0; i<counter; i++)
	   {
		id2[i]=communication[i+5];
	   }	
		
			
		if(comand=='2')
		{
			
			printf("lampadina %d si trova in del \n", id);
			
			int idDaVerificare=atoi(id1);
			
			if(id==idDaVerificare)
			{
				strcpy(communication, "0XXXXXXXX");
				if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
	            
	            close(fdFifoComandi);//chiudo la fifo dei comandi
	            
	            //termino tutti i figli
	            
	            if(numeroLampadine>0)
	    {
			
	  
	    
	    for(int i=0; i<numeroLampadine;i++)
	    {
			kill(vetPidBulb[i], SIGTERM);
			printf("AVVISATA \n");
		}
		
		printf("HO AVVISATO LAMPADINE E ATTENDO \n");
		getchar();
		
		
		
		printf("RITORNATO \n");
		
		getchar();
		
	    }
		
		printf("PRIMA DI FRIOGORIFERI \n");
		getchar();
		
		if(numeroFrigoriferi>0)
		{
		
		
	    
	    for(int i=0; i<numeroFrigoriferi;i++)
	    {
			kill(vetPidFridge[i], SIGTERM);
		}
		
		
		
	    }
		
		if(numeroFinestre>0)
		{
		
		
	    
	    for(int i=0; i<numeroFinestre;i++)
	    {
			kill(vetPidWindow[i], SIGTERM);
		}
		
		
		
	    }
		
		if(numeroTende>0)
		{
		
		
	    
	    for(int i=0; i<numeroTende;i++)
	    {
			kill(vetPidTenda[i], SIGTERM);
		}
		
		
		
	}
	
	    if(numeroTermostati>0)
	    {
		
		
	    
	    for(int i=0; i<numeroTermostati;i++)
	    {
			kill(vetPidTermostato[i], SIGTERM);
		}
		
		
		
	}
	
	    if(numeroTimer>0)
	    {
		
		
	    
	    for(int i=0; i<numeroTimer;i++)
	    {
			kill(vetPidTimer[i], SIGTERM);
		}
		
		
		
	    }
	    
	    if(numeroHub>0)
	    {
		
	    
	    for(int i=0; i<numeroHub;i++)
	    {
			kill(vetPidHub[i], SIGTERM);
		}
		
		
		
	   }
	            
	            
	            //incremento il semaforo e segnalo al padre
	            sem_post(semaforoFifoComandi);
				kill(getppid(), SIGUSR2);
				
			}else
			{
				
				
				
				
				
				
				
				
				
				
				
				
				printf("processo padre ha letto il comando 2 \n");
			
			sem_post(semaforoFifoComandi);
			
			sem_getvalue(semaforoFifoComandi, &value );
			printf("processo iniziale dopo post ha ottenuto questo valore semaforo %d", value);
			
			printf("processo padre scrive sulla pipe %s \n", communication);
			
			if(numeroLampadine>0)
			{
				printf("CENTRALINA GESTISCO LAMPADINE \n");
			
			 //riscrivo sulla piipe
				   if(write(fdFifoComandi, communication, strlen(communication))==-1)
			       {
				      printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				      return EXIT_FAILURE;
			       }
				 
				 checkValue=0;
				 
				 //lampadine direttamente connesse 
				   for(int i=0; i<(numeroLampadine); i++)
			       {
				  	  kill(vetPidBulb[i], SIGUSR1);
			       }
			
			       
			
			       while(checkValue<numeroLampadine)
			       {
				      
			       } 
			       
			       
			       printf("processo iniziale ottiene feedback da tutte bulb \n");
			       
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
			          return EXIT_FAILURE;
		           }
			       
			       printf("processo iniziale ha letto questo %s \n", communication);
			 }
			       
			      if(communication[0]!='0')
				   {
					   
					   printf("SONO DI NUOVO IN UN CICLO \n");
			
			if(numeroFrigoriferi>0)
			{
			
					   //riscrivo sulla piipe
				       if(write(fdFifoComandi, communication, strlen(communication))==-1)
			           {
				           printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				           return EXIT_FAILURE;
			           }
				 
				 checkValue=0;
				 
				       //lampadine direttamente connesse 
				       for(int i=0; i<(numeroFrigoriferi); i++)
			           {
				  	      kill(vetPidFridge[i], SIGUSR1);
			           }
			
			            
			
			            while(checkValue<numeroFrigoriferi)
			            {
				           
			            } 
			       
			           if(read(fdFifoComandi, communication, strlen(communication))==-1)
		               {
			               printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			               getchar();
			               return EXIT_FAILURE;
		                }
			 }
			       
			            if(communication[0]!='0')
				        { 
			
			if(numeroFinestre>0)
			{
							//riscrivo sulla piipe
				          if(write(atoi(argv[2]), communication, strlen(communication))==-1)
			              {
				              printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				              return EXIT_FAILURE;
			              }
			              
			              checkValue=0;
				 
				          //lampadine direttamente connesse 
				          for(int i=0; i<(numeroFinestre); i++)
			              {
				  	         kill(vetPidWindow[i], SIGUSR1);
			              }
			
			              
			              while(checkValue<numeroFinestre)
			              {
			              } 
			       
			              if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                  {
			                  printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			                  getchar();
			                  return EXIT_FAILURE;
		                  }
			}
					   
					      if(communication[0]!='0')
					      {
			
			if(numeroTimer>0)
			{
							  	//riscrivo sulla piipe
				             if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                 {
				                printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                return EXIT_FAILURE;
			                  }
				 
				 checkValue=0;
				 
				            //lampadine direttamente connesse 
				              for(int i=0; i<(numeroTimer); i++)
			                  {
				  	             kill(vetPidTimer[i], SIGUSR1);
			                  }
			
			                  
			
			                  while(checkValue<numeroTimer)
			                  {
				                 
			                   } 
			       
			                  if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                      {
			                      printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			                      getchar();
			                      return EXIT_FAILURE;
		                       }
							  
			}
							  
							  if(communication[0]!='0')
							  {
			if(numeroHub>0)
			{
			
								   	//riscrivo sulla piipe
				               if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                   {
				                   printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                   return EXIT_FAILURE;
			                   }
				 
				 checkValue=0;
				 
				               //lampadine direttamente connesse 
				               for(int i=0; i<(numeroHub); i++)
			                   {
				  	              kill(vetPidHub[i], SIGUSR1);
			                   }
			
			
			                   while(checkValue<numeroHub)
			                   {    
								  
			                    } 
			     
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
			          return EXIT_FAILURE;
		           }     
		           
			   }   
		           
		           if(communication[0]!='0')
		           {
					   
					   if(numeroTende>0)
			{
			
								   	//riscrivo sulla piipe
				               if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                   {
				                   printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                   return EXIT_FAILURE;
			                   }
				 
				 checkValue=0;
				 
				               //lampadine direttamente connesse 
				               for(int i=0; i<(numeroTende); i++)
			                   {
				  	              kill(vetPidTenda[i], SIGUSR1);
			                   }
			
			
			                   while(checkValue<numeroTende)
			                   {    
								  
			                    } 
			     
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
			          return EXIT_FAILURE;
		           }     
		           
			   } 
					   
					   
		    		   if(communication[0]!='0')
					   {
						   if(numeroTermostati>0)
			{
			
								   	//riscrivo sulla piipe
				               if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                   {
				                   printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                   return EXIT_FAILURE;
			                   }
				 
				 checkValue=0;
				 
				               //lampadine direttamente connesse 
				               for(int i=0; i<(numeroTermostati); i++)
			                   {
				  	              kill(vetPidTermostato[i], SIGUSR1);
			                   }
			
			
			                   while(checkValue<numeroTermostati)
			                   {    
								  
			                    } 
			     
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
			          return EXIT_FAILURE;
		           }     
		           
			   } 
						  
						   if(communication[0]!='0')
					       {
					          //non faccio niente
					        
					         }else{
								 
								 
								 if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroTermostati; i++)
						   {
							   if (vetPidTermostato[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   //invio sigterm al processo in questione
						   kill(vetPidTermostato[indice], SIGTERM);
						   
						   //alloco un nuovo vettore lampadine
						   pid_t *vetPidTermostato1=(pid_t *)malloc((numeroTermostati-1)*sizeof(pid_t));
						   
						   //copio tutti i pid inferiori dell indice dal primo vettore al secondo
						   for(int i=0; i<indice; i++)
						   {
							   vetPidTermostato1[i]=vetPidTermostato[i];
						   }
						   //stessa cosa da dopo l'inidice incriminato
						   for(int i=(indice+1); i<numeroTermostati; i++)
						   {
							   vetPidTermostato1[i-1]=vetPidTermostato[i];
						   }
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidTermostato=vetPidTermostato1;
						   
						   numeroTermostati=numeroTermostati-1;
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
						  
						  
						  
					         }	 
					      
						   
					  
					  
					  }else{
						  
						  if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroTende; i++)
						   {
							   if (vetPidTenda[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   //invio sigterm al processo in questione
						   kill(vetPidTenda[indice], SIGTERM);
						   
						   //alloco un nuovo vettore lampadine
						   pid_t *vetPidTenda1=(pid_t *)malloc((numeroTende-1)*sizeof(pid_t));
						   
						   //copio tutti i pid inferiori dell indice dal primo vettore al secondo
						   for(int i=0; i<indice; i++)
						   {
							   vetPidTenda1[i]=vetPidTenda[i];
						   }
						   //stessa cosa da dopo l'inidice incriminato
						   for(int i=(indice+1); i<numeroTende; i++)
						   {
							   vetPidTenda1[i-1]=vetPidTenda[i];
						   }
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidTenda=vetPidTenda1;
						   
						   numeroTende=numeroTende-1;
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
						  
					  }	 
					  
					  
					  }else
					  { 
					  
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroHub; i++)
						   {
							   if (vetPidHub[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   //invio sigterm al processo in questione
						   kill(vetPidHub[indice], SIGTERM);
						   
						   //alloco un nuovo vettore lampadine
						   pid_t *vetPidHub1=(pid_t *)malloc((numeroHub-1)*sizeof(pid_t));
						   
						   //copio tutti i pid inferiori dell indice dal primo vettore al secondo
						   for(int i=0; i<indice; i++)
						   {
							   vetPidHub1[i]=vetPidHub[i];
						   }
						   //stessa cosa da dopo l'inidice incriminato
						   for(int i=(indice+1); i<numeroHub; i++)
						   {
							   vetPidHub1[i-1]=vetPidHub[i];
						   }
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidHub=vetPidHub1;
						   
						   numeroHub=numeroHub-1;
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   
			   }
			                   
		                           
								  
							  }else{
					  
					   //qui ho trovato una lampadina
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroTimer; i++)
						   {
							   if (vetPidTimer[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   //invio sigterm al processo in questione
						   kill(vetPidTimer[indice], SIGTERM);
						   
						   //alloco un nuovo vettore lampadine
						   pid_t *vetPidTimer1=(pid_t *)malloc((numeroTimer-1)*sizeof(pid_t));
						   
						   //copio tutti i pid inferiori dell indice dal primo vettore al secondo
						   for(int i=0; i<indice; i++)
						   {
							   vetPidTimer1[i]=vetPidTimer[i];
						   }
						   //stessa cosa da dopo l'inidice incriminato
						   for(int i=(indice+1); i<numeroTimer; i++)
						   {
							   vetPidTimer1[i-1]=vetPidTimer[i];
						   }
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidTimer=vetPidTimer1;
						   
						   numeroTimer=numeroTimer-1;
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
					  }else{
					   
					   //qui ho trovato una lampadina
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroFinestre; i++)
						   {
							   if (vetPidWindow[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   //invio sigterm al processo in questione
						   kill(vetPidWindow[indice], SIGTERM);
						   
						   //alloco un nuovo vettore lampadine
						   pid_t *vetPidWindow1=(pid_t *)malloc((numeroFinestre-1)*sizeof(pid_t));
						   
						   //copio tutti i pid inferiori dell indice dal primo vettore al secondo
						   for(int i=0; i<indice; i++)
						   {
							   vetPidWindow1[i]=vetPidWindow[i];
						   }
						   //stessa cosa da dopo l'inidice incriminato
						   for(int i=(indice+1); i<numeroFinestre; i++)
						   {
							   vetPidWindow1[i-1]=vetPidWindow[i];
						   }
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidWindow=vetPidWindow1;
						   
						   numeroFinestre=numeroFinestre-1;
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
					   
					   
				        }else{
					  
					   //qui ho trovato una lampadina
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroFrigoriferi; i++)
						   {
							   if (vetPidFridge[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   //invio sigterm al processo in questione
						   kill(vetPidFridge[indice], SIGTERM);
						   
						   //alloco un nuovo vettore lampadine
						   pid_t *vetPidFridge1=(pid_t *)malloc((numeroFrigoriferi-1)*sizeof(pid_t));
						   
						   //copio tutti i pid inferiori dell indice dal primo vettore al secondo
						   for(int i=0; i<indice; i++)
						   {
							   vetPidFridge1[i]=vetPidFridge[i];
						   }
						   //stessa cosa da dopo l'inidice incriminato
						   for(int i=(indice+1); i<numeroFrigoriferi; i++)
						   {
							   vetPidFridge1[i-1]=vetPidFridge[i];
						   }
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidFridge=vetPidFridge1;
						   
						   
						   numeroFrigoriferi=numeroFrigoriferi-1;
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   } 
					   
					   
					   
					   
				 }else{
					  
					   //qui ho trovato una lampadina
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroLampadine; i++)
						   {
							   if (vetPidBulb[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   //invio sigterm al processo in questione
						   kill(vetPidBulb[indice], SIGTERM);
						   
						   //alloco un nuovo vettore lampadine
						   pid_t *vetPidBulb1=(pid_t *)malloc((numeroLampadine-1)*sizeof(pid_t));
						   
						   //copio tutti i pid inferiori dell indice dal primo vettore al secondo
						   for(int i=0; i<indice; i++)
						   {
							   vetPidBulb1[i]=vetPidBulb[i];
						   }
						   //stessa cosa da dopo l'inidice incriminato
						   for(int i=(indice+1); i<numeroLampadine; i++)
						   {
							   vetPidBulb1[i-1]=vetPidBulb[i];
						   }
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidBulb=vetPidBulb1;
						   
						   numeroLampadine=numeroLampadine-1;
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
						   
					       
				
				
				
				
				
				if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
	            
				kill(getppid(), SIGUSR1);
			}
		}	
	    else if(comand=='6')
		{
			/*
		  * tra parentesi non saprei se conviene usare l handler per il cambio di stato
		  * 
		  */  
		  
		  printf("hub %d è nel comando 6 \n", id);
		  
		  char label=communication[4];
		  char pos=communication[8];
		  
		  printf("label e pos identificate %c %c \n", label, pos);
		  
		 int idDaVerificare=atoi(id1);
		  
		  if(idDaVerificare==id)
		  {
			 printf("hub %d riconosce il proprio id \n", id);
			 strcpy(communication, "0XXXXXXXX");
			 
			 if(label=='0')
			 {
				 
				 printf("hub riconosce id e riconosce interruttore giusto \n");
			 
			  if (pos=='0')
			  {
				  if(strcmp(interruttore, "ON")==0)
				  {
					  printf("DEVO SWITCHARE A OFF \n");
					  //chiama segnale cambio interruttore
					  raise(SIGINT);
					  
					  printf("IL MIO INTERRUTTROE VALE %s \n", interruttore);
					  
					  getchar();
				  }
				  
				  
			  }else 
			  {
				  printf("devo invertire interruttore a on");
				  
				  if(strcmp(interruttore, "OFF")==0)
				  {
					  printf("il mio interruttore era %s \n", interruttore);
					  
					  //chiama segnale cambio interruttore
					  raise(SIGINT);
					  
					  printf("il mio interruttore ora %s \n", interruttore);
					  
				  }
				  
			  }
			  
		    }else
		    {
				printf("lampadina riconosce proprio id e interruttore sbagliato \n");
				
				strcpy(communication, "01XXXXXXX");
				
			}
			  
		  }else//questo è importante altrimenti non va avanti
		  {
			
			
			printf("hub scrive sulla pipe %s \n", communication);
			
			 //riscrivo sulla piipe
				   if(write(fdFifoComandi, communication, strlen(communication))==-1)
			       {
				      printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				      return EXIT_FAILURE;
			       }
				 
				 //conto i feedback di ritorno
			       checkValue=0;
				 
				 //lampadine direttamente connesse 
				   for(int i=0; i<(numeroLampadine); i++)
			       {
					   printf("AVVISATA \n");
				  	  kill(vetPidBulb[i], SIGUSR1);
			       }
			
			
			
			sem_post(semaforoFifoComandi);
			
			sem_getvalue(semaforoFifoComandi, &value );
			printf("processo hub dopo post ha ottenuto questo valore semaforo %d", value);  
			
			       while(checkValue<(numeroLampadine))
			       {
				      /*printf("checkvalue in iniziale è %d \n", checkValue);
				      sigsuspend(&maschera);
				      */
			       } 
			       
			      
			       
			       printf("processo iniziale ottiene feedback da tutte bulb \n");
			       
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
			          return EXIT_FAILURE;
		           }
			       
			       printf("hub ha letto questo %s \n", communication);
			       
			       if(communication[0]!='0')
				   {
					   if(numeroFrigoriferi>0)
					   {
					   //riscrivo sulla piipe
				       if(write(fdFifoComandi, communication, strlen(communication))==-1)
			           {
				           printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				           return EXIT_FAILURE;
			           }
                        
                        printf("sto notificando frighi \n");			 
				 
				       //lampadine direttamente connesse 
				       for(int i=0; i<(numeroFrigoriferi); i++)
			           {
				  	      kill(vetPidFridge[i], SIGUSR1);
			           }
			
			             while(checkValue<(numeroFrigoriferi))
			            {
				          
				      
			              } 
			       
			           printf("esco frighi \n");
			       
			           if(read(fdFifoComandi, communication, strlen(communication))==-1)
		               {
			               printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			               getchar();
			               return EXIT_FAILURE;
		                }
		                
					}
			       
			            if(communication[0]!='0')
				        { 
						
						  if(numeroFinestre>0)
						  {	
							
							//riscrivo sulla piipe
				          if(write(atoi(argv[2]), communication, strlen(communication))==-1)
			              {
				              printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				              return EXIT_FAILURE;
			              }
				 
				           checkValue=0;
				           
				          //lampadine direttamente connesse 
				          for(int i=0; i<(numeroFinestre); i++)
			              {
				  	         kill(vetPidWindow[i], SIGUSR1);
			              }
			
			               while(checkValue<(numeroFinestre))
			              {
				             
				      
			               } 
			       
			       
			              if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                  {
			                  printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			                  getchar();
			                  return EXIT_FAILURE;
		                  }
		                  
					  }
					   
					      if(communication[0]!='0')
					      {
							  
							  if(numeroTimer>0)
							  {
							  	//riscrivo sulla piipe
				             if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                 {
				                printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                return EXIT_FAILURE;
			                  }
				 
				               checkValue=0;
				               
				            //lampadine direttamente connesse 
				              for(int i=0; i<(numeroTimer); i++)
			                  {
				  	             kill(vetPidTimer[i], SIGUSR1);
			                  }
			
			                   while(checkValue<(numeroTimer))
			                   {
				                 
				      
			                   } 
			       
			       
			                  if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                      {
			                      printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			                      getchar();
			                      return EXIT_FAILURE;
		                       }
							  
						  }
							  
							  if(communication[0]!='0')
							  {
								  if(numeroHub>0)
								  {
								  
								  
								   	//riscrivo sulla piipe
				               if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                   {
				                   printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                   return EXIT_FAILURE;
			                   }
				               
				               checkValue=0;
				 
				               //lampadine direttamente connesse 
				               for(int i=0; i<(numeroHub); i++)
			                   {
				  	              kill(vetPidHub[i], SIGUSR1);
			                   }
			
			                   //conto i feedback di ritorno
			                    while(checkValue<(numeroHub))
			                   {
				                 
				      
			                    } 
							}
			       
			                    
			                    if(communication[0]!='0')
		                                   {
											   if(numeroTende>0)
											   {
											       //riscrivo sulla piipe
				                              if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                                  {
				                                 printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                                 return EXIT_FAILURE;
			                                  }
			                                  
			                                  checkValue=0;
				 
				                              //lampadine direttamente connesse 
				                              for(int i=0; i<(numeroTende); i++)
			                                  {
				  	                              kill(vetPidTenda[i], SIGUSR1);
			                                  }
			
			
			                                   while(checkValue<(numeroTende))
			                                  {
				                                 
				      
			                                   } 
			       
			       
			                                   if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                                       {    
			                                      printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			                                      getchar();
			                                      return EXIT_FAILURE;
		                                       }
										   }
		                                       if(communication[0]!='0')
		                                       {
												   if(numeroTermostati>0)
												   {
												   
												       //riscrivo sulla piipe
				                                  if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                                      {
				                                     printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                                     return EXIT_FAILURE;
			                                      }
				                                  
				                                  checkValue=0;
				 
				                                  //lampadine direttamente connesse 
				                                  for(int i=0; i<(numeroTermostati); i++)
			                                      {
				  	                                  kill(vetPidTermostato[i], SIGUSR1);
			                                       }
			
			                                       while(checkValue<(numeroTermostati))
			                                      {
				                                     
				      
			                                       } 
			       
			       
			                                      if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                                          {      
			                                          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			                                          getchar();
			                                          return EXIT_FAILURE;
		                                           }
			       
			                                       printf("ho finito anche coi termostati \n");
		                                      }
								        } 
							       }
							  }
						  }
					   
					   
				        }
					   
					   
					   
					   
				   
				   
				  
			
		      }
			  
			  
		  }
		  
		  printf("hub scrive questo %s \n", communication);
		  
		  if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
	            
	            printf("hub %d invia segnale a processo iniziale 	\n ", id);
				sem_post(semaforoFifoComandi);
				kill(getppid(), SIGUSR1);
				
		  
		  
		}else if(comand=='3')
		{
			printf("hub %d sono nel comando 3 \n", id);
			
			int idDaVerificare=atoi(id1);
		  
		  if(idDaVerificare==id)
		  {
			  strcpy(communication, "0XXXXXXXX");
			 
			 printf("riconosciuto il mio id %d \n", id);
			 
			 char informazioni[100];
			  
			 
			  
			  char statoAusiliare[]="spento";
			  
			  if(stato==1)
			  {
				  strcpy(statoAusiliare, "acceso");
			  }
			  
			  /*
			   * PROBABILMENTE QUI VA INSERITO INFO MIRRORING
			   * 
			   */ 
			  
					 sprintf(informazioni, "hub %d %s ", id, statoAusiliare);
					 
					 
			                 if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
			                 {
				                    printf("errore \n");
				                    return EXIT_FAILURE;
			                  }
			 
			 
			 
			         printf("ho scritto %s \n", informazioni);
			 
				 
			  
			  
		  }else//devo propagare il segnale come il processo iniziale
		  {
			  printf("hub propaga segnale \n");
			 
			 sem_post(semaforoFifoComandi);
			  
			    if(numeroLampadine>0)
		    {
			
			 
		       if(write(fdFifoComandi, communication, strlen(communication))==-1)
		       {
			  printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
			  return EXIT_FAILURE;
		       }
				 
		       checkValue=0;
				 
		        //lampadine direttamente connesse 
				   for(int i=0; i<(numeroLampadine); i++)
			       {
				  	  kill(vetPidBulb[i], SIGUSR1);
			       }
			
			       
			
			       while(checkValue<numeroLampadine)
			       {
				      
			       } 
			       
			       
			       printf("processo iniziale ottiene feedback da tutte bulb \n");
			       
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
			          return EXIT_FAILURE;
		           }
			       
			       printf("processo iniziale ha letto questo %s \n", communication);
			 }
			       
			      if(communication[0]!='0')
				   {
					   
					   printf("SONO DI NUOVO IN UN CICLO \n");
			
			if(numeroFrigoriferi>0)
			{
			
					   //riscrivo sulla piipe
				       if(write(fdFifoComandi, communication, strlen(communication))==-1)
			           {
				           printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				           return EXIT_FAILURE;
			           }
				 
				 checkValue=0;
				 
				       //lampadine direttamente connesse 
				       for(int i=0; i<(numeroFrigoriferi); i++)
			           {
				  	      kill(vetPidFridge[i], SIGUSR1);
			           }
			
			            
			
			            while(checkValue<numeroFrigoriferi)
			            {
				           
			            } 
			       
			           if(read(fdFifoComandi, communication, strlen(communication))==-1)
		               {
			               printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			               getchar();
			               return EXIT_FAILURE;
		                }
			     }
			       
			            if(communication[0]!='0')
				        { 
			
			if(numeroFinestre>0)
			{
							//riscrivo sulla piipe
				          if(write(atoi(argv[2]), communication, strlen(communication))==-1)
			              {
				              printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				              return EXIT_FAILURE;
			              }
			              
			              checkValue=0;
				 
				          //lampadine direttamente connesse 
				          for(int i=0; i<(numeroFinestre); i++)
			              {
				  	         kill(vetPidWindow[i], SIGUSR1);
			              }
			
			              
			              while(checkValue<numeroFinestre)
			              {
			              } 
			       
			              if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                  {
			                  printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			                  getchar();
			                  return EXIT_FAILURE;
		                  }
			}
					   
					      if(communication[0]!='0')
					      {
			
			if(numeroTimer>0)
			{
							  	//riscrivo sulla piipe
				             if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                 {
				                printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                return EXIT_FAILURE;
			                  }
				 
				 checkValue=0;
				 
				            //lampadine direttamente connesse 
				              for(int i=0; i<(numeroTimer); i++)
			                  {
				  	             kill(vetPidTimer[i], SIGUSR1);
			                  }
			
			                  
			
			                  while(checkValue<numeroTimer)
			                  {
				                 
			                   } 
			       
			                  if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                      {
			                      printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			                      getchar();
			                      return EXIT_FAILURE;
		                       }
							  
			}
							  
							  if(communication[0]!='0')
							  {
			if(numeroHub>0)
			{
			
								   	//riscrivo sulla piipe
				               if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                   {
				                   printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                   return EXIT_FAILURE;
			                   }
				 
				 checkValue=0;
				 
				               //lampadine direttamente connesse 
				               for(int i=0; i<(numeroHub); i++)
			                   {
				  	              kill(vetPidHub[i], SIGUSR1);
			                   }
			
			
			                   while(checkValue<numeroHub)
			                   {    
								  
			                    } 
			     
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
			          return EXIT_FAILURE;
		           }     
		           
			   }   
		           
		           if(communication[0]!='0')
		           {
					   
					   if(numeroTende>0)
			{
			
								   	//riscrivo sulla piipe
				               if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                   {
				                   printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                   return EXIT_FAILURE;
			                   }
				 
				 checkValue=0;
				 
				               //lampadine direttamente connesse 
				               for(int i=0; i<(numeroTende); i++)
			                   {
				  	              kill(vetPidTenda[i], SIGUSR1);
			                   }
			
			
			                   while(checkValue<numeroTende)
			                   {    
								  
			                    } 
			     
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
			          return EXIT_FAILURE;
		           }     
		           
			   } 
					   
					   
		    		   if(communication[0]!='0')
					   {
						   if(numeroTermostati>0)
			{
			
								   	//riscrivo sulla piipe
				               if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                   {
				                   printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                   return EXIT_FAILURE;
			                   }
				 
				 checkValue=0;
				 
				               //lampadine direttamente connesse 
				               for(int i=0; i<(numeroTermostati); i++)
			                   {
				  	              kill(vetPidTermostato[i], SIGUSR1);
			                   }
			
			
			                   while(checkValue<numeroTermostati)
			                   {    
								  
			                    } 
			     
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
			          return EXIT_FAILURE;
		           }     
		           
			   } 
						  
						   if(communication[0]!='0')
					       {
					          //non faccio niente
					        
					         }else{
								 
								 
								 if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroTermostati; i++)
						   {
							   if (vetPidTermostato[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   char informazioni[500];
						   
						   if(read(fdTermostatiComandi[indice], informazioni, 500)==-1)
						   {
							   printf("errore lettura pipe lampadina\n");
						   }
						   
						   printf("ho letto %s \n", informazioni);
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							   printf("errore scrittura \n");
						   }
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
						  
						  
						  
					         }	 
					      
						   
					  
					  
					  }else{
						  
						  if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroTende; i++)
						   {
							   if (vetPidTenda[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   char informazioni[500];
						   
						   if(read(fdTendeComandi[indice], informazioni, 500)==-1)
						   {
							   printf("errore lettura pipe lampadina\n");
						   }
						   
						   printf("ho letto %s \n", informazioni);
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							   printf("errore scrittura \n");
						   }
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
						  
					  }	 
					  
					  
					  }else
					  { 
					  
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroHub; i++)
						   {
							   if (vetPidHub[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   char informazioni[500];
						   
						   if(read(fdHubComandi[indice], informazioni, 500)==-1)
						   {
							   printf("errore lettura pipe lampadina\n");
						   }
						   
						   printf("ho letto %s \n", informazioni);
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							   printf("errore scrittura \n");
						   }
						   
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   
			   }
			                   
		                           
								  
							  }else{
					  
					   //qui ho trovato una lampadina
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroTimer; i++)
						   {
							   if (vetPidTimer[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   char informazioni[500];
						   
						   if(read(fdTimerComandi[indice], informazioni, 500)==-1)
						   {
							   printf("errore lettura pipe lampadina\n");
						   }
						   
						   printf("ho letto %s \n", informazioni);
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							   printf("errore scrittura \n");
						   }
						   
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
					  }else{
					   
					   //qui ho trovato una lampadina
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroFinestre; i++)
						   {
							   if (vetPidWindow[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   char informazioni[500];
						   
						   if(read(fdFinestreComandi[indice], informazioni, 500)==-1)
						   {
							   printf("errore lettura pipe lampadina\n");
						   }
						   
						   printf("ho letto %s \n", informazioni);
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							   printf("errore scrittura \n");
						   }
						   
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
					   
					   
				        }else{
					  
					   //qui ho trovato una lampadina
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroFrigoriferi; i++)
						   {
							   if (vetPidFridge[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   char informazioni[500];
						   
						   if(read(fdFrigoriferiComandi[indice], informazioni, 500)==-1)
						   {
							   printf("errore lettura pipe lampadina\n");
						   }
						   
						   printf("ho letto %s \n", informazioni);
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							   printf("errore scrittura \n");
						   }
						   
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   } 
					   
					   
					   
					   
				 }else{
					  
					  printf("ho rilevato una lampadina incriminata \n");
					  
					   //qui ho trovato una lampadina
					   if (pidSender!=0)
					   {
						   
						   
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroLampadine; i++)
						   {
							   if (vetPidBulb[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   char informazioni[500];
						   
						   printf("STO LEGGENDO SU STO FD %d \n", fdLampadineComandi[indice]);
						   
						   if(read(fdLampadineComandi[indice], informazioni, 500)==-1)
						   {
							   printf("errore lettura pipe lampadina\n");
						   }
						   
						   printf("ho letto %s \n", informazioni);
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							   printf("errore scrittura \n");
						   }
						   
						   
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
			  
		  }
		  
		  
		  printf("sono a fine ciclo \n");
		  
		  if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
	            
	            if(communication[0]=='0')
			    {
					printf("ho inviato il segnale 2 \n");
					
					kill(getppid(),SIGUSR2);
				}else
				{
					printf("ho inviato il segnale 1 \n");
					kill(getppid(),SIGUSR1);
				}
				
				sem_post(semaforoFifoComandi);
				
		}else if(comand=='4')
		{
			
			
			char nome[]="hub";
			
			char statoS[]="spento";
			  
			if(stato==1)
			{
		        strcpy(statoS, "acceso");
			}
			
			char informazioni[100]; 
		   
		    sprintf(informazioni, "%s %d %s \n", nome, id, statoS);
		    
		     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
		    
		    
		    if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
		   
				
				if(num>0)
				{
					if(write(pipeComandi, "dispositivi connessi hub \n", strlen("dispositivi connessi hub \n"))==-1)
	                {
				      printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				      return EXIT_FAILURE;
	                }
					
					sem_post(semaforoFifoComandi);
					
					//cicli operativi 
					
					  //cicli operativi
				   
				  if(numeroLampadine>0)//lampadine
				  {  
					  printf("entro in lampadine \n");
			   
			         checkValue=0;
			    
			         for(int i=0; i<(numeroLampadine); i++)
			         {
					    kill(vetPidBulb[i], SIGUSR1);
			         }
			         
			         
			
			 
			
			         while(checkValue<numeroLampadine)
			         {
			         }
			         
			         char informazioni[500];
			    
			         for(int i=0; i<numeroLampadine; i++)
			         {
						 
						 
						 if(read(fdLampadineComandi[i], informazioni, 500)==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
						  
					 }
			     
			     
			     }
				  
				   //frigoriferi
				   
				 if(numeroFrigoriferi>0)//lampadine
				  {  
			   
			         checkValue=0;
			    
			         for(int i=0; i<(numeroFrigoriferi); i++)
			         {
					    kill(vetPidFridge[i], SIGUSR1);
			         }
			         
			         
			          char informazioni[500];
			 
			
			         while(checkValue<numeroFrigoriferi)
			         {
			         }
			    
			         for(int i=0; i<numeroLampadine; i++)
			         {
						
						 
						 if(read(fdFrigoriferiComandi[i], informazioni, 500)==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
						  
					 }
			     
			     
			     }  
				   
				   
				   //finestre
				  if(numeroFinestre>0)//lampadine
				  {  
			   
			         checkValue=0;
			    
			         for(int i=0; i<(numeroFinestre); i++)
			         {
					    kill(vetPidWindow[i], SIGUSR1);
			         }
			         
			         
			
			         char informazioni[500];
			
			         while(checkValue<numeroFinestre)
			         {
			         }
			    
			         for(int i=0; i<numeroFinestre; i++)
			         {
						 
						 
						 if(read(fdFinestreComandi[i], informazioni, 500)==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
						  
					 }
			     
			     
			     } 
				   //tende
				  if(numeroTende>0)//lampadine
				  {  
			   
			         checkValue=0;
			    
			         for(int i=0; i<(numeroTende); i++)
			         {
					    kill(vetPidTenda[i], SIGUSR1);
			         }
			         
			         
			         char informazioni[500];
			 
			
			         while(checkValue<numeroTende)
			         {
			         }
			    
			         for(int i=0; i<numeroTende; i++)
			         {
						 char informazioni[100];
						 
						 if(read(fdTendeComandi[i], informazioni, 500)==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
						  
					 }
			     
			     
			     } 
				   //termostati
				  if(numeroTermostati>0)//lampadine
				  {  
			   
			         checkValue=0;
			    
			         for(int i=0; i<(numeroTermostati); i++)
			         {
					    kill(vetPidTermostato[i], SIGUSR1);
			         }
			         
			         
			
			         char informazioni[500];
			
			         while(checkValue<numeroTermostati)
			         {
			         }
			    
			         for(int i=0; i<numeroTermostati; i++)
			         {
						 
						 
						 if(read(fdTermostatiComandi[i], informazioni, 500)==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
						  
					 }
			     
			     
			     } 
				   //timer
				  if(numeroTimer>0)//lampadine
				  {  
			   
			         checkValue=0;
			    
			         for(int i=0; i<(numeroTimer); i++)
			         {
					    kill(vetPidTimer[i], SIGUSR1);
			         }
			         
			         char informazioni[500];
			
			 
			
			         while(checkValue<numeroTimer)
			         {
			         }
			    
			         for(int i=0; i<numeroTimer; i++)
			         {
						 
						  
						  
						 if(read(fdTimerComandi[i], informazioni, 500)==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
						  
					 }
			     
			     
			     } 
				   //hub  
				  if(numeroHub>0)//lampadine
				  {  
			   
			         checkValue=0;
			    
			         for(int i=0; i<(numeroHub); i++)
			         {
					    kill(vetPidHub[i], SIGUSR1);
			         }
			         
			         char informazioni[1000];
			 
			
			         while(checkValue<numeroHub)
			         {
			         }
			    
			         for(int i=0; i<numeroHub; i++)
			         {
						 
						 
						 
						 if(read(fdHubComandi[i], informazioni, 1000)==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE IN PROCESSO INIZIALE\n");
						   getchar();
						   return EXIT_FAILURE;
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
						  
					 }
			     
			     
			     }
				  
				  
					
					
					
				}
				
			    
	            
				kill(getppid(), SIGUSR1);
				sem_post(semaforoFifoComandi);	
			
		}else if(comand=='0')//c'è gia stato un feedback positivo
		{
			printf("individuazione già avvenuta \n");
			
			if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            printf("lampadina %d invia segnale al padre \n", id);
				sem_post(semaforoFifoComandi);
				kill(getppid(), SIGUSR1);
				
			
		}
		
			
			close(fdFifoComandi);//chiudo la fifo comandi		
         		 lettura=0; //ripristino lettura
         		  strcpy(communication, "XXXXXXXXX");//ripristino communication
		}
		
		
		
		
		
		//se un segnale esterno cambia l'interruttore lo stato della lampadina si adegua
		if((stato==0) && (strcmp(interruttore, "ON")==0))
		{
		    stato=1;
			
	         //avvisa a tutti i dispositivi sottostanti di cambiare interruttore
	    
	    char statoStringa[]="0";
									
									sprintf(statoStringa, "%d", stato);
	    
	   if(numeroLampadine>0)
	    {
			
	   
	    
	    
	    for(int i=0; i<numeroLampadine;i++)
	    {
			write(fdLampadineInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    
	    for(int i=0; i<numeroLampadine;i++)
	    {
			kill(vetPidBulb[i], SIGINT);
			printf("CENTRALINA AVVISATA LAMPADINA\n");
		}
		
		
		
	    }
		
		
		if(numeroFrigoriferi>0)
		{
		
		
	    
	    for(int i=0; i<numeroFrigoriferi;i++)
	    {
			write(fdFrigoriferiInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    for(int i=0; i<numeroFrigoriferi;i++)
	    {
			kill(vetPidFridge[i], SIGINT);
		}
		
		
		
	    }
		
		if(numeroFinestre>0)
		{
		
		for(int i=0; i<numeroFinestre;i++)
	    {
			write(fdFinestreInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    
	    
	    for(int i=0; i<numeroFinestre;i++)
	    {
			kill(vetPidWindow[i], SIGINT);
		}
		
		
		
	    }
		
		if(numeroTende>0)
		{
		
		for(int i=0; i<numeroTende;i++)
	    {
			write(fdTendeInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    for(int i=0; i<numeroTende;i++)
	    {
			kill(vetPidTenda[i], SIGINT);
		}
		
		
		
	}
	
	    if(numeroTermostati>0)
	    {
		
		for(int i=0; i<numeroTermostati;i++)
	    {
			write(fdTermostatiInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    for(int i=0; i<numeroTermostati;i++)
	    {
			kill(vetPidTermostato[i], SIGINT);
		}
		
		
	}
	
	    if(numeroTimer>0)
	    {
		
		for(int i=0; i<numeroTimer;i++)
	    {
			write(fdTimerInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    for(int i=0; i<numeroTimer;i++)
	    {
			kill(vetPidTimer[i], SIGINT);
		}
		
		
	    }
	    
	    if(numeroHub>0)
	    {
		
		for(int i=0; i<numeroHub;i++)
	    {
			write(fdHubInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    for(int i=0; i<numeroHub;i++)
	    {
			kill(vetPidHub[i], SIGINT);
		}
		
		
		
	   }
									
	      
	      //check mirroring
	      if((connected==1)&&(flagEsterno==1))
	      {
	      pipePadre=atoi(argv[4]);
		
		char statoString[]="0";
		
		sprintf(statoString, "%d", stato);
		
		write(pipePadre, statoString, strlen(statoString));
		
		write(pipePadre, argv[1], strlen(argv[1]));
		
		kill(getppid(), SIGIO);
		
		printf("HO INVIATO SEGNALE ALLA CENTRALINA \n");
		}
	  }else if((stato==1) && (strcmp(interruttore, "OFF")==0))
		{
			printf("HUB E ENTRATO NEL CAMBIO STATO \n");
			getchar();
			
			stato=0;
		
		
		char statoStringa[]="0";
									
									sprintf(statoStringa, "%d", stato);
			
			 //avvisa a tutti i dispositivi sottostanti di cambiare interruttore
			 
	if(numeroLampadine>0)
	    {
			
	   
	    
	    
	    for(int i=0; i<numeroLampadine;i++)
	    {
			write(fdLampadineInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    
	    for(int i=0; i<numeroLampadine;i++)
	    {
			kill(vetPidBulb[i], SIGINT);
			printf("CENTRALINA AVVISATA LAMPADINA\n");
		}
		
		
		
	    }
		
		
		if(numeroFrigoriferi>0)
		{
		
		
	    
	    for(int i=0; i<numeroFrigoriferi;i++)
	    {
			write(fdFrigoriferiInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    for(int i=0; i<numeroFrigoriferi;i++)
	    {
			kill(vetPidFridge[i], SIGINT);
		}
		
		
		
	    }
		
		if(numeroFinestre>0)
		{
		
		for(int i=0; i<numeroFinestre;i++)
	    {
			write(fdFinestreInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    
	    
	    for(int i=0; i<numeroFinestre;i++)
	    {
			kill(vetPidWindow[i], SIGINT);
		}
		
		
		
	    }
		
		if(numeroTende>0)
		{
		
		for(int i=0; i<numeroTende;i++)
	    {
			write(fdTendeInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    for(int i=0; i<numeroTende;i++)
	    {
			kill(vetPidTenda[i], SIGINT);
		}
		
		
		
	}
	
	    if(numeroTermostati>0)
	    {
		
		for(int i=0; i<numeroTermostati;i++)
	    {
			write(fdTermostatiInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    for(int i=0; i<numeroTermostati;i++)
	    {
			kill(vetPidTermostato[i], SIGINT);
		}
		
		
	}
	
	    if(numeroTimer>0)
	    {
		
		for(int i=0; i<numeroTimer;i++)
	    {
			write(fdTimerInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    for(int i=0; i<numeroTimer;i++)
	    {
			kill(vetPidTimer[i], SIGINT);
		}
		
		
	    }
	    
	    if(numeroHub>0)
	    {
		
		for(int i=0; i<numeroHub;i++)
	    {
			write(fdHubInterruttore[i], statoStringa, strlen(statoStringa)+1);
		}
	    
	    for(int i=0; i<numeroHub;i++)
	    {
			kill(vetPidHub[i], SIGINT);
		}
		
		
		
	   }
									
			
			//check mirroring
			if((connected==1)&&(flagEsterno==1))
			{
			pipePadre=atoi(argv[4]);
		
		char statoString[]="0";
		
		sprintf(statoString, "%d", stato);
		
		write(pipePadre, statoString, strlen(statoString));
		
		write(pipePadre, argv[1], strlen(argv[1]));
		
		kill(getppid(), SIGIO);
		
		printf("HUB HO INVIATO SEGNALE ALLA CENTRALINA \n");
		}
	}
		
		printf("COMPLETATO \n");
	}
	
	return 0;
}

void handlerCambiaInterruttore(int signum)
{
	
    printf("hub riceve cambio interruttore \n");	
	
	if(flagEsterno==0)
	{
		printf("hub dentro a lettura di valore interruttore \n");
		
	char value[]="X";
	
	printf("DEVO LEGGER QUESTO FD %d \n", fdPadreInterruttore);
	
	if(read(fdPadreInterruttore, value, strlen(value)+1)==-1)
	{
		printf("ERRORE \n");
	}
	
	printf("il valore interruttore %s \n", value);
	
	if(strcmp(value, "0")==0)
	{
		strcpy(interruttore, "OFF");
		printf("il mio interruttore è ora a %s \n", interruttore);
	}else
	{
		strcpy(interruttore, "ON");
		printf("il mio interruttore è ora a %s \n", interruttore);
	}
    
    }else
    {
		if(strcmp(interruttore, "ON")==0)
	{
		strcpy(interruttore, "OFF");
		printf("il mio interruttore è ora a %s \n", interruttore);
	}else
	{
		strcpy(interruttore, "ON");
	}
	}	
   
	 
	 
}


//implementazione dell handler per il segnale utente 1.
void handlerLetturaPipe(int signum)
{
	
	lettura=1;
	//rispetto alla lampadina aggiungiamo checvalue come con la centralina che serve a gestire cicli di feedback
	checkValue=checkValue+1;
	
}

void handlerMirroring(int signum, siginfo_t *siginfo, void *context)
{
	
	
	//aumento checkvalue mirroring così da indicare alla centralina che c'e stato un mirroring
	checkValueMirroring=checkValueMirroring+1;
	
	//creo un  nuovo vettore dinamico di pid o
    pid_t *pidSenderMirroring1=(pid_t *)malloc(checkValueMirroring*sizeof(pid_t));
	for(int i=0; i<(checkValue-1); i++)//copio i vecchi rifermenti del vettore
	{
		pidSenderMirroring1[i]=pidSenderMirroring[i];
	  }
	
	if(checkValueMirroring>0)
	{			
	   //libero la memori puntata in precedenza dal vettore 
	   free(pidSenderMirroring);
	}
	
				
	 //scambio i riferimenti tra l originale vettore pid  e quell nuovo creato
	pidSenderMirroring=pidSenderMirroring1;
				
    //ottengo il pid dell segnale nell ultimo indice del vettor
    pidSenderMirroring[checkValueMirroring-1]=siginfo->si_pid;
	            
	
	//aumento il checkvalue1 che serve a indicare l'avvenuto mirroring
	checkValue1=checkValue1+1;
	
	
}

void handlerAck(int signum, siginfo_t *siginfo, void *context)
{
	//ottiene il pid del processo che ha generato il segnale
	 pidSender=siginfo->si_pid;
	 checkValue=checkValue+1;//aumenta il checkvalue nel caso l handler venga usato in un ciclo di feedback

}
