/*
 * 
 * 
 * centralina
 * 
 * 
 * 
 * NOTE
 * dealloc vettori dinamici
 * bisogna metter sigaction 
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

//MACRO
//macro  per pulire lo schermo ad ogni input comando dato dall utente
#define clear() printf("\033[H\033[J");

// DICHIARAZIONE FUNZIONI
//funzione per la separazione dei comandi inviati all utente nelle parti costituenti
void separa(char* cmd, char* comando, char* parametro1, char* parametro2, char* parametro3);
//gestore per il segnale utente 1 che permette il continuo dell esecuzione
void handlerLetturaPipe(int signum);
//gestore per il segnale di delete
void handlerAck(int signum, siginfo_t *siginfo, void *context);
//gestore del nack
void handlerNack(int signum);
//gestore mirroring
void handlerMirroring(int signum, siginfo_t *siginfo, void *context);

//VARIABILI GLOBALI
pid_t pidSender=0;
int checkValue=0;
int control=1;//serve per controllare se un operazione di inserimento è avvenuta con successo
int checkValueMirroring=0;
pid_t * pidSenderMirroring;
int checkValue1=0;



int main(int argc, char **argv)
{
	
	
	//pipe comandi dal processo iniziale
	int pipeProcessoIniziale=atoi(argv[5]);
	fcntl(pipeProcessoIniziale, F_SETFL, O_NONBLOCK);
	
	//inseriamo qui la inizializzazione di sto affare se no da errore
	pidSenderMirroring=(pid_t *)malloc(checkValueMirroring*sizeof(pid_t));

    
    //pid linker
    
    printf("il pidLinker prima aoti per centralina è %s \n", argv[3]);
    
    pid_t pidLinker=atoi(argv[3])+1;//boh
    
    printf("il pidLinker per centralina è %d \n", pidLinker );
    fflush(stdout);


	//creazione fifo comandi
	//in questa fifo vengono diffusi a tutto il sistema i comandi del dispostivo
	char* fifoComandi="./fifoComandi";
	int fdFifoComandi;//quando apri la fifo la metti qui
	fdFifoComandi=open(fifoComandi,O_RDWR);
	if(fdFifoComandi==-1)
		   {
			   printf("ERRORE APERTURA PIPE DA PARTE DI CENTRALINA \n");
				  return EXIT_FAILURE;
		   }
	
	
	//apro o creo il semaforo per la lettura dalla fifo comandi
	sem_t* semaforoFifoComandi=sem_open("IdCm", O_CREAT, 0666, 0);
	if(semaforoFifoComandi==-1)
	{
		printf("ERRORE APERTURA SEMAFORO \n");
	}
	
	int value;
	sem_getvalue(semaforoFifoComandi, &value);
	printf("valore semaforo comandi creazione %d \n", value);
	
	
	
	//apro o creo il semaforo per lettura stdin
	sem_t* semaforoEsterno=sem_open("EsSm", O_CREAT, 0666, 0);
	if(semaforoEsterno==-1)
	{
		printf("ERRORE APERTURA SEMAFORO \n");
	}
	
			  
	
	
	//maschera per i cicli di feedback
	//imposto come handler di default del segnale utente 1 l'handler per la lettura della pipe principale
	  struct sigaction act2;
	memset(&act2, '\0', sizeof(act2));
	act2.sa_sigaction=&handlerLetturaPipe;
	act2.sa_flags=SA_SIGINFO|SA_RESTART;
	sigaction(SIGUSR1, &act2, NULL);
	
	
	
	/*
	 * per ora questo è qui non escludo debba stare da altre parti
	 */ 
	
	//handler per la gestione del delete
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_sigaction=&handlerAck;
	act.sa_flags=SA_SIGINFO;
	sigaction(SIGUSR2, &act, NULL);
	
	
	//handler mirroring
	
	struct sigaction act1;
	memset(&act1, '\0', sizeof(act1));
	act1.sa_sigaction=&handlerMirroring;
	act1.sa_flags=SA_RESTART|SA_SIGINFO;
	sigaction(SIGIO, &act1, NULL);
	
	
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
    
        //con questa funzione tolgo il segnale SIGINT che mi gestisce un cambio del valore stato in un figlio
   // sigdelset(&maschera, SIGIO);
	
	printf("maschera etc creata \n");
	
	//variabili per la gestione dispositivi connessi
	//una variabile intera indica il numero di dispositivi connessi. inizialmente a 0.
	//un vettore dinamico contente i pid dei dispositivi connessi inizialmente vuoto
	
	/*
	 * temporaneamente a 1
	 * inseriti i comandi per la pipe coi figli probabilmente da metter su tutti e da aggiunger in link
	 */ 
	
	//lampadine
	pid_t *vetPidBulb;
    int *fdLampadineMirroringLettura;//questo serve a leggere l'id del mirroring e il valore interruttore
    int *fdLampadineComandi;//questo serve a scrivere l'output dei comandi
    int *fdLampadineInterruttore;//questo serve a scrivere dal padre il valore dell interruttore
    
    int numeroLampadine=1;
	vetPidBulb=(pid_t *)malloc(numeroLampadine*sizeof(pid_t));
	
	fdLampadineMirroringLettura=(int *)malloc(numeroLampadine*sizeof(int));
	fdLampadineComandi=(int *)malloc(numeroLampadine*sizeof(int));
	fdLampadineInterruttore=(int *)malloc(numeroLampadine*sizeof(int));
	
	
	printf("arrivato dopo vettori dinamici \n");
	
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
	
	int fdLampadinaInterruttore[2];
	if(pipe(fdLampadinaInterruttore)==-1)
	{
		printf("ERRORE PIPE \n");
		return EXIT_FAILURE;
	}
	
	fcntl(fdLampadinaComandi[0], F_SETFL, O_NONBLOCK);
	fcntl(fdLampadinaMirroringLettura[0], F_SETFL, O_NONBLOCK);
	fcntl(fdLampadinaInterruttore[0], F_SETFL, O_NONBLOCK);
	
	printf("arrivato dopo pipe \n");
	
	char pipeMirroringLettura[100];
	char pipeComandi[100];
	char pipeInterruttore[100];

	
	sprintf(pipeMirroringLettura, "%d", fdLampadinaMirroringLettura[1]); 
	sprintf(pipeComandi, "%d", fdLampadinaComandi[1]); 
	sprintf(pipeInterruttore, "%d", fdLampadinaInterruttore[0]); 
	
	 char *args[]={"./lampadina", "4", "test1", argv[3], pipeMirroringLettura, pipeComandi, pipeInterruttore, NULL};
	
	
	printf("arrivato dopo pipe \n");
	
	
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
	
	
	
	close(fdLampadinaMirroringLettura[1]);
	
	close(fdLampadinaComandi[1]);
	
	close(fdLampadinaInterruttore[0]);
	
	
	fdLampadineMirroringLettura[0]=fdLampadinaMirroringLettura[0];
	
	fdLampadineComandi[0]=fdLampadinaComandi[0];
	
	fdLampadineInterruttore[0]=fdLampadinaInterruttore[1];
	
	
	printf("LAMPADINA CREATA \n");
	
	//frigoriferi
	int numeroFrigoriferi=0;
	pid_t *vetPidFridge=(pid_t *)malloc(numeroFrigoriferi*sizeof(pid_t));
    int* fdFrigoriferiComandi=(int *)malloc(numeroFrigoriferi*sizeof(int));
    int *fdFrigoriferiInterruttore=(int *)malloc(numeroFrigoriferi*sizeof(int));
        int *fdFrigoriferiMirroringLettura=(int *)malloc(numeroFrigoriferi*sizeof(int));

	
	//finestre
	int numeroFinestre=0;
	pid_t *vetPidWindow=(pid_t *)malloc(numeroFinestre*sizeof(pid_t));
	int* fdFinestreComandi=(int *)malloc(numeroFinestre*sizeof(int));
    int *fdFinestreInterruttore=(int *)malloc(numeroFinestre*sizeof(int));
    int *fdFinestreMirroringLettura=(int *)malloc(numeroFinestre*sizeof(int));
	
	//hub
	int numeroHub=1;
	pid_t *vetPidHub=(pid_t *)malloc(numeroHub*sizeof(pid_t));
	int *fdHubMirroringLettura=(int *)malloc(numeroHub*sizeof(int));
	int* fdHubComandi=(int *)malloc(numeroHub*sizeof(int));
    int *fdHubInterruttore=(int *)malloc(numeroHub*sizeof(int));

	
	printf("arrivato dopo vettori dinamici \n");
	
	
	int fdHub1Interruttore[2];
	if(pipe(fdHub1Interruttore)==-1)
	{
		printf("ERRORE PIPE \n");
		return EXIT_FAILURE;
	}
	
	int fdHub1MirroringLettura[2];
	if(pipe(fdHub1MirroringLettura)==-1)
	{
		printf("ERRORE PIPE \n");
		return EXIT_FAILURE;
	}
	
	int fdHub1Comandi[2];
	if(pipe(fdHub1Comandi)==-1)
	{
		printf("ERRORE PIPE \n");
		return EXIT_FAILURE;
	}
	
	fcntl(fdHub1Comandi[0], F_SETFL, O_NONBLOCK);
	fcntl(fdHub1MirroringLettura[0], F_SETFL, O_NONBLOCK);
	
	fcntl(fdHub1Interruttore[0], F_SETFL, O_NONBLOCK);
	
	printf("arrivato dopo pipe \n");
	
	
	
	sprintf(pipeMirroringLettura, "%d", fdHub1MirroringLettura[1]); 
	
	sprintf(pipeInterruttore, "%d", fdHub1Interruttore[0]);
	printf("passo a hub pipe INTERRUTTORE %d \n", fdHub1Interruttore[0]);

	
	sprintf(pipeComandi, "%d", fdHub1Comandi[1]);
	
	 
	 char *argsHub[]={"./hub", "5", "test", argv[3], pipeMirroringLettura, pipeComandi, pipeInterruttore, NULL};
	
	
	printf("arrivato dopo pipe hub \n");
	
	
	fflush(stdout);
	vetPidHub[0]=fork();
	if(vetPidHub[0]==-1)
	{
		printf("ERROE FORK HUb \n");
		return(EXIT_FAILURE);
	}
	
	printf("arrivato prima exec \n");
	
	if(vetPidHub[0]==0)
	{
		
		execvp(argsHub[0], argsHub);
	}
	
	printf("arrivato dopo fork \n");
	
	
	
	close(fdHub1MirroringLettura[1]);
	
		close(fdHub1Comandi[1]);
		
close(fdHub1Interruttore[0]);
	
	fdHubMirroringLettura[0]=fdHub1MirroringLettura[0];
	fdHubComandi[0]=fdHub1Comandi[0];
	fdHubInterruttore[0]=fdHub1Interruttore[1];
	
	printf("io ho pipe INTERRUTTORE %d \n", fdHubInterruttore[0]);

	
	printf("hub creata \n");
	
	
	
	//timer
	int numeroTimer=0;
	pid_t *vetPidTimer=(pid_t *)malloc(numeroTimer*sizeof(pid_t));
    int* fdTimerComandi=(int *)malloc(numeroTimer*sizeof(int));
    int *fdTimerInterruttore=(int *)malloc(numeroTimer*sizeof(int));
	    int *fdTimerMirroringLettura=(int *)malloc(numeroTimer*sizeof(int));


	//termostati
	int numeroTermostati=0;
	pid_t *vetPidTermostato=(pid_t *)malloc(numeroTermostati*sizeof(pid_t));
    int* fdTermostatiComandi=(int *)malloc(numeroTermostati*sizeof(int));
    int *fdTermostatiInterruttore=(int *)malloc(numeroTermostati*sizeof(int));
	    int *fdTermostatiMirroringLettura=(int *)malloc(numeroTermostati*sizeof(int));

	
	//tende
	int numeroTende=0;
	pid_t *vetPidTenda=(pid_t *)malloc(numeroTende*sizeof(pid_t));
    int* fdTendeComandi=(int *)malloc(numeroTende*sizeof(int));
    int *fdTendeInterruttore=(int *)malloc(numeroTende*sizeof(int));
        int *fdTendeMirroringLettura=(int *)malloc(numeroTende*sizeof(int));

	
	printf("CENTRALINA ARRIVATA DOPO CREAZIONE DISPOSITIVI \n");
	
	//CENTRALINA  VERA E PROPRIA


    //id
    int id=atoi(argv[1]);

    //interruttore del sistema
    //se disattivato non si puo inserire alcun comando
    char interruttoreSistema[]="OFF";
    strcpy(interruttoreSistema, argv[2]);
    
    //interruttore mirroring
    /*
     * temporaneamente a on per verificare funzionamento
     */ 
     char interruttoreMirroring[]="ON";
    
    
	
	//vettore che contiene il comando inserito dall utente nella sua interezza
    char* cmd;
    cmd=malloc(100*sizeof(char));
    size_t size=100;
    
    //i suoi componenti vengono separati dalla funzione separa nei seguenti sottovettori
    //contiene il comando vero e proprio
    char comando[100];
    //contiene il primo parametro del comando
    char parametro1[100];
    //contiene il secondo parametro del comando
    char parametro2[100];
    //contiene il terzo parametro del comando
    char parametro3[100];
	
	
	//per l'inserimento in pipe
	
	/*
	 * ho un dubbio sulla label ma fa lo stesso
	 * 
	 */ 
	char comand;
	char id1[3];
	char device;
	char id2[3];
	char pos;
	//massimo fisso di 9 caratteri escluso il terminatore, questo va scritto sulla pipe
	char communication[]="XXXXXXXXX";
	
	

	//ciclo per la continua ripetizione dell inserimento comandi
    //variabile binaria indica se uscire dal programma
       int ext=0;


    /*
     * da notare che mi da warning comparazione tra pointer etc
     */ 
	do
	{
	
	   //a ogni ciclo puliamo la shell
	   //clear();
	  
       //STATO E REGISTRO
       //valore stato iniziale
        int stato=0;
       //valore di registro
	   int num=numeroLampadine+numeroFinestre+numeroFrigoriferi+numeroHub+numeroTimer; 
       
       //ripristino checkValue
       checkValue=0;
       		
	   //TITOLETTO
	   printf("CENTRALINA \n \n");

       	
	   //interruttore di sistema
	   if (strcmp(interruttoreSistema, "ON")==0)
	   {
	      stato=1;
	      printf("STATO: ACCESO \n");
       }
       else 
       {
		   stato=0;
		   printf("STATO: SPENTO \n");
	   }

   	   //per la gestioone mirroring
   	      int numeroMirroring=0;//numero dei dispositivi con valori anomali
   	   int *vettoreMirroringId=(int *)malloc(numeroMirroring*sizeof(int));//vettore che contiene gli id dei dispositivi con valori anomali
   	int *vettoreMirroringStato=(int *)malloc(numeroMirroring*sizeof(int));//vettore che contiene lo stato dei dispositivo con valori anomali
   	  
   	  char **vettoreMirroringMessaggio;
   	  vettoreMirroringMessaggio=malloc(numeroMirroring*sizeof(char*));
   	  
   	   
	   if (strcmp(interruttoreMirroring, "ON")==0)
	   {
	      printf("MIRRORING: ATTIVO \n");
	      
	      /*
	       * bisogna sistemarlo per tutti i tipi di dispositivi
	       */ 
	      
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
	       
           if(numeroMirroring>0)//stampo solo se il numero di dispositivi con valore anomalo è maggiore di zero
           { 
              printf("\ni seguenti dispositivi direttamente connessi presentano un valore anomalo:\n");
           
              for(int i=0; i<numeroMirroring; i++)
              {
			      printf("id: %d stato: %d  %s\n", vettoreMirroringId[i], vettoreMirroringStato[i], vettoreMirroringMessaggio[i]);
		      }  
           }
       }
       else 
       {
		   
		   printf("MIRRORING: DISATTIVO \n");
	   }
   	   
   	   
   	   
	   printf("NUMERO DISPOSITIVI CONNESSI: ");
	   printf("%d \n", num);
	
   	   printf("\n");
	
	   //LEGGENDA DEI COMANDI DISPONIBILI PER L UTENTE
	   printf("ISTRUZIONI \n");
	   printf("inserisci uno dei seguenti comandi per interagire con la centralina \n");
	   printf("- list \n");
	   printf("- add <device> \n");
	   printf("- del <id> \n");
	   printf("- link <id> to <id> \n");
	   printf("- switch <id> <label> <pos> \n");
	   printf("- info <id> \n");
	   printf("\n");
       printf("per ulteriori informazioni sui comandi inserisci: help <nome comando> \n");
       printf("per uscire inserisci: exit \n");
      
      
      if(stato==0)
      { 
       printf("\n il sistema è spento. è possibile accenderlo con il comando <switch 0 accensione/spegnimento ON> \n");
       printf("\n quando il sistema è spento i comandi switch e link non sono abilitati\n");
      }
    
       
       /*
        * temporaneo
        * 
        */ 
        
        int value1;
        
        sem_getvalue(semaforoFifoComandi, &value1);
       
       if(value1==1)
       {
		   sem_wait(semaforoFifoComandi);
	   }
       
     printf("inserisci un qualsiasi carattere per continuare e inserisci un comando \n");
     getchar();
      
      //prima di leggere controlla se nessuno occupa lo stdin
      sem_wait(semaforoEsterno);
        
        
        scanf("%[^\n]s",cmd);
        
        
        printf("COMANDO %s \n", cmd);
       
      
       
       //separo il comando nei suoi costituenti con la funzione separa
       separa(cmd, comando, parametro1, parametro2, parametro3);
     
       printf("PARAMETRO1:%s", parametro1);
       printf("PARAMETRO2:%s", parametro2);     
       printf("PARAMETRO3:%s", parametro3);          
     
     
       printf("\n");
  
      
        
  
       /*
	    * l'implementazione reale dei comandi va fatta a tempo debito
	    */
    
       if((strcmp(comando, "list")==0))
       {   
		
		
		   comand='4';
		   communication[0]=comand;
	
		   
		   if(write(fdFifoComandi, communication, strlen(communication))==-1)
		   { 
			 printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA \n");
		     return EXIT_FAILURE;
		   }
			  
			  
			  
		   //conesnto alla centralina di legger la fifo
		   sem_post(semaforoFifoComandi);
			  
	       //avviso la centralina che puo leggere
	       kill(getppid(), SIGUSR1);
	       
	       printf("centrlina ha notificato \n");
			  
		   while(checkValue<1)
		   {
		   }
			  
		   char informazioni[2000];
					  
		   if(read(pipeProcessoIniziale, informazioni, 2000)==-1)
		   {
		       printf("nessun dispositivo nel sistema, potrebbero esser gia stati connessi alla centralina\n");
   		   }else
	 	   {
			   printf("\ndispositivi non connessi presenti nel sistema\n");
			   printf("%s \n", informazioni);
			}
					  
					    
			  
			  
			   //ora toocca alla centralina stampare i suoi dispositivi connessi
			   
			   if(num=0)
			   {
				   printf("nessun dispositivo connesso alla centralina \n");
			   }else
			   {
				   
			       	  					    
			  
			  
			   
				   //cicli operativi
				   
				  if(numeroLampadine>0)//lampadine
				  {  
					 printf("lampadine connesse alla centralina \n");
			   
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
					      
					     printf("%s \n", informazioni);
						  
					 }
			     
			     
			     }
				  
				   //frigoriferi
				   
				 if(numeroFrigoriferi>0)//lampadine
				  {  
					  printf("frigoriferi direttamente connessi alla centralina \n");
			   
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
					      
					     printf("%s \n", informazioni);
						  
					 }
			     
			     
			     }  
				   
				   
				   //finestre
				  if(numeroFinestre>0)//lampadine
				  {  
					  
					  printf("finestre direttamente connessi alla centralina \n");
			   
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
					      
					     printf("%s \n", informazioni);
						  
					 }
			     
			     
			     } 
				   //tende
				  if(numeroTende>0)//lampadine
				  {  
					  printf("tende direttamente connessi alla centralina \n");
			   
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
					      
					     printf("%s \n", informazioni);
						  
					 }
			     
			     
			     } 
				   //termostati
				  if(numeroTermostati>0)//lampadine
				  {  
					  
					  printf("termostati direttamente connessi alla centralina \n");
			   
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
					      
					     printf("%s \n", informazioni);
						  
					 }
			     
			     
			     } 
				   //timer
				  if(numeroTimer>0)//lampadine
				  {  
					  
					  printf("timer direttamente connessi alla centralina \n");
			   
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
					      
					     printf("%s \n", informazioni);
						  
					 }
			     
			     
			     } 
				   //hub  
				  if(numeroHub>0)//lampadine
				  {  
					  
					  printf("hub direttamente connessi alla centralina \n");
			   
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
					      
					     printf("%s \n", informazioni);
						  
					 }
			     
			     
			     }
				  
				   
				  
				  
				  
				  
				   
			   }
			   
			  
			
			 
			
			   //rièristina checkValue
			   checkValue=0;  
			  
			  //ripristina communication una volta terminato
			  strcpy(communication, "XXXXXXXXX");
		
             
       
       }
       else if((strcmp(comando, "add")==0))//verifico se il comando ricevuto è add
       {
		  
		 
		   
		     //di seguito impostiamo i codici di comunicazione con il processo iniziale
		  
		     comand='1';
		  
		    //in base alla tipologia di dispositivo invio un codice differente per il dispositivo
		  
		     if(strcmp(parametro1, "bulb")==0)
		     {
			     device='0';
			     
		     }
		     else if(strcmp(parametro1, "window")==0)
		     {
			    device='1';
		     }
		     else if(strcmp(parametro1, "fridge")==0)
		     {
			    device='2';
		     }
		     else if (strcmp(parametro1, "hub")==0)
		     {
			    device='3';
		     }
		     else if(strcmp(parametro1, "timer")==0)
		     {
			    device='4';
		     }
		     else if(strcmp(parametro1, "thermostat")==0)
		     {
				device='5';
			 }
			 else if(strcmp(parametro1, "tent")==0)
			 {
				device='6';
			 }
		     else //se non ho riconosciuto alcun dispositivo
		     {
			    device='7';
		     }
	     	 
	         if(device!='7')//continuo solo se ho riconosciuto dispositivo
	         {
				 //imposto i valori vettore di comunicazione
			     communication[0]=comand;
			     communication[4]=device;
			  
			  

			  
			     //manda in pipe i dettagli del comando
			   
			     if(write(fdFifoComandi, communication, strlen(communication))==-1)
			     {
				    printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				    return EXIT_FAILURE;
			     }
			  
			     printf("centralina scrive su pipe ");
			  
			     sem_post(semaforoFifoComandi);//posto il semaforo cosi si puo legger da pipe
			  
			     //qui avvisa giustamente solo il padre
			     kill(getppid(), SIGUSR1);
			  
			  
			   //imposto l'handler di nack	
             	signal(SIGUSR2, handlerNack);
			      //imposto gestore di nack per segnale utente due
			     sigsuspend(&maschera);//attendo un segnale di ack
			  
			  if(control==0)//se control è 0 il dispositivo non è stato aggiunto
			  {
				  printf("\nnon è stato possibile aggiungere il dispositivo \n");
				  printf("premere un pulsante per continuare \n");
				  
			  }else//in caso contrario si e lo comunico
			  {
				  if(strcmp(parametro1, "bulb")==0)
				  {
					  printf("\nLampadina inserita con successo\n");
					  printf("premere un pulsante per continuare \n");
					  
				  }else if(strcmp(parametro1, "fridge")==0)
				  {
					   printf("\nFrigorifero inserito con successo\n");
					  printf("premere un pulsante per continuare \n");
					  
				  }else if(strcmp(parametro1, "window")==0)
				  {
					   printf("\nFinestra inserita con successo\n");
					   printf("premere un pulsante per continuare \n");
				  
				  }else if(strcmp(parametro1, "hub")==0)
				  {
					   printf("\nHub inserito con successo\n");
					   printf("premere un pulsante per continuare \n");
				  
				  }else if(strcmp(parametro1, "timer")==0)
				  {
					  printf("\nTimer inserito con successo\n");
					  printf("premere un pulsante per continuare \n");
				  
				  }else if(strcmp(parametro1, "thermostat")==0)
				  { 
					  printf("\nTermostato inserito con successo\n");
					  printf("premere un pulsante per continuare \n");
				  
				  }else if(strcmp(parametro1, "tent")==0)
				  {
					  printf("\nTenda inserita con successo\n");
					  printf("premere un pulsante per continuare \n");
				  }
				  
			  }
			  
		     } 	
		     else//in caso il dispositivo non sia riconosciuto
		     {
			     printf("dispositivo da aggiungere non riconosciuto");
			     printf("premere un pulsante per continuare \n");
		     }
		  
	    
	  
	     
		
       }
       else if(strcmp(comando, "del")==0)
       {
		   
		   
		   comand='2';
		   
		   if(strlen(parametro1)<4)
		   {
			 strcpy(id1, parametro1);
			 if(id1[0]>'9'||id1[0]<'0')
			 {
				 printf("errore con id");
			 }
			 if(id1[1]>'9'||id1[1]<'0')
			 {
				 id1[1]='X';
			 }
			 if(id1[2]>'9'||id1[2]<'0')
			 {
				 id1[2]='X';
			 }
			 communication[0]=comand;
			 communication[1]=id1[0];
			 communication[2]=id1[1];
			 communication[3]=id1[2];
			 
			 printf("centralina scrivo sulla pipe %s \n", communication);
			 
			 if(write(fdFifoComandi, communication, strlen(communication))==-1)
			  {
				  printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				  return EXIT_FAILURE;
			  }
			  
			  sem_getvalue(semaforoFifoComandi, &value);
			  printf("centralina valore prima del post %d \n", value);
			  
			   //conesnto alla centralina di legger la fifo
			  sem_post(semaforoFifoComandi);
			  
			  sem_getvalue(semaforoFifoComandi, &value);
			  printf("centralina valore  dopo post %d \n", value);
			  
			  
			  printf("centralina invio il segnale al padre \n");
			  
			  checkValue=0;
			  
			  //avviso la centralina che puo leggere
			  kill(getppid(), SIGUSR1);
			  
			  printf("centralina in attesa \n");
			  
			  //attendo il feedback di ritorno del processo iniziale
			  while(checkValue<1)
			  {
			  }
			  
			  printf("centralina ottengo feedback del padre e leggo  \n");
			  
			  if(read(fdFifoComandi, communication, strlen(communication))==-1)
		      {
			     printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			     getchar();
			     return EXIT_FAILURE;
		      }
		      
		      printf("centralina legge %s \n", communication);
			  
			  if(communication[0]!='0')//se è stato trovato prima non ha senso continuare
			  {
				
				if(numeroLampadine>0)
				{
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
			       
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
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
				          if(write(fdFifoComandi, communication, strlen(communication))==-1)
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
		                                           
											   }
									           
									           
									           printf("COMMUNICATION ALLA FINE CENTRALINA %s \n", communication);
									           
		                         
		                         if(communication[0]!='0')
		                         {
									 printf("\nid non trovato. Probabilmente il dispositivo non esiste nel sistema");
								 }else{
									 
									 //TERMOSTATI
									 
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
									//TENDE	 
									
									
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
									 
						}else{			 
					   /*
					    * dovrebbe esser sicuramente diverso da 0
					    * 
					    */ 
					   //qui ho trovato una lampadina
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
					   /*
					    * dovrebbe esser sicuramente diverso da 0
					    * 
					    */ 
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
					   /*
					    * dovrebbe esser sicuramente diverso da 0
					    * 
					    */ 
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
					   /*
					    * dovrebbe esser sicuramente diverso da 0
					    * 
					    */ 
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
					   /*
					    * dovrebbe esser sicuramente diverso da 0
					    * 
					    */ 
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
				  
			  }
			  
		  
			  
			  
			 
			   
		   }
		   else
		   {
			   printf("id non accettabile");
		   }
		  
	          sem_wait(semaforoFifoComandi);
		   
          	 //ripristina communication
			  strcpy(communication, "XXXXXXXXX");	   
		   
		   
       }
       else if(strcmp(comando, "link")==0)
       {
		   if (stato==1)
		   {
		   
		   printf("centralina nel comando link \n");
		   printf("comando %s parametro 1 %s parametro 2 %s \n", comando, parametro1, parametro2);
		
		   
		  comand='5';
		  communication[0]=comand;
		  
		  //questo contiene i risultati del comando
		  char communicationVerifica[]="XXX";
		  
		  
		  if((strlen(parametro1)<4)&&(strcmp(parametro1, "0")!=0)&&(strcmp(parametro1, parametro2)!=0))
		  {
		     strcpy(id1, parametro1);
			 if(id1[0]>'9'||id1[0]<'0')
			 {
				 printf("errore con id");
			 }
			 if(id1[1]>'9'||id1[1]<'0')
			 {
				 id1[1]='X';
			 }
			 if(id1[2]>'9'||id1[2]<'0')
			 {
				 id1[2]='X';
			 }
			
			 communication[1]=id1[0];
			 communication[2]=id1[1];
			 communication[3]=id1[2];
			 
			 printf("centralina id1  %s \n", id1);
			 
			 if(strlen(parametro2)<4)
			 {
	 			 strcpy(id2, parametro2);
	 			 
	 			 printf("id2 dovrebbe esser inizio %s", id2);
	 			 
	     		 if(id2[0]>'9'||id2[0]<'0')
		     	 {
					 /*
					  * questo e quello sopra van gestiti
					  * 
					  */ 
				    printf("errore con id");
			     }
			     if(id2[1]>'9'||id2[1]<'0')
			     {
				    id2[1]='X';
			     }
			     if(id2[2]>'9'||id2[2]<'0')
			     {
				    id2[2]='X';
			     }
			
			     communication[5]=id2[0];
			     communication[6]=id2[1];
			     communication[7]=id2[2];
			     
			     printf("centralina id2 %s \n", id2);
			     
			 
			     
			       if(strcmp(parametro2, "0")==0)
			       {
					   strcpy(communicationVerifica, "X5X");
					   printf("il secondo id è la centralina");
					   
					   /*
					    * si deve gestire il resto dopo ma intanto abbiamo un input
					    * 
					    */ 
				   }
			       
			       printf("centralina scrivo sulla fifo %s %s", communication, communicationVerifica);
			       
			       if(write(fdFifoComandi, communication, strlen(communication))==-1)
			       {
				      printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA \n");
				      return EXIT_FAILURE;
			       }
			       
			        if(write(fdFifoComandi, communicationVerifica, strlen(communicationVerifica))==-1)
			       {
				      printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA \n");
				      return EXIT_FAILURE;
			       }
			       
			       printf("centralina avvisa iniziale \n");
			           //avvisiamo come al solito il processo iniziale
			     kill(getppid(), SIGUSR1);
			     
			  
			     int value;
			     sem_getvalue(semaforoFifoComandi, &value);
			     printf("valore centralina prima post %d \n", value );
			     
			      printf("incremento il semaforo \n");
			     //conesnto alla centralina di legger la fifo
			      sem_post(semaforoFifoComandi);
			      
			      sem_getvalue(semaforoFifoComandi, &value);
			      printf("valore centralina dopo post %d \n", value );
			     
			     //a differenza degli altri comandi qui facciamo una verifica solo alla fineù
			     /*
			      * in realtà forse occorre alla fine
			      * 
			      */ 
			      printf("centralina in attesa ritorno padre \n");
			      sigsuspend(&maschera);
			      
			      printf("centralina ottiene ritorno dal padre \n");
			     
			     //lampadine direttamente connesse 
				 for(int i=0; i<(numeroLampadine); i++)
			     {
				  	 kill(vetPidBulb[i], SIGUSR1);
			     }
			     
			       //finestre direttamente connesse 
				 for(int i=0; i<(numeroFinestre); i++)
			     {
				  	 kill(vetPidWindow[i], SIGUSR1);
			     }
			     
			       //frigoriferi direttamente connesse 
				 for(int i=0; i<(numeroFrigoriferi); i++)
			     {
				  	 kill(vetPidFridge[i], SIGUSR1);
			     }
			     
			       //timer direttamente connesse 
				 for(int i=0; i<(numeroTimer); i++)
			     {
				  	 kill(vetPidTimer[i], SIGUSR1);
			     }
			     
			       //hub direttamente connesse 
				 for(int i=0; i<(numeroHub); i++)
			     {
				  	 kill(vetPidHub[i], SIGUSR1);
			     }
			     
			     printf("centralina entra nel suspend \n");
			     //controllo che tutti i dispositivi connessi siano ritornati
			     fflush(stdout);
			     
			     checkValue=0;
			     
			     while(checkValue<num)
			     {
					 sigsuspend(&maschera);
				 }
				 
				 
				 fflush(stdout);
				 
				 //leggp il valore pipe con il comando sapendo che avran terminato tutti
				 
				 if(read(fdFifoComandi, communication, strlen(communication))==-1)
		         {
			             printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			             getchar();
			            return EXIT_FAILURE;
		         }
				 
				 //leggp il valore pipe con il check
				 
				 if(read(fdFifoComandi, communicationVerifica, strlen(communicationVerifica))==-1)
		         {
			             printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			             getchar();
			            return EXIT_FAILURE;
		         }
				 
				 if((strcmp(communicationVerifica, "X51")==0)||(strcmp(communicationVerifica, "X5X")==0))
				 {
					 printf("primo id specificato inesistente \n");
					 fflush(stdout);
				 }
				 
				 if((strcmp(communicationVerifica, "551")==0)||(strcmp(communicationVerifica, "X51")==0))
				 {
					 printf(" il secondo id specificato non è un dispositivo di ccontrollo \n");
				     fflush(stdout);
				 }
				 
				 
				 
				 
				 
				 if((strcmp(communicationVerifica, "55X")==0))
				 {
					 
					 printf(" sintassi comando corretta \n");
					 fflush(stdout);
					 
					 if(strcmp(parametro2, "0")==0)
					 {
						 
						 printf("il secondo id è centralina \n");
						 fflush(stdout);
						 
						 //qui bisogna avviare il processo di copia
						 
						 //verifichiamo se il dispositivo non è gia connesso
						 if(pidSender!=0)
						 {
							 printf("errore identità \n");
							 fflush(stdout);
							 
							 char* fifoLink="./fifoLink";
	            int fdFifoLink=open(fifoLink,O_RDWR);
	            sem_t* semaforoLink;
	            semaforoLink=sem_open("LkSm", O_CREAT, 0666, 0);	
	            
	            sem_wait(semaforoLink);
	            if(write(fdFifoLink, "0", strlen("0"))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            
	            kill(pidLinker, SIGUSR2);//invio il segnale al linker	
				
							
							 printf("il dispositivo da connettere specificato è gia connesso alla centralina \n");
							 fflush(stdout);
						 }else
						 {
							 printf("controllo identità superato in centralina \n");
							 fflush(stdout);
							 
							 //processo di copia
							 
							 char* fifoLink="./fifoLink";
	            int fdFifoLink=open(fifoLink,O_RDWR);
	            if(fdFifoLink==-1)
	            {
					printf("ERRORE APERTURA FIFO LINK DA PARTE CENTRALINA \n");
					getchar();
					return EXIT_FAILURE;
				}
	            
	            
	            sem_t* semaforoLink;
	            semaforoLink=sem_open("LkSm", O_CREAT, 0666, 0);	
	            if(semaforoLink==-1)
	            {
					printf("ERRORE APERTURA SEMAFORO LINK DA PARTE CENTRALINA \n");
					getchar();
					return EXIT_FAILURE;
				}
	            
	            
	            printf("centralina aspetta il semaforo \n");
	            
	            sem_getvalue(semaforoLink, &value);
	            printf("il valore del semaforo link in centralina è %d \n ", value);
	            
	            sem_wait(semaforoLink);
	            
	            if(write(fdFifoLink, "2", strlen("2"))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE LINK DA PARTE DI CENTRALINA");
				  return EXIT_FAILURE;
	            }
	            
	            
	            printf("notifico %d \n", pidLinker);
	            kill(pidLinker, SIGUSR2);//invio il segnale al linker	
				
				printf("centralina in suspend \n");
				
				sigsuspend(&maschera);	//aspetta feedback
				
				printf("centralina esce suspend ma non semaforo \n");
				
				sem_wait(semaforoLink);	
				
				printf("centralina esce da suspend e semafor \n");
				
				char tipo[]="0";
				
				if(read(fdFifoLink, tipo, strlen(tipo))==-1)
		{
			printf("ERRORE LETTURA PIPE LINK DA PARTE DI CENTRALINA \n");
			getchar();
			return EXIT_FAILURE;
		}
		 
		 printf("la centralina ha letto il tipo %s \n", tipo);
		 
		 //alloco lo spazio sulla base di cio che ho letto
		fflush(stdout);
		
				if(strcmp(tipo, "0")==0)
				{
					printf("centralina sta creadndo una lampadina \n");
					fflush(stdout);
					
					numeroLampadine=numeroLampadine+1;
					
				pid_t *vetPidBulb1=(pid_t *)malloc(numeroLampadine*sizeof(pid_t));
				for(int i=0; i<(numeroLampadine-1); i++)
				{
					vetPidBulb1[i]=vetPidBulb[i];
				}
				
				vetPidBulb=vetPidBulb1;
				
			
				
				vetPidBulb[numeroLampadine-1]=fork();
				
				if(vetPidBulb[numeroLampadine-1]==-1)
				{
					printf("ERRORE CREAZIONE LAMPADINA \n");
					getchar();
					return EXIT_FAILURE;
				}
	            
	           char attivazione[]="linking"; 
	           
	           char pidLinkerString[100]; 
	           
	           strcpy(pidLinkerString, argv[3]);
	            
	            //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	            char *args[]={"./lampadina",  attivazione, attivazione, pidLinkerString, NULL};
	   
                if (vetPidBulb[numeroLampadine-1]==0)
	            {
	               execvp(args[0], args);
	            }
	            
	            printf("centralina in attesa del figlio \n");
	            fflush(stdout);
	            
	            //in attesa del figlio
				 sigsuspend(&maschera);
				
				printf("ritornato avviso quindi avviso il linker \n");
				fflush(stdout); 
				 
				 //avviso receiver
				 kill(pidLinker, SIGUSR1);
					
				}
				
				/*
				 * manca il resto
				 */ 
				 
				 
				 
						 }
						 
					 }else if((pidSender!=0))//in alternativa si vede se eliminare
					 {
						 /*
						  * probabilmente qui deve aspettare
						  */ 
						  
						 
						 //variabile che contiene l'indice dove si trova il pid
						   int indice;
						   
						  //tipo di dispositivo dell indice 
						   int dispositivo=0;
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroLampadine; i++)
						   {
							   if (vetPidBulb[i]==pidSender)
							   {
								   indice=i;
								   dispositivo=1;
							   }
						   }
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroFinestre; i++)
						   {
							   if (vetPidWindow[i]==pidSender)
							   {
								   indice=i;
								   dispositivo=2;
							   }
						   }
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroFrigoriferi; i++)
						   {
							   if (vetPidFridge[i]==pidSender)
							   {
								   indice=i;
								   dispositivo=3;
							   }
						   }
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroTimer; i++)
						   {
							   if (vetPidTimer[i]==pidSender)
							   {
								   indice=i;
								   dispositivo=4;
							   }
						   }
						   
						   //cerco l'indice incrimnato nel vettore lampadine
						   for(int i=0; i<numeroHub; i++)
						   {
							   if (vetPidHub[i]==pidSender)
							   {
								   indice=i;
								   dispositivo=5;
							   }
						   }
						 
						  if(dispositivo=1)
						  {
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
							  
						   }else if(dispositivo=2)
						   {
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
						   
						   }else if(dispositivo=3)
						   {
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
						   
						   
						   }else if(dispositivo=4)
						   {
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
						   
						   }else if(dispositivo=5)
						   {
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
						   
						   }
						 
						   
						   
						  
						   
						   
						 
						 
						 
					 }
					 
					 pidSender=0;
					 
					 
				 }
				 
			     
			 }else
			 {
				printf("id2 troppo lungo");
			 }
			 
			 
			}else
			{
				if(strcmp(parametro1, "0")==0)
				{
					printf("la centralina non può esser linkata a nessun dispositivo");
				}
				else if (strcmp(parametro1, parametro2)==0)
				{
					printf("hai inserito due id uguali");
				}
				else 
				{
					printf("id1 troppo lungo");
				}
			}		 
			
		}else
		{
			printf("centralina spenta, impossibile inserire comando link \n");
		}  
			  
			  
			  //ripristina communication
			  strcpy(communication, "XXXXXXXXX");
			  
			  
		  
       }
       else if(strcmp(comando, "switch")==0)
       {
		   printf("sono nel comando switch \n");
		   
		  if(stato==1)
		  {
		     comand='6';
		  
		     printf("il valore in parametro2 è %s \n", parametro2);
		     printf("il valore in parametro3 è %s \n", parametro3);
		  
		     if(strlen(parametro1)<4)
		     {
		        strcpy(id1, parametro1);
			    if(id1[0]>'9'||id1[0]<'0')
			    {
				   printf("errore con id");
			    }
			    if(id1[1]>'9'||id1[1]<'0')
			    {
				   id1[1]='X';
			    }
			    if(id1[2]>'9'||id1[2]<'0')
			    {
			       id1[2]='X';
			    }  
		        communication[1]=id1[0];
			    communication[2]=id1[1];
			    communication[3]=id1[2];
			 
			    if((strcmp(parametro2, "accensione/spegnimento")==0)||((strcmp(parametro2, "apertura/chiusura"))==0)||((strcmp(parametro2, "mirroring"))==0))
			    {
			       if(strcmp(parametro2, "accensione/spegnimento")==0)
			       {
				      communication[4]='0';
			       }
			       else
			       {
				   communication[4]='1';
			       }
			       
			       printf("accensione spegnimento passato \n");
			   
			       if((strcmp(parametro3, "ON")==0)||((strcmp(parametro3, "OFF"))==0))
			       {
				   
				     printf("on of passato \n");
				   
				     if(strcmp(parametro1, "0")==0)//verifichiamo se il bersaglio di switch sia centralina
				     {
						 printf("sono in check centralina \n");
						 
						 if(strcmp(interruttoreMirroring, "ON")==0)
						 {
							 printf("interruttore mirroring acceso \n");
							 
							 if(strcmp(parametro2, "mirroring")==0)
							 {
								 printf("il secondo parametro è mirroring \n");
								 
								 if(strcmp(parametro3, "ON")==0)
								 {
									 printf("il terzo parametro è on \n");
									 
									 strcpy(interruttoreMirroring, "ON");
									 
									 printf("ho svolto la op \n");
								 }
								 else
								 {
									 strcpy(interruttoreMirroring, "OFF");
								 }
							 }else if(strcmp(parametro2, "accensione/spegnimento")==0)
							 {
								 if(((strcmp(parametro3, "ON")==0)&&(strcmp(interruttoreSistema, "OFF")==0))||((strcmp(parametro3, "OFF")==0)&&(strcmp(interruttoreSistema, "ON")==0)))
								 {
									if((strcmp(parametro3, "ON")==0)&&(strcmp(interruttoreSistema, "OFF")==0))
									{
										strcpy(interruttoreSistema, "ON");
										stato=1;
									}else
									{
										strcpy(interruttoreSistema, "OFF");
										stato=0;
										
									} 
									
									
									char statoStringa[]="0";
									
									sprintf(statoStringa, "%d", stato);
									
									//qui ci sono le operazioni di mirroring
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
									
									
								 }
							}	 
							 
						 }else
						 {
							 if(strcmp(parametro2, "mirroring")==0)
							 {
								 if(strcmp(parametro3, "ON")==0)
								 {
									 strcpy(interruttoreMirroring, "ON");
								 }
								 else
								 {
									 strcpy(interruttoreMirroring, "OFF");
								 }
							 }else if(strcmp(parametro2, "accensione/spegnimento")==0)
							 {
								 if(strcmp(parametro3, "ON")==0)
								 {
									 strcpy(interruttoreSistema, "ON");
									 stato=1;
								 }
								 else
								 {
									 strcpy(interruttoreSistema, "OFF");
									 stato=0;
								 }
								 
							 }else
							 {
								 printf("label inesistente per la centralina \n");
							 }
						 }
					   printf("esco dagli if centralina \n");
					   
					   sem_post(semaforoFifoComandi); //cosi il wait dopo non ci da problemi
					   	 
					 }else
					 {
				   
				   
				     if(strcmp(parametro3, "OFF")==0)
			         {
				        communication[8]='0';
			         }
			         else
			         {
				        communication[8]='1';
			         }
				  
				     communication[0]=comand;
			       
			       
			              
			       
			 
			         printf("centralina scrivo sulla pipe %s \n", communication);
			 
			         if(write(fdFifoComandi, communication, strlen(communication))==-1)
			         {
				        printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				        return EXIT_FAILURE;
			          }
			  
			         sem_getvalue(semaforoFifoComandi, &value);
			         printf("centralina valore prima del post %d \n", value);
			  
			      //conesnto alla centralina di legger la fifo
			         sem_post(semaforoFifoComandi);
			  
			         sem_getvalue(semaforoFifoComandi, &value);
			         printf("centralina valore  dopo post %d \n", value);
			  
			  
			         printf("centralina invio il segnale al padre \n");
			  
			     //attendo il feedback di ritorno del processo iniziale
			         //sigsuspend(&maschera);
			        checkValue=0;
			     
			     
			      //avviso la centralina che puo leggere
			         kill(getppid(), SIGUSR1);
			  
			         printf("centralina in attesa \n");
			  
			      
			  
			        while(checkValue!=1)
			        {
					}
			  
			        printf("centralina ottengo feedback del padre e leggo  \n");
			  
			         if(read(fdFifoComandi, communication, strlen(communication))==-1)
		             {
			            printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			            getchar();
			            return EXIT_FAILURE;
		             }
		      
		             printf("centralina legge %s \n", communication);
			  
			         if(communication[0]!='0')//se è stato trovato prima non ha senso continuare
			         {
						 if(numeroLampadine>0)
						 {
						 
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
							 printf("CENTRALINA LAMPADINA AVVISATA \n");
				  	         kill(vetPidBulb[i], SIGUSR1);
			              }
			
			       
			
		   	             while(checkValue<numeroLampadine)
			            {
				                
				      
			            } 
			            
			            printf("RITORNO DA LAMPADINE \n");
			       
			       
			            if(read(fdFifoComandi, communication, strlen(communication))==-1)
		                {
			               printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			               getchar();
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
				                printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                return EXIT_FAILURE;
			                 }
			                 
			                 checkValue=0;
				 
				             //lampadine direttamente connesse 
				            for(int i=0; i<(numeroFrigoriferi); i++)
			                {
				  	            kill(vetPidFridge[i], SIGUSR1);
			                }
			
			                 while(checkValue<(numeroFrigoriferi))
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
				                if(write(fdFifoComandi, communication, strlen(communication))==-1)
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
			
		     	               while(checkValue!=(numeroFinestre-1))
			                    {
				                   printf("checkvalue in iniziale è %d \n", checkValue);
				                   sigsuspend(&maschera);
				      
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
										 
										 printf("CENTRALINA DEVE AVVISARE HUB \n");
										 
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
											  printf("CENTRALINA HUB AVVISATO %d \n", vetPidHub[i]);
				  	                           kill(vetPidHub[i], SIGUSR1);
			                               }
			                               
			                               printf("CENTRALINA ATTESA \n");
			
			                              while(checkValue<(numeroHub))
			                             {
				                             
				      
			                             } 
			                             
			                             printf("RITORNO DA HUB \n");
			       
			       
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
		                                           
											   }
									           
									           
									           printf("COMMUNICATION ALLA FINE CENTRALINA %s \n", communication);
									           
									               if(communication[0]!='0')
									               {								                 
									                  printf("\nid non trovato. Probabilmente il dispositivo non esiste nel sistema");
								                   }else
								                   {
													   if(communication[1]=='X')
					                                   {                   
					                                           printf("switch completato \n");
					                                   }else
					                                   {
						                                        printf("la label specificata non è disponibile nel dispositivo \n");
					                                   }
												   }
								               }else
								               {
												   if(communication[1]=='X')
					                               {  
					                                   printf("switch completato \n");
					                                }else
					                               {
						                                printf("la label specificata non è disponibile nel dispositivo \n");
					                                }
											   }
								            }else
								            {
												if(communication[1]=='X')
					                            {  
					                                printf("switch completato \n");
					                            }else
					                            {
						                             printf("la label specificata non è disponibile nel dispositivo \n");
					                             }
											}
								  
							             }
							             else
				                         {
							               if(communication[1]=='X')
					                       {  
					                           printf("switch completato \n");
					                       }else
					                       {
						                        printf("la label specificata non è disponibile nel dispositivo \n");
					                        }
						                }
						           }
						           else
				                  {
							          if(communication[1]=='X')
					                  {  
					                      printf("switch completato \n");
					                  }else
					                  {
						                  printf("la label specificata non è disponibile nel dispositivo \n");
					                  }
						          }
					  
				              }
				              else
				              {
							     if(communication[1]=='X')
					             {  
					                printf("switch completato \n");
					             }else
					             {
						            printf("la label specificata non è disponibile nel dispositivo \n");
					             }
						      }
					   
					       }
					       else
				          {
						     if(communication[1]=='X')
					         {  
					             printf("switch completato \n");
					          }else
					          {
						         printf("la label specificata non è disponibile nel dispositivo \n");
					          }
					      }
					   
					   
				       }else
				       {
					      if(communication[1]=='X')
					      {  
					        printf("switch completato \n");
					      }else
					      {
						      printf("la label specificata non è disponibile nel dispositivo \n");
					      }
				      }
			    
			      }
			      
			      printf("esco dall annidamento \n");
			    
			    }else
			    {
					printf("pos non riconosciuto \n");
				}  
			   
		     }else
		     {
				 printf("label inesistente \n");
			 }
			 
		  }else
		  {
			  printf("id non disponibie \n");
		  }
		
		sem_wait(semaforoFifoComandi);//così ho fatto almeno una lettura quindi so che semaforo è uguale a 1 lo ridecremento a 0
		  
	  }else
	  {
		  if((strcmp(parametro1, "0")==0)&&(strcmp(parametro2, "accensione/spegnimento")==0)&&(strcmp(parametro3, "ON")==0))
		  {
			  
			  printf("centralina dentro accensione/spegnimento \n");
			  
			  strcpy(interruttoreSistema, "ON");
			  stato=1;
			  
			  if(strcmp(interruttoreMirroring, "ON")==0)
			  {
				  
				  printf("centralina devo gestire mirroring \n");
				  
				  char statoStringa[]="0";
									
									sprintf(statoStringa, "%d", stato);
				  
				  if(numeroLampadine>0)
				  {
					  
					  printf("devo avvisare lampadine \n");
				  
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
		
		printf("ritorno lampadine avvisate \n");
		
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
			
			printf("devo scrivere hub \n");
		
		for(int i=0; i<numeroHub;i++)
	    {
			printf("scrivo \n");
			
			if(write(fdHubInterruttore[i], statoStringa, strlen(statoStringa)+1)==-1)
			{
				printf("errore scrittura centralina hub \n");
				
				printf("statoStringa %s \n", statoStringa);
			}
			
			printf("ho scritto %s \n", statoStringa);
		}
		
		printf("devo avvisare hub \n");
	    
	    for(int i=0; i<numeroHub;i++)
	    {
			kill(vetPidHub[i], SIGINT);
			printf("AVVISATO \n");
		}
		
		
		
	   }
	   printf("ho terminato di avvisare \n");
   
			  }
			  
			  printf("centralina accesa \n");
			  getchar();
		  }
		  else
		  {
			  printf("centralina spenta, impossibile inserire comando switch \n");
		  }
	   }
		  
		  strcpy(communication, "XXXXXXXXX");
		 
		  
       }else if(strcmp(comando, "info")==0)
       {
		   
		  comand='3';
		  
		  if(strlen(parametro1)<4)
		   {
			 strcpy(id1, parametro1);
			 if(id1[0]>'9'||id1[0]<'0')
			 {
				 printf("errore con id");
			 }
			 if(id1[1]>'9'||id1[1]<'0')
			 {
				 id1[1]='X';
			 }
			 if(id1[2]>'9'||id1[2]<'0')
			 {
				 id1[2]='X';
			 }
			 communication[0]=comand;
			 communication[1]=id1[0];
			 communication[2]=id1[1];
			 communication[3]=id1[2];
			 
			 printf("centralina scrivo sulla pipe %s \n", communication);
			 
			 if(write(fdFifoComandi, communication, strlen(communication))==-1)
			  {
				  printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				  return EXIT_FAILURE;
			  }
			  
			  sem_getvalue(semaforoFifoComandi, &value);
			  printf("centralina valore prima del post %d \n", value);
			  
			   //conesnto alla centralina di legger la fifo
			  sem_post(semaforoFifoComandi);
			  
			  sem_getvalue(semaforoFifoComandi, &value);
			  printf("centralina valore  dopo post %d \n", value);
			  
			  
			  printf("centralina invio il segnale al padre \n");
			  
			  checkValue=0;
			  
			  //avviso la centralina che puo leggere
			  kill(getppid(), SIGUSR1);
			  
			  printf("centralina in attesa \n");
			  
			  //attendo il feedback di ritorno del processo iniziale
			  while(checkValue<1)
			  {
			  }
			  
			  printf("centralina ottengo feedback del padre e leggo  \n");
			  
			  if(read(fdFifoComandi, communication, strlen(communication))==-1)
		      {
			     printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			     getchar();
			     return EXIT_FAILURE;
		      }
		      
		      printf("centralina legge %s \n", communication);
			  
			  if(communication[0]!='0')//se è stato trovato prima non ha senso continuare
			  {
				
				if(numeroLampadine>0)
				{
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
			       
			       if(read(fdFifoComandi, communication, strlen(communication))==-1)
		           {
			          printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			          getchar();
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
				          if(write(fdFifoComandi, communication, strlen(communication))==-1)
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
									  
									  printf("avviso hub \n");
									  
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
								   printf("hub avvisato \n");
				  	              kill(vetPidHub[i], SIGUSR1);
			                   }
			
			                  printf("ritornato \n");
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
		                                           
											   }
									           
									           
									           printf("COMMUNICATION ALLA FINE CENTRALINA %s \n", communication);
									           
		                         
		                         if(communication[0]!='0')
		                         {
									 printf("\nid non trovato. Probabilmente il dispositivo non esiste nel sistema");
								 }else{
									 
									 //TERMOSTATI
									 
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
						   
						   printf("\ninformazioni reperite \n");
					      printf("%s \n", informazioni);
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
									 
								
								}else{
									//TENDE	 
									
									
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
						   
						   printf("\ninformazioni reperite \n");
					      printf("%s \n", informazioni);
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
									 
						}else{			 
					   /*
					    * dovrebbe esser sicuramente diverso da 0
					    * 
					    */ 
					   //qui ho trovato una lampadina
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
						   
						   printf("\ninformazioni reperite \n");
					      printf("%s \n", informazioni);
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
								  
							  }else{
					   /*
					    * dovrebbe esser sicuramente diverso da 0
					    * 
					    */ 
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
						   
						   printf("\ninformazioni reperite \n");
					      printf("%s \n", informazioni);
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
							  
							  
						  }else{
					   /*
					    * dovrebbe esser sicuramente diverso da 0
					    * 
					    */ 
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
						   
						   printf("\ninformazioni reperite \n");
					      printf("%s \n", informazioni);
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
					   
					   
				        }else{
					   /*
					    * dovrebbe esser sicuramente diverso da 0
					    * 
					    */ 
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
						   
						   printf("\ninformazioni reperite \n");
					      printf("%s \n", informazioni);
						   
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   } 
					   
					   
					   
					   
				   }else{
					   /*
					    * dovrebbe esser sicuramente diverso da 0
					    * 
					    */ 
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
						   
						   if(read(fdLampadineComandi[indice], informazioni, 500)==-1)
						   {
							   printf("errore lettura pipe lampadina\n");
						   }
						   
						   printf("\ninformazioni reperite \n");
					      printf("%s \n", informazioni);
					  
						   
						   
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
				  
			  }else{
				  
				  
				  if(pidSender!=0)
				  {
					  char informazioni[500];
					  
					  if(read(pipeProcessoIniziale, informazioni, 500)==-1)
					  {
						  printf("ERRORE LETTURA CENTRALINA PIPE \n");
					  }
					  
					  printf("\ninformazioni reperite \n");
					  printf("%s \n", informazioni);
					  
				  }
				  
				  pidSender=0;
			  }
			  
		  
			  
			  
			 
			   
		   }
		   else
		   {
			   printf("id non accettabile");
		   }
		  
	          sem_wait(semaforoFifoComandi);
		   
          	 //ripristina communication
			  strcpy(communication, "XXXXXXXXX");	   
		   
		   
		   
	   
       }
       else if(strcmp(comando, "help")==0)
       {
		   if(strcmp(parametro1, "list")==0)
           {    
		      printf("il comando list elenca tutti i dispositivi");
           }
           else if(strcmp(parametro1, "add")==0)
           {
		      printf("il comando add aggiunge un device al sistema e ne specifica i dettagli");
           }
           else if(strcmp(parametro1, "del")==0)
           {
		      printf("il comando del rimuove il dispositivo specificato. se ad esso vi sono dispositivi connessi elimina anche quelli");
           }
           else if(strcmp(parametro1, "link")==0)
           {
		       printf("il comando link collega due dispositivi tra loro");
               
           }
           else if(strcmp(parametro1, "switch")==0)
           {
		       printf("il comando switch modifica l'interruttore del dispositivo specificato nella posizione indicata");
           }
           else if(strcmp(parametro1, "info")==0)
           {
		       printf("il comando info mostra i dettagli del dispositivo specificato");
           }
           else
           {
			   printf("comando non presente tra i comandi disponibili");
		   }
		   
	   }
	   else if(strcmp(parametro1, "exit")==0)
	   {
		   ext=1;
		    kill(getppid(), SIGTERM);
	   }
	   else
	   {
		   printf("comando non riconosciuto");
	   }
    
    fflush(stdout);
    
    //attende inserimento di un carattere per continuare
    getchar();
	
	//ripristina communication
    strcpy(communication, "XXXXXXXXX");
    
    int value;
			sem_getvalue(semaforoFifoComandi, &value);
			printf("VALORE SEMAFORO TERMINE COMANDO %d \n", value);
			
  
    
    
    } while(ext!=1);//verifico se l utente ha inserito exit nel caso termino il programma
    
    sem_post(semaforoEsterno);
    
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

//implementazione dell handler per il segnale utente 1.
void handlerLetturaPipe(int signum)
{
	
	/*
	 * NOTA
	 * come al solito questo non dovrebbe far nulla
	 * ho un dubiio sulla gestione del cambio dello stato
	 * probabilmente ce qualche problema se segnali si sovrappongono
	 */ 
	 checkValue=checkValue+1;
}



void handlerNack(int signum)
{
	control=0;
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
	            
	
	//avviso del rischio di mirroring
	printf("\nATTENZIONE INTERRUTTORE ATTIVATO ESTERNAMENTE O NUOVO DISPOSITIVO CONNESSO ALLA CENTRALINA, RISCHIO MIRRORING \n");
	
	
	//aumento il checkvalue1 che serve a indicare l'avvenuto mirroring
	checkValue1=checkValue1+1;
	
	
}

void handlerAck(int signum, siginfo_t *siginfo, void *context)
{
	 pidSender=siginfo->si_pid;
	 checkValue=checkValue+1;

}
