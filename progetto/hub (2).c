/*
 * hub
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
//funzione che gestisce una terminazione non critica
void terminazione(pid_t* vetPidBulb, int numeroLampadine, pid_t* vetPidWindow, int numeroFinestre, pid_t* vetPidFridge, int numeroFrigoriferi, pid_t* vetPidTenda, int numeroTende, pid_t* vetPidTermostato, int numeroTermostati, pid_t* vetPidTimer, int numeroTimer, pid_t* vetPidHub, int numeroHub);
//funzione che gestisce terminazione critica
void terminazioneFatale(pid_t* vetPidBulb, int numeroLampadine, pid_t* vetPidWindow, int numeroFinestre, pid_t* vetPidFridge, int numeroFrigoriferi, pid_t* vetPidTenda, int numeroTende, pid_t* vetPidTermostato, int numeroTermostati, pid_t* vetPidTimer, int numeroTimer, pid_t* vetPidHub, int numeroHub);

//handler
//handler per il segnale utente  cambia il valore dell'interruttore .
void handlerCambiaInterruttore(int signum);
//gestore per il segnale utente  che permette il continuo dell esecuzione
void handlerLetturaPipe(int signum);
//gestore mirroring
void handlerMirroring(int signum, siginfo_t *siginfo, void *context);
//gestore per il segnale di delete e altri segnali che richiedono un ack
void handlerAck(int signum, siginfo_t *siginfo, void *context);
//gestione terminazione
void handlerTerminazione(int signum, siginfo_t *siginfo, void *context);
//gestione tool esterno
void handlerLetturaFifoTool(int signum);


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
int pipeComandi;
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
//esecuzione dei cicli operativi
//variabile che se settata a 1 termina il ciclo operativo
int ext=0;	
int extErrore=0;
	
int main(int argc, char **argv)
{
	//VARIABILI PRINCIPALI DEL DISPOSITIVO
	//variabile che rappresenta stato
    int stato;
    //id del dispositivo
	int id;
	
	
	
	
	//VARIABILII DI SERVIZIO

    //informazioni relative ai dispositivi connessi
   
   //lampadine
	int numeroLampadine=0;//numero lampadine connesse inizialmente a 0
	pid_t *vetPidBulb=(pid_t *)malloc(numeroLampadine*sizeof(pid_t));//vettore dei pid delle lampadine figlie
    int *fdLampadineMirroringLettura=(int *)malloc(numeroLampadine*sizeof(int));//vettore di  pipe con le lampadine che consente la lettura delle info di mirroring
    int *fdLampadineComandi=(int *)malloc(numeroLampadine*sizeof(int));// vettore di pipe con le lampadine che consente la propagazione output comandi
	int* fdLampadineInterruttore=(int *)malloc(numeroLampadine*sizeof(int));//vettore di pipe in scrittura tramite cui svolgere le operazioni di setting dell interruttore dei figli
	
	//lo stesso per tutti gli altri dispositivi
	
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
    
    //fd pipe per inviare informazioni al padre sullo stato dispositivo
    int pipePadre;
   	
   	
	//inseriamo qui la inizializzazione di sto affare se no da errore
	pidSenderMirroring=(pid_t *)malloc(checkValueMirroring*sizeof(pid_t));
    
   	
   	int numeroMirroring=0;//numero dei dispositivi con valori anomali o modificati manualmente
   	int *vettoreMirroringId=(int *)malloc(numeroMirroring*sizeof(int));//vettore che contiene gli id dei dispositivi con valori anomali
   	int *vettoreMirroringStato=(int *)malloc(numeroMirroring*sizeof(int));//vettore che contiene lo stato dei dispositivo con valori anomali
   	   
    char **vettoreMirroringMessaggio;//vettore che contiene alcune informazioni relative allo stato dei dispositivi che hanno subito variazioni esterne
   	vettoreMirroringMessaggio=malloc(numeroMirroring*sizeof(char*));
   	  
   
	//per la gestione comandi dalla centralina
      
  
    char* fifoComandi="./fifoComandi";  //stringa del percorso fifo che contiene i comandi
	int fdFifoComandi; // fd della fifo di cui sopra per quando viene aperta
	char communication[]="XXXXXXXXX";//vettore usato per la comunicazione
	char comand;//comando estratto dal vettore comunicazione
	char pos;//posizione dell interruttore estratto dal vettore comunicazione se serve
	fdFifoComandi=open(fifoComandi,O_RDWR);
		if (fdFifoComandi==-1)
		{
			printf("ERRORE APERTURA FIFO COMANDI DA PARTE DI HUB \n");
		    printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		    perror("");
		    getchar();
		    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		    return EXIT_FAILURE;
		}
	
    //apro o creo il semaforo per la lettura dalla fifo comandi
	sem_t* semaforoFifoComandi=sem_open("IdCm", O_CREAT, 0666, 0);
	if(semaforoFifoComandi==-1)
	{
		printf("ERRORE APERTURA SEMAFORO COMANDI DA PARTE DI HUB \n");
		printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		perror("");
		getchar();
		terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
        return EXIT_FAILURE;
	}
	
	//per la gestione comandi del tool esterno
	char* fifoTool="./fifoTool";
	int fdFifoTool = open(fifoTool, O_RDWR);
    if (fdFifoTool==-1)
	{
		printf("ERRORE APERTURA FIFO COMANDI DA PARTE DI HUB \n");
		printf("ERRORE FATALE. ");
		perror("");
		getchar();
		terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		return EXIT_FAILURE;
	}
	
	//per passare eventuali eliminazioni di dispositivi al tool esterno
	char* fifoIdEsterno="./fifoIdEsterno";
	int fdFifoIdEsterno;//quando apri la fifo la metti qui
	fdFifoIdEsterno=open(fifoIdEsterno, O_RDWR);
	if(fdFifoIdEsterno==-1)
    {
		printf("ERRORE APERTURA FIFO ID ESTERNO DA PARTE DI HUB POTREBBE ESSER IMPOSSIBILE IL SUO UTILIZZO \n");
		perror("");
		getchar();
			
	 }

	
	//gestione segnali		  

	
	
	//handler per la gestione del ack
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_sigaction=&handlerAck;
	sa.sa_flags=SA_RESTART|SA_SIGINFO;
	sigaction(SIGUSR2, &sa, NULL);
	
	//imposto l'handler del mirroring
	struct sigaction sa1;
	memset(&sa1, '\0', sizeof(sa1));
	sa1.sa_sigaction=&handlerMirroring;
	sa1.sa_flags=SA_RESTART|SA_SIGINFO;
	sigaction(SIGIO, &sa1, NULL);
	
	
	//handler del cambio interruttore
	
   struct sigaction sa2;
	sa2.sa_handler=handlerCambiaInterruttore;
	sa2.sa_flags=SA_RESTART;
	sigaction(SIGINT, &sa2, NULL);
	
	//handler lettura pipe
	struct sigaction sa3;
	sa3.sa_handler=handlerLetturaPipe;
	sa3.sa_flags=SA_RESTART;
	sigaction(SIGUSR1, &sa3, NULL);

	//handler terminazione
	struct sigaction sa4;
	memset(&sa4, '\0', sizeof(sa4));
	sa4.sa_sigaction=&handlerMirroring;
	sa4.sa_flags=SA_RESTART|SA_SIGINFO;
	sigaction(SIGTERM, &sa4, NULL);
	
	//handler gestione interazione tool esterno
	struct sigaction sa5;
	sa5.sa_handler=handlerLetturaFifoTool;
	sa5.sa_flags=SA_RESTART;
	sigaction(SIGURG, &sa5, NULL); 

	
	  //dichiarazione della maschera per l'attesa
    sigset_t maschera;
    //il set viene riempito con tutti i segnali di sistema
    sigfillset(&maschera);
    // con questa funzione tolgo il segnale SIGUSR1 che mi gestisce la lettura nella fifo comandi
    sigdelset(&maschera, SIGUSR1);
    //con questa funzione tolgo il segnale SIGUSR2 che mi gestisce un ack per alcuni comandi specifici
    sigdelset(&maschera, SIGUSR2);
     //con questa funzione tolgo il segnale SIGTERM che mi gestisce una terminazione esterna
    sigdelset(&maschera, SIGTERM);
    //con questa funzione tolgo il segnale SIGKILL che mi gestisce una terminazione fatale, nel caso ci fosse la rivolta delle macchine
    sigdelset(&maschera, SIGKILL);
    //con questa funzione tolgo il segnale SIGIO nella ricezione del mirroring
    sigdelset(&maschera, SIGIO);
      //con questa funzione tolgo il segnale SIGINT che mi gestisce il cambio valore interruttore
    sigdelset(&maschera, SIGINT);
    //con questa funzione tolgo il segnale SIGURG che mi gestisce un comando dal tool esterno
    sigdelset(&maschera, SIGURG);
    
    
    //INIZIO OPERAZIONI PER IMPOSTARE L'HUB
    if(strcmp(argv[2], "standard")==0)
	{
		//reperimento dell'id dagli argomenti del programma
		//conto le x nel valore passato come argomento da riga di comando
		int counter=3;//parto da tre (solo valori numerici passati a riga di comando)
		for(int i=0; i<3; i++)
		{
			if(argv[1][i]=='X')
			{
			  counter=counter-1;//se individuo una X decremento di uno il contatore
			}
		}
		char idStringa[counter];//genero una stringa con lunghezza pari al valore numerico pasato
		for(int i=0; i<(3-counter); i++)
		{
			idStringa[i]=argv[1][i];//inizializzo la stringa suddetta con i soli valori numerici trovati nella riga di comando
		}
		id=atoi(idStringa);//inserisco in id la conversione in intero del vettore di sopra
	 
        pipePadre=atoi(argv[4]);
        
        pipeComandi=atoi(argv[5]);
    
        fdPadreInterruttore=atoi(argv[6]);
    
    
    }else if (strcmp(argv[2], "linked")==0)
    {
	
		
	}
    
    
    //VERI E PROPRI CICLI OPERATIVI
    while(ext!=1)//finche ext non è settato a 1 continuo ciclo
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
						printf("ERRORE LETTURA PIPE MIRRORING LAMPADINE DA PARTE DI HUB \n");
						printf("ERRORE FATALE \n");
						perror("");
						getchar();
		                terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
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
						
						printf("ERRORE LETTURA PIPE MIRRORING FINESTRE DA PARTE DI HUB \n");
						printf("ERRORE FATALE \n");
						perror("");
						getchar();
						terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
				
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
						
						printf("ERRORE LETTURA PIPE MIRRORING FRIGORIFERI DA PARTE DI HUB \n");
						printf("ERRORE FATALE \n");
						perror("");
						getchar();
						terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
						
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
						
						printf("ERRORE LETTURA PIPE MIRRORING HUB DA PARTE DI HUB \n");
						printf("ERRORE FATALE \n");
						perror("");
						getchar();
						terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
						
						
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
						
						printf("ERRORE LETTURA PIPE MIRRORING TIMER DA PARTE DI HUB \n");
						printf("ERRORE FATALE \n");
						perror("");
						getchar();
						terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
						
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
						
						printf("ERRORE LETTURA PIPE MIRRORING TERMOSTATI DA PARTE DI HUB \n");
						printf("ERRORE FATALE \n");
						perror("");
						getchar();
		                terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
						
						
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
						
						printf("ERRORE LETTURA PIPE MIRRORING TENDE DA PARTE DI HUB \n");
						printf("ERRORE FATALE \n");
						perror("");
						getchar();
						terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
						
						
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
				
		
		
		   sem_wait(semaforoFifoComandi);
		
		   if(read(fdFifoComandi, communication, strlen(communication))==-1)
		   {
			printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		    printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		    perror("");
		    getchar();
		    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		    
		    
		    return EXIT_FAILURE;
		   }
		
		
		   //scomposizione del comando
		   comand=communication[0];		
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
		
			
		   if(comand=='2')//implementazione di del
		   {
			  //converto l'id
			  int idDaVerificare=atoi(id1);
			
			  if(id==idDaVerificare)//se l'id da verificare è quello corretto
			  {
				  strcpy(communication, "0XXXXXXXX");//copio la stringa su communication che indica che è stato trovato l'id giusto
				  if(write(fdFifoComandi, communication, strlen(communication))==-1)
	              {
				     printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		             printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		             perror("");
		             getchar();
		          
					 terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		          
		          
		             return EXIT_FAILURE;
	              }
	            
	            
	              //termino tutti i figli
	            
	              if(numeroLampadine>0)
	              {
			
	  
	    
	                 for(int i=0; i<numeroLampadine;i++)
	                 {
			             kill(vetPidBulb[i], SIGTERM);
		             }
		
				
	             }
		
		
		        if(numeroFrigoriferi>0)//stessa cosa frigoriferi
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
				
			}else//replico comando su tutti le lampadine riscrivendolo sulla fifo comandi
			{ //al ritorno di tutte le lampadine leggo la fifo comandi. se ho trovato l'id tra le lampadine figlie rimuovo dal vettore dei figli altrimenti continuo
				
			    sem_post(semaforoFifoComandi);
						
			    if(numeroLampadine>0)
			    {
			
			        //riscrivo sulla piipe
				   if(write(fdFifoComandi, communication, strlen(communication))==-1)
			       {
				      printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
	                  printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		              perror("");
		              getchar();
					  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		              
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
			       
			       			       
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
					  printf("ERRORE FATALE. ");
					  perror("");
					  getchar();
					  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
					  return EXIT_FAILURE;
		           }
			       
			  }
			       
			  if(communication[0]!='0')//a questo punto coi frigoriferi e via cosi
			 {
					   			
			      if(numeroFrigoriferi>0)
			      { 
			
					   //riscrivo sulla piipe
				       if(write(fdFifoComandi, communication, strlen(communication))==-1)
			           {
				           printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                   perror("");
		                   getchar();
						   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                   
		                   return EXIT_FAILURE;
			           }
				 
				       checkValue=0;
				 
				       for(int i=0; i<(numeroFrigoriferi); i++)
			           {
				  	      kill(vetPidFridge[i], SIGUSR1);
			           }
			
			            
			
			            while(checkValue<numeroFrigoriferi)
			            {
				           
			            } 
			       
			           if(read(fdFifoComandi, communication, strlen(communication))==-1)
		               {
			               printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                   perror("");
		                   getchar();
						   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
            
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
								printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		            		    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
            
		                        
		                        return EXIT_FAILURE;
			                }
			              
			                checkValue=0;
				 
				            for(int i=0; i<(numeroFinestre); i++)
			                {
				  	           kill(vetPidWindow[i], SIGUSR1);
			                }
			
			              
			                while(checkValue<numeroFinestre)
			                {
			                } 
			       
			                if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                    {
			                  printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                      printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                      perror("");
		                      getchar();
		               		  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
       
		                      
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
				                   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
		                		   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
           
		                           
		                           return EXIT_FAILURE;
			                    }
				 
				                checkValue=0;
				 
				              for(int i=0; i<(numeroTimer); i++)
			                  {
				  	             kill(vetPidTimer[i], SIGUSR1);
			                  }
			
			                  
			
			                  while(checkValue<numeroTimer)
			                  {
				                 
			                   } 
			       
			                  if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                      {
			                      printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                          printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                          perror("");
		                          getchar();
		                          terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
  
		                          
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
				                   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
		                           
		                      	   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
     
		                           
		                           return EXIT_FAILURE;
			                     }
				 
			 	                 checkValue=0;
				 
				                 for(int i=0; i<(numeroHub); i++)
			                     {
				  	                kill(vetPidHub[i], SIGUSR1);
			                     }
			
			
			                     while(checkValue<numeroHub)
			                     {      
								  
			                     } 
			     
			                     if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                         {
			                        printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                            printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                            perror("");
		                            getchar();
		                            
									terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
               
		                            
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
										   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                                   perror("");
		                                   getchar();
		                     		       terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
          
		                               
		                                   return EXIT_FAILURE;
				                 
			                           }
				 
				                       checkValue=0;
				 
				                       for(int i=0; i<(numeroTende); i++)
			                           {
				  	                        kill(vetPidTenda[i], SIGUSR1);
			                           }
			
			
			                           while(checkValue<numeroTende)
			                           {    
								  
			                           } 
			     
			                           if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                               {
										   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                                   perror("");
		                                   getchar();
		                              
		                         		   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
     
		                              
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
											 printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                                     printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                                     perror("");
		                                     getchar();
		                                  
		                          		     terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
        
		                                  
		                                     return EXIT_FAILURE;
			                             }
				 
				                         checkValue=0;
				 
				                         for(int i=0; i<(numeroTermostati); i++)
			                             {
				  	                           kill(vetPidTermostato[i], SIGUSR1);
			                             }
			
			
			                              while(checkValue<numeroTermostati)
			                              {    
								  
			                              } 
			     
			                              if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                                  {
											  
											  printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                                      printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                                      perror("");
		                                      getchar();
		                            		  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
          
		                                      
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
						   
						                    //cerco l'indice incrimnato nel vettore termostati
						                    for(int i=0; i<numeroTermostati; i++)
						                    {
							                    if (vetPidTermostato[i]==pidSender)
							                    {
								                    indice=i;
							                    }
						                    }
						   
						                     //invio sigterm al processo in questione
						                     kill(vetPidTermostato[indice], SIGTERM);
						   
						                     //alloco un nuovo vettore termostati
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
						                      
						                      if(numeroTermostati>0)
						                      {
												  free(vetPidTermostato);
											  }
						                      
						                       //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						                       vetPidTermostato=vetPidTermostato1;
						   
						                      numeroTermostati=numeroTermostati-1;
					                      }
					   
					                      //ripristino pidSender
					                      pidSender=0;
						  
						  
						  
					                 }	 
					      
						   
					  }else{//la stessa cosa viene fatta in seguito
						  
						  if (pidSender!=0)
					      {
						     int indice;
						   
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
						   
						     if(numeroTende>0)
						     {
				                 free(vetPidTenda);
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
						   int indice;
						   
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
						   
						    if(numeroHub>0)
						     {
				                 free(vetPidHub);
				             }
						   
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidHub=vetPidHub1;
						   
						   
						   
						   numeroHub=numeroHub-1;
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   
			        }
			                   
		                           
								  
	              }else{
					  
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
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
						   
						    if(numeroTimer>0)
						     {
				                 free(vetPidTimer);
				             }
						   
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidTimer=vetPidTimer1;
						   
						   numeroTimer=numeroTimer-1;
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
				}else{
					   
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
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
						   
						    if(numeroFinestre>0)
						     {
				                 free(vetPidWindow);
				             }
						   
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidWindow=vetPidWindow1;
						   
						   numeroFinestre=numeroFinestre-1;
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
					   
					   
				}else{
					  
					   if (pidSender!=0)
					   {
						   //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
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
						   
						    if(numeroFrigoriferi>0)
						     {
				                 free(vetPidFridge);
				             }
						   
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidFridge=vetPidFridge1;
						   
						   
						   numeroFrigoriferi=numeroFrigoriferi-1;
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   } 
					   
					   
					   
					   
			    }else{
					  
					   if (pidSender!=0)
					   {
						   int indice;
						   
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
						   
						    if(numeroLampadine>0)
						     {
				                 free(vetPidBulb);
				             }
						   
						   //inserisco nel puntatore del vettore originario l'inidirizzo del nuovo vettore
						   vetPidBulb=vetPidBulb1;
						   
						   numeroLampadine=numeroLampadine-1;
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
						   
					       
				
				
				
				
				//replico il comando
				if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				     printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		             printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		             perror("");
		             getchar();
		            
				    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
            
		            
		             return EXIT_FAILURE;
	            }
	            
	            //avviso il padre
				kill(getppid(), SIGUSR1);
			}
		}	
	    else if(comand=='6')//switch
		{
			
		  
          //deve legger anche label e po		  
		  char label=communication[4];
		  char pos=communication[8];
		  
		  
		  int idDaVerificare=atoi(id1);
		  
		  if(idDaVerificare==id)//riconosce il proprio id, la procedura è la solia
		  {
			 strcpy(communication, "0XXXXXXXX");
			 
			 if(label=='0')
			 {
				 
			 
			   if (pos=='0')
			   {
				   if(strcmp(interruttore, "ON")==0)
				   {
					  
					  //chiama segnale cambio interruttore
					  raise(SIGINT);
					  					  
				  }
				  
				  
			  }else 
			  {
				  
				  if(strcmp(interruttore, "OFF")==0)
				  {
					  
					  //chiama segnale cambio interruttore
					  raise(SIGINT);
					  
					  
				  }
				  
			  }
			  
		    }else//riconosciuto id giusto ma interruttore sbagliato
		    {
				
				strcpy(communication, "01XXXXXXX");
				
			}
			  
		  }else//replicazione comando se non riconosce id
		  {   
			  
			  if(numeroLampadine>0)
			  {
			
			     //riscrivo sulla piipe
			     if(write(fdFifoComandi, communication, strlen(communication))==-1)
			     {
				           printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                   perror("");
		                   getchar();
		                  
		      		       terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
            
		                  
		                   return EXIT_FAILURE;
			      }
				 
				 //conto i feedback di ritorno
			      checkValue=0;
				 
				   for(int i=0; i<(numeroLampadine); i++)
			       {
					   printf("AVVISATA \n");
				  	  kill(vetPidBulb[i], SIGUSR1);
			       }
			
			
			
			        sem_post(semaforoFifoComandi);
			
		            while(checkValue<(numeroLampadine))
			        {
				      
			        } 
			       
			      
			       			       
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			               printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                   perror("");
		                   getchar();
		                   
		                   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
      
		                   
		                   return EXIT_FAILURE;
		           }
			       
			       } 			       
			       if(communication[0]!='0')
				   {
					   if(numeroFrigoriferi>0)
					   {
					   //riscrivo sulla piipe
				       if(write(fdFifoComandi, communication, strlen(communication))==-1)
			           {
				           printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                   perror("");
		                   getchar();
		          		    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
         
		                   
		                   return EXIT_FAILURE;
			           }
                        
				 
				       for(int i=0; i<(numeroFrigoriferi); i++)
			           {
				  	      kill(vetPidFridge[i], SIGUSR1);
			           }
			
			             while(checkValue<(numeroFrigoriferi))
			            {
				          
				      
			              } 
			       
			       
			           if(read(fdFifoComandi, communication, strlen(communication))==-1)
		               {
			               printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                   perror("");
		                   getchar();
		                   
		           		    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
        
		                   
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
				             printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                     printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                     perror("");
		                     getchar();
		                     
		        		    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
             
		                     
		                     
		                     return EXIT_FAILURE;
			              }
				 
				           checkValue=0;
				           
				          for(int i=0; i<(numeroFinestre); i++)
			              {
				  	         kill(vetPidWindow[i], SIGUSR1);
			              }
			
			               while(checkValue<(numeroFinestre))
			              {
				             
				      
			               } 
			       
			       
			              if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                  {
			                    printf("ERRORE APERTURA FIFO COMANDI DA PARTE DI HUB \n");
								printf("ERRORE FATALE. ");
								perror("");
								getchar();
								terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
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
								 printf("ERRORE APERTURA FIFO COMANDI DA PARTE DI HUB \n");
								 printf("ERRORE FATALE. ");
								 perror("");
								 getchar();
								 terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
								 return EXIT_FAILURE;
			                  }
				 
				               checkValue=0;
				               
				              for(int i=0; i<(numeroTimer); i++)
			                  {
				  	             kill(vetPidTimer[i], SIGUSR1);
			                  }
			
			                   while(checkValue<(numeroTimer))
			                   {
				                 
				      
			                   } 
			       
			       
			                  if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                      {
			                      printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                          printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                          perror("");
		                          getchar();
								  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                    
		                          
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
				                   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
									printf("ERRORE FATALE. ");
									perror("");
									getchar();
									terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
									return EXIT_FAILURE;
			                   }
				               
				               checkValue=0;
				 
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
				                                 printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                                         printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                                         perror("");
		                                         getchar();
		                                         
		                                         terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
     
		                                         
		                                         return EXIT_FAILURE;
			                                  }
			                                  
			                                  checkValue=0;
				 
				                              for(int i=0; i<(numeroTende); i++)
			                                  {
				  	                              kill(vetPidTenda[i], SIGUSR1);
			                                  }
			
			
			                                   while(checkValue<(numeroTende))
			                                  {
				                                 
				      
			                                   } 
			       
			       
			                                   if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                                       {    
			                                      printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                                          printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                                          perror("");
		                                          getchar();
		                                  
		                                 		  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
         
		                                          
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
				                                     printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                                             printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                                             perror("");
		                                             getchar();
													terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                    
		                                             
		                                             return EXIT_FAILURE;
			                                      }
				                                  
				                                  checkValue=0;
				 
				                                  for(int i=0; i<(numeroTermostati); i++)
			                                      {
				  	                                  kill(vetPidTermostato[i], SIGUSR1);
			                                       }
			
			                                       while(checkValue<(numeroTermostati))
			                                      {
				                                     
				      
			                                       } 
			       
			       
			                                      if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                                          {      
			                                          printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                                              printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                                              perror("");
		                                              getchar();
		                                              
												      terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                     
		                                              return EXIT_FAILURE;
		                                           }
			       
		                                      }
								        } 
							       }
							  }
						  }
					   
					   
				    }
					   
					      
				   
				  
			
		      }
			  
			  
		  }
		  
		  //replico il comando
		  if(write(fdFifoComandi, communication, strlen(communication))==-1)
	       {
				  printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		          printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
				  perror("");
		          getchar();
		          terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		          
		          
		          return EXIT_FAILURE;
	       }
	            
	     sem_post(semaforoFifoComandi);
		 kill(getppid(), SIGUSR1);
				
		  
		  
		}else if(comand=='3')//comando list
		{
			
			int idDaVerificare=atoi(id1);
		  
		    if(idDaVerificare==id)
		    {
			   strcpy(communication, "0XXXXXXXX");
			 			 
			   char informazioni[3000];
			  
			   char statoAusiliare[]="spento";
			  
			   if(stato==1)
			   {
				  strcpy(statoAusiliare, "acceso");
			   }
			  
			
			   
			   char messaggio[2000];
		      sprintf(messaggio, "dispositivi anomali: %d %d", stato, id);
		      
		      if(numeroMirroring==0)
		      {
				  sprintf(messaggio, "%s nessuno", messaggio);
			  }
		      
		      for(int i=0; i<numeroMirroring; i++)
		      {
				  sprintf(messaggio, "%s id: %d stato: %d messaggio:%s", messaggio, vettoreMirroringId[i], vettoreMirroringStato[i], vettoreMirroringMessaggio[i]);
			  }
			  
				sprintf(informazioni, "hub %d %s %s", id, statoAusiliare, messaggio);
					 
					 
			    if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
			    {
				      printf("ERRORE SCRITTURA PIPE COMANDI DA PARTE DI HUB \n");
		              printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		              perror("");
		              getchar();
                      terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		              
		              
		              return EXIT_FAILURE;
			    }
			 
			 
						 
				 
			  
			  
		  }else//devo propagare il segnale come il processo iniziale
		  {
			 
			 sem_post(semaforoFifoComandi);
			  
			    if(numeroLampadine>0)
		        {
			
			 
		           if(write(fdFifoComandi, communication, strlen(communication))==-1)
		           {
					   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		               printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		               perror("");
		               getchar();
					   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		               
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
			       
			       
			       
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		              printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		              perror("");
		              getchar();
					  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		              
		              
		              return EXIT_FAILURE;
		           }
			       
			     }
			       
			     if(communication[0]!='0')
				 {
					   
			
			        if(numeroFrigoriferi>0)
			        {
			
					   //riscrivo sulla piipe
				       if(write(fdFifoComandi, communication, strlen(communication))==-1)
			           {
				           printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                   perror("");
		                   getchar();
						   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                   
		                   
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
			               printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                   perror("");
		                   getchar();
						   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                   
		                   
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
				              printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                      printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                      perror("");
		                      getchar();
						      terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                      
		                      
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
			                  printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                      printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                      perror("");
		                      getchar();
		      				  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                
		                      
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
				                printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
    		      				terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
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
			                      printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                          printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                          perror("");
		                          getchar();
		      				      terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                          
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
				                   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
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
			                       printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
								   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                          
		                          
		                          
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
				                   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
     		      				  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                           
		                           
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
			                      
				                   printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
								   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                           
		                           
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
				                   
				                   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
		   		      			   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                        
		                           
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
								   
				                   printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
		                           
		               		       terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
            
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
										   printf("ERRORE LETTURA PIPE TERMOSTATI COMANDI DA PARTE DI HUB \n");
		                                   printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                                   perror("");
		                                   getchar();
		                   		           terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                
		                                   
		                                   return EXIT_FAILURE;
						              }
						   
						   
						   
						   
						            if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						            {
										 printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                                 printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                                 perror("");
		                                 getchar();
		      		      				 terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                           
		                                 return EXIT_FAILURE;
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
								   printf("ERRORE LETTURA PIPE TENDE COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
		                           
		      		      		   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                     
		                           
		                           return EXIT_FAILURE;
							    }
						   
						   
						   
						   
								if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
								{
								   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
		                           
     		      				  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                           
		                           
		                           return EXIT_FAILURE;
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
							    printf("ERRORE LETTURA PIPE HUB COMANDI DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		                        
			      				terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
	                        
		                        
		                        return EXIT_FAILURE;
						   }
						   						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							    printf("ERRORE LETTURA FIFO COMANDI DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		      				    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
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
							    printf("ERRORE LETTURA PIPE COMANDI TIMER DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		                        
		      				  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
						   }
						   
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							    printf("ERRORE SCRITTURA PIPE COMANDI DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		                        
		      				    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
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
							    printf("ERRORE LETTURA PIPE COMANDI FINESTRE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		                        
		      				    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
						   }
						   
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							    printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		      				    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        
		                        return EXIT_FAILURE;
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
							   printf("ERRORE LETTURA PIPE COMANDI FRIGORIFERI DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		         		        terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
               
		                        
		                        return EXIT_FAILURE;
						   }
						   
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							   printf("ERRORE SCRITTURA FIFO COMANDI FINESTRE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
				      			terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                        
		                        
		                        
		                        return EXIT_FAILURE;
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

						   for(int i=0; i<numeroLampadine; i++)
						   {
							   if (vetPidBulb[i]==pidSender)
							   {
								   indice=i;
							   }
						   }
						   
						   char informazioni[500];
						   
						   
						   if(read(fdLampadineComandi[indice], informazioni, 500)==-1)
						   {
							    printf("ERRORE LETTURA PIPE COMANDI LAMPADINE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		 		      		    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                       
		                        
		                        return EXIT_FAILURE;
						   }
						   
						   
						   
						   
						   if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
						   {
							    printf("ERRORE SCRITTURA FIFO COMANDI  DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		 		      		    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                       
		                        
		                        
		                        return EXIT_FAILURE;
						   }
						   
						   
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
			  
		    }
		  
		  
		  
		    if(write(fdFifoComandi, communication, strlen(communication))==-1)//riscrivo il comando sulla fifo comandi
	        {
				  printf("ERRORE SCRITTURA FIFO COMANDI FINESTRE DA PARTE DI HUB \n");
		          printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		          perror("");
		          getchar();
		          
		      	  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		          
		          
		          return EXIT_FAILURE;
	        }
	            
	        if(communication[0]=='0')//se ero io il dispositivo che doveva stampare avviso il padre con un ack cosi che possa reperire le mie info sulla pipe
			{
					
					
					kill(getppid(),SIGUSR2);
		    }else//altrimenti lo avviso normalmente
		    {
					
					kill(getppid(),SIGUSR1);
		    }
				
		    sem_post(semaforoFifoComandi);
				
		}else if(comand=='4')//implementazione list
		{
			//semplicemente scrive le informazioni base
			
			char nome[]="hub";
			
			char statoS[]="spento";
			  
			if(stato==1)
			{
		        strcpy(statoS, "acceso");
			}
			
			char informazioni[100]; 
		   
		    sprintf(informazioni, "%s %d %s \n", nome, id, statoS);
		    
		    //scrivo sulla pipe del padre le info reperite
		     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
	         {
				 
				   printf("ERRORE SCRITTURA PIPE COMANDI PADRE DA PARTE DI HUB \n");
		           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		           perror("");
		           getchar();
		           
		      	   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		           
		           
		           return EXIT_FAILURE;
	         }
		    
		    
		    if(write(fdFifoComandi, communication, strlen(communication))==-1)
	        {
				  
	        }
		   
				
		    if(num>0)
		    {
					if(write(pipeComandi, "dispositivi connessi hub \n", strlen("dispositivi connessi hub \n"))==-1)
	                {
				       printf("ERRORE SCRITTURA PIPE PADRE DA PARTE DI HUB \n");
		               printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		               perror("");
		               getchar();
		               terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);

		               
		               
		               return EXIT_FAILURE;
	                }
					
					sem_post(semaforoFifoComandi);
					
					//cicli operativi 
					
					  //cicli operativi
				   
					if(numeroLampadine>0)//lampadine
					{	  
			   
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
								printf("ERRORE LETTURA PIPE COMANDI LAMPADINE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		      				  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
						        printf("ERRORE SCRITTURA PIPE COMANDI PADRE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
								terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);

		                        return EXIT_FAILURE;
					     }
						  
					 }
			     
			     
			     }
				  
				   //frigoriferi
				   
				 if(numeroFrigoriferi>0)
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
						    printf("ERRORE LETTURA PIPE COMANDI FRIGORIFERI DA PARTE DI HUB \n");
		                    printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                    perror("");
		                    getchar();
		      				terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);

		                    return EXIT_FAILURE;
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
						    printf("ERRORE SCRITTURA PIPE COMANDI PADRE DA PARTE DI HUB \n");
		                    printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                    perror("");
		                    getchar();
		      				terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                    
		                    
		                    return EXIT_FAILURE;
					     }
						  
					 }
			     
			     
			     }  
				   
				   
				   //finestre
				  if(numeroFinestre>0)
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
								printf("ERRORE LETTURA PIPE COMANDI FINESTRE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		      				    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
								printf("ERRORE SCRITTURA PIPE COMANDI PADRE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		      				    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
					     }
						  
					 }
			     
			     
			     } 
				   //tende
				  if(numeroTende>0)
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
						
						 
						 if(read(fdTendeComandi[i], informazioni, 500)==-1)
					     {
								printf("ERRORE LETTURA PIPE COMANDI TENDE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		      				    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
								printf("ERRORE SCRITTURA PIPE COMANDI PADRE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
 		      				  terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
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
								printf("ERRORE LETTURA PIPE COMANDI TERMOSTATI DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();

		      				    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
								printf("ERRORE SCRITTURA PIPE COMANDI PADRE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		      				    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
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
								printf("ERRORE LETTURA PIPE COMANDI TIMER DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();

		      				   terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
						        printf("ERRORE SCRITTURA PIPE COMANDI PADRE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
								terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                        
		                        
		                        return EXIT_FAILURE;
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
								printf("ERRORE LETTURA PIPE COMANDI HUB DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		  		      		    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                      
		                        
		                        return EXIT_FAILURE;
					     }
					      
					     if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
					     {
								printf("ERRORE LETTURA PIPE COMANDI PADRE DA PARTE DI HUB \n");
		                        printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                        perror("");
		                        getchar();
		                        
		     		      	    terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
                   
		                        
		                        return EXIT_FAILURE;
					     }
						  
					 }
			     
			     
			     }
				  
				  
					
					
					
				}
				
			    
	            
				kill(getppid(), SIGUSR1);
				sem_post(semaforoFifoComandi);	
			
		}else if(comand=='0')//c'è gia stato un feedback positivo
		{
			//semplicemente replico il comando
			if(write(fdFifoComandi, communication, strlen(communication))==-1)
	        {
				                   printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI HUB \n");
		                           printf("ERRORE FATALE. PREMERE UN PULSANTE PER USCIRE.\n");
		                           perror("");
		                           getchar();
		                           
		      				       terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		                           
		                           
		                           return EXIT_FAILURE;
	        }
	            
				sem_post(semaforoFifoComandi);
				kill(getppid(), SIGUSR1);
				
			
		}
		
			
			 lettura=0; //ripristino lettura
             strcpy(communication, "XXXXXXXXX");//ripristino communication
		}
		
		
		
		
		
		//se un segnale esterno cambia l'interruttore lo stato dell hub si adegua
		if((stato==0) && (strcmp(interruttore, "ON")==0))
		{
		    stato=1;
			
	         //avvisa a tutti i dispositivi sottostanti di cambiare interruttore
	    
	        char statoStringa[]="0";
			sprintf(statoStringa, "%d", stato);
	    
	        if(numeroLampadine>0)//se il numero delle lampadine è maggiore di zero
	        {
			
	   
	    
	    
	           for(int i=0; i<numeroLampadine;i++) //scrivo sulla pipe interruttore il nuovo stato
	           { 
			      write(fdLampadineInterruttore[i], statoStringa, strlen(statoStringa)+1);
		       }
	    
	    
	           for(int i=0; i<numeroLampadine;i++)//invio il segnale di cambio interruttore a tutte le lampadine
	           {
			      kill(vetPidBulb[i], SIGINT);
 		       }
		
		
		
	         }
		     //la stessa cosa viene ripetuta per tutti i dispositivi
		
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
									
	      
	      //se connesso e c'è stata una modifica esterna o una modifica dovuta a uno switch o un link quindi è stato letto un comando avviso il padre del cambio interruttore
	      if((connected==1)&&((lettura==1)||(flagEsterno==1)))
	      {
	           pipePadre=atoi(argv[4]);
		       char statoString[]="0";
		       sprintf(statoString, "%d", stato);		
		       write(pipePadre, statoString, strlen(statoString));	
		       write(pipePadre, argv[1], strlen(argv[1]));
		 	   kill(getppid(), SIGIO);
		   }
	  }else if((stato==1) && (strcmp(interruttore, "OFF")==0))
	  {
			stato=0;//adeguo lo stato
			
			//il resto è uguale a prima
			
			     //avvisa a tutti i dispositivi sottostanti di cambiare interruttore
	    
	        char statoStringa[]="0";
			sprintf(statoStringa, "%d", stato);
	    
	        if(numeroLampadine>0)//se il numero delle lampadine è maggiore di zero
	        {
			
	   
	    
	    
	           for(int i=0; i<numeroLampadine;i++) //scrivo sulla pipe interruttore il nuovo stato
	           { 
			      write(fdLampadineInterruttore[i], statoStringa, strlen(statoStringa)+1);
		       }
	    
	    
	           for(int i=0; i<numeroLampadine;i++)//invio il segnale di cambio interruttore a tutte le lampadine
	           {
			      kill(vetPidBulb[i], SIGINT);
 		       }
		
		
		
	         }
		     //la stessa cosa viene ripetuta per tutti i dispositivi
		
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
									
	      
	      //se connesso e c'è stata una modifica esterna o una modifica dovuta a uno switch o un link quindi è stato letto un comando avviso il padre del cambio interruttore
	      if((connected==1)&&((lettura==1)||(flagEsterno==1)))
	      {
	           pipePadre=atoi(argv[4]);
		       char statoString[]="0";
		       sprintf(statoString, "%d", stato);		
		       write(pipePadre, statoString, strlen(statoString));	
		       write(pipePadre, argv[1], strlen(argv[1]));
		 	   kill(getppid(), SIGIO);
		   }
		
	  }
		
		
		
		
		if(flagEsterno==1)
		{
			//devo svolgere le operazioni per la gestione di un segnale da tool esterno
			
			
					flagEsterno=0;
			
					char comando[2];//variabile in cui leggere il comando
					read(fdFifoTool, comando, 2);
				

				   if(atoi(comando)==50)//il comando da tool esterno 50 resetta il vettore mirroring
				   {
					   
					   free(vettoreMirroringId);
					   free(vettoreMirroringStato);
					   for(int i=0; i<numeroMirroring; i++)
					   {
					      free(vettoreMirroringMessaggio[i]);
					   }
					   
					   free(vettoreMirroringMessaggio);
					   numeroMirroring=0;
		                 
	               }
			
			
			
		}
		
		
		
		
	}
	
	//aggiorno il tool esterno
	   
	//devo passare info al tool esterno
	char messaggio[100];
	   
	sprintf(messaggio, "1X6X%dX%d|",id, getpid());
	   
	if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	{
		   printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		   getchar();
	 }
	
	//verifico se sono uscito correttamente o con un errore critico propagato da un figlio
	if(extErrore)
	{
		
		
		terminazioneFatale( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);
		
		
		
    }else
    {
		
		terminazione( vetPidBulb, numeroLampadine,  vetPidWindow, numeroFinestre,  vetPidFridge,  numeroFrigoriferi,  vetPidTenda,  numeroTende,  vetPidTermostato,  numeroTermostati,  vetPidTimer,  numeroTimer, vetPidHub,  numeroHub);

	
	
    }
	
	return 0;
}



//IMPLEMENTAZIONE FUNZIONI

//funzioni varie
//funzione che gestisce una terminazione non critica
void terminazione(pid_t* vetPidBulb, int numeroLampadine, pid_t* vetPidWindow, int numeroFinestre, pid_t* vetPidFridge, int numeroFrigoriferi, pid_t* vetPidTenda, int numeroTende, pid_t* vetPidTermostato, int numeroTermostati, pid_t* vetPidTimer, int numeroTimer, pid_t* vetPidHub, int numeroHub)
{
    //chiude le fifo
	unlink("./fifoIdEsterno");
    unlink("./fifoComandi");
    unlink("./fifoLink"); /*puo esser anche questo*/
	unlink("./fifoTool"); 
	
	//chiude i semafori
	sem_unlink("IdCm");
	
	//avvisa tutti i figli di terminare
	
	if(numeroLampadine>0)
	{
		for(int i=0; i<numeroLampadine; i++)
		{
			kill(vetPidBulb[i], SIGTERM);
		}
	}
	
	if(numeroFinestre>0)
	{
		for(int i=0; i<numeroFinestre; i++)
		{
			kill(vetPidWindow[i], SIGTERM);
		}
	}
	
	if(numeroFrigoriferi>0)
	{
		for(int i=0; i<numeroFrigoriferi; i++)
		{
			kill(vetPidFridge[i], SIGTERM);
		}
	}
	
	if(numeroTermostati>0)
	{
		for(int i=0; i<numeroTermostati; i++)
		{
			kill(vetPidTermostato[i], SIGTERM);
		}
	}
	
	if(numeroTende>0)
	{
		for(int i=0; i<numeroTende; i++)
		{
			kill(vetPidTenda[i], SIGTERM);
		}
	}
	
	if(numeroHub>0)
	{
		for(int i=0; i<numeroHub; i++)
		{
			kill(vetPidHub[i], SIGTERM);
		}
	}
	
	if(numeroTimer>0)
	{
		for(int i=0; i<numeroTimer; i++)
		{
			kill(vetPidTimer[i], SIGTERM);
		}
	}
	
	
	
	
}
//funzione che gestisce una terminazione critica
void terminazioneFatale(pid_t* vetPidBulb, int numeroLampadine, pid_t* vetPidWindow, int numeroFinestre, pid_t* vetPidFridge, int numeroFrigoriferi, pid_t* vetPidTenda, int numeroTende, pid_t* vetPidTermostato, int numeroTermostati, pid_t* vetPidTimer, int numeroTimer, pid_t* vetPidHub, int numeroHub)
{
    //chiude le fifo
	unlink("./fifoIdEsterno");
    unlink("./fifoComandi");
    unlink("./fifoLink"); /*puo esser anche questo*/
	unlink("./fifoTool"); 
	
	//chiude i semafori
	sem_unlink("IdCm");
	
	//avvisa tutti i figli di terminare
	
	if(numeroLampadine>0)
	{
		for(int i=0; i<numeroLampadine; i++)
		{
			kill(vetPidBulb[i], SIGTERM);
		}
	}
	
	if(numeroFinestre>0)
	{
		for(int i=0; i<numeroFinestre; i++)
		{
			kill(vetPidWindow[i], SIGTERM);
		}
	}
	
	if(numeroFrigoriferi>0)
	{
		for(int i=0; i<numeroFrigoriferi; i++)
		{
			kill(vetPidFridge[i], SIGTERM);
		}
	}
	
	if(numeroTermostati>0)
	{
		for(int i=0; i<numeroTermostati; i++)
		{
			kill(vetPidTermostato[i], SIGTERM);
		}
	}
	
	if(numeroTende>0)
	{
		for(int i=0; i<numeroTende; i++)
		{
			kill(vetPidTenda[i], SIGTERM);
		}
	}
	
	if(numeroHub>0)
	{
		for(int i=0; i<numeroHub; i++)
		{
			kill(vetPidHub[i], SIGTERM);
		}
	}
	
	if(numeroTimer>0)
	{
		for(int i=0; i<numeroTimer; i++)
		{
			kill(vetPidTimer[i], SIGTERM);
		}
	}
	
	
	//avvisa il padre 
	kill(getppid(), SIGTERM);
	
	
	
}


//handler

//handler per il cambio interruttore
void handlerCambiaInterruttore(int signum)
{
	
	
	if(flagEsterno==0)//se il comando proviene dalla centralina il flag è settato a 0
	{
		//nel caso leggo dalla pipe interruttore del padre il valore che si vuole avere nell interruttore
		
    	char value[]="X";
	
	
	    if(read(fdPadreInterruttore, value, strlen(value)+1)==-1)
	    {
		   printf("ERRORE DI LETTURA NELLA PIPE INTERRUTTORE DA PARTE DI HUB\n");
		   printf("ERRORE FATALE\n");
		   getchar();
		   extErrore=1;//in questo caso la gestione è un po diversa. imposto a 1 ext errore e richiamo sigterm su il processo stesso
		   raise(SIGTERM);

	    }
	
	
	    if(strcmp(value, "0")==0)
	    {
		   strcpy(interruttore, "OFF");
	    }else
	    {
		   strcpy(interruttore, "ON");
 	    }
    
    }else
    {
		//in caso contrario scambia semplicemente da un interruttore all altro
		
		if(strcmp(interruttore, "ON")==0)
	    {
		   strcpy(interruttore, "OFF");
	    }else
	    {
		   strcpy(interruttore, "ON");
	    }
	}	
   
	 
	 
}

//implementazione dell handler per indicare la lettura pipe
void handlerLetturaPipe(int signum)
{
	//imposto il flag lettura per consentire hub di leggere
	lettura=1;
	//rispetto alla lampadina aggiungiamo checvalue come con la centralina che serve a gestire cicli di feedback
	checkValue=checkValue+1;
	
}

//implementazione dell handler per la gestione del mirroring
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


//implementazione handler usato per ottenere un ack e il pid del dispositivo che ha inviato il segnale
void handlerAck(int signum, siginfo_t *siginfo, void *context)
{
	//ottiene il pid del processo che ha generato il segnale
	 pidSender=siginfo->si_pid;
	 checkValue=checkValue+1;//aumenta il checkvalue nel caso l handler venga usato in un ciclo di feedback

}

//handler per la lettura della fifo tool esterno
void handlerLetturaFifoTool(int signum)
{
	//setta il flag esterno a 1 così che si possa legger la fifo del tool esterno
	flagEsterno=1;
}

//handler per la terminazione
void handlerTerminazione(int signum, siginfo_t *siginfo, void *context)
{
    //setta il flag ext 1 così che si possa uscire dal ciclo
	ext=1;
	
	pidSender=siginfo->si_pid;
	
	if(pidSender!=getppid())//se il segnale terminazione è arrivato dal basso vuol dire che in basso è accaduto un errore fatale quindi devo terminare in modo fatale anche sopra
	{
		extErrore=1;
	}
}
