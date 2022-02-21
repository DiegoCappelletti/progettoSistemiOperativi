/*
 * 
 * processo iniziale 
 * il processo iniziale esegue le seguenti azioni in sequenza
 * 1) genera il gestore degli ID
 * 2)imposta i dispositivi presenti nel sistema
 * 3)avvia la centralina
 * 4)si mette in attesa di un segnale per eseguire i comandi inviati dalla centralina leggendo da una fifo 
 * 5)alla terminazione rimuove tutte le fifo e i semafori con nome
 * 
 * 
 * NOTE
 * non so per il malloc dei vettori dinamici
 * non so per i privilegi di apertura semafori
 * RICORDA DI AGGIORNARE ADD CON LE PIPE
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

//DICHIARAZIONI FUNZIONI
//handler per gestione segnale lettura da fifo comandi
void handlerLetturaFifo(int signum);
//gestore per il segnale di delete
void handlerAck(int signum, siginfo_t *siginfo, void *context);

//VARIABILI GLOBALI
pid_t pidSender=0;

int checkValue=0;

int main(int argc, char **argv)
{
	//GENERAZIONE TOOL ESTERNO
	
	//creazione fifo tool esterno
	//in questa fifo vengono diffusi i pid e gli id dei dispositivi
	char* fifoIdEsterno="./fifoIdEsterno";
	int fdFifoIdEsterno;//quando apri la fifo la metti qui
	if(mkfifo(fifoIdEsterno, 0666)==-1)
	{
		printf("ERRORE CREAZIONE FIFO ID ESTERNO IN PROCESSO INIZIALE  \n");
	    getchar();
	}
	fdFifoIdEsterno=open(fifoIdEsterno,O_RDWR);
	if(fdFifoIdEsterno==-1)
    {
	    printf("ERRORE APERTURA FIFO ID ESTERNO IN PROCESSO INIZIALE IL PROGRAMMA CONTINUERA' MA NON SARA' POSSIBILE USARE IL TOOL ESTERNo \n");
		getchar(); 
			
	}
	
	
	/*
	int status;
	
		//apro o creo il semaforo per lettura stdin
	sem_t* semaforoEsterno=sem_open("EsSm", O_CREAT, 0666, 0);
	if(semaforoEsterno==-1)
	{
		printf("ERRORE APERTURA SEMAFORO \n");
	}
	
	sem_post(semaforoEsterno);
	
	if(fork()==0)
	{
		
		printf("sono il figlio \n");
		status=system("./toolEsternov.1");
		return 0;
	}
	
	printf("processo iniziale sta continuando \n");
	
	*/
	
	
	//GENERAZIONE ELEMENTI PRINCIPALI DEL SISTEMA
	
	//creazione fifo comandi
	//in questa fifo vengono diffusi a tutto il sistema i comandi del dispostivo
	char* fifoComandi="./fifoComandi";
	int fdFifoComandi;//quando apri la fifo la metti qui
	if(mkfifo(fifoComandi, 0666)==-1)
	{
		printf("ERRORE CREAZIONE PIPE COMANDI \n");
	}
	fdFifoComandi=open(fifoComandi,O_RDWR);
	if(fdFifoComandi==-1)
		{
			printf("ERRORE APERTURA PIPE IN FIFO COMANDI \n");
			getchar();
			return EXIT_FAILURE;
		}
	//vettore di comunicazione
    char communication[]="XXXXXXXXX";
    //per l'elaborazione del comando
    char comand;
    char id1[3];
    char device;
    char id2[3];
    char pos;
    
    //apro o creo il semaforo per la lettura dalla fifo comandi
	sem_t* semaforoFifoComandi=sem_open("IdCm", O_CREAT, 0666, 0);
	if(semaforoFifoComandi==-1)
	{
		printf("ERRORE APERTURA SEMAFORO \n");
	}
	
	int value;
			  sem_getvalue(semaforoFifoComandi, &value);
			  printf("valore creazione semaforo fifo comandi %d \n", value);
			  
	
	//ora genero il processo che gestisce gli id
	int pidGestoreID=fork();
	//inseriamo il pid generato all'interno di un vettore per passarlo come parametro
	char bufferPidGestoreID[10];
	sprintf(bufferPidGestoreID, "%d", pidGestoreID);
	if (pidGestoreID==-1)
	{
		printf("ERRORE CREAZIONE PROCESSO ID \n");
	}
	
	//argomenti da passare al generatore di id
	char *argsID[]={"./gestoreID", NULL};
	//il figlio esegue gestore ID
     if (pidGestoreID==0)
	 {
	   execvp(argsID[0], argsID);
	 }
	 
	 //apro o creo il semaforo con nome del gestore ID
	sem_t* semaforoPipeID=sem_open("IdSm", O_CREAT, 0666, 0);
	if(semaforoPipeID==-1)
	{
		printf("ERRORE APERTURA SEMAFORO \n");
	}
	
	//generazione gestore linl
	
	pid_t pidLinker=fork();
	
	char *argsLinker[]={"./gestoreLink", NULL};
	
	if(pidLinker==0)
	{
		execvp(argsLinker[0], argsLinker);
	}
	
	int pidLinkerInt=pidLinker;
	
	char pidLinkerString[100];
	
	sprintf(pidLinkerString, "%d", pidLinkerInt);
	
	
	char attivazione[]="standard";
	
	//IMPOSTAZIONE INIZIALE DEI DISPOSITIVI DISPONIBILI
	
	/*
	 * ci sono da sistemare i clear
	 * 
	 */ 
	
	
	//non di controllo
	
	//lampadine
	printf("\n\n");
	printf("setting dei parametri iniziali dei dispositivi \n");
	printf("inserisci il numero di lampadine ");
	//numero lampadine da gestire
	int numeroLampadine=0;
	scanf("%d",&numeroLampadine);
	getchar();
	
	//pulisco la shell
	//clear();
	
	//vettore dei pid delle lampadine non connesse a nulla
	pid_t *vetPidBulb=(pid_t *)malloc(numeroLampadine*sizeof(pid_t));
    
    int *fdLampadineComandi;
    fdLampadineComandi=(int *)malloc(numeroLampadine*sizeof(int));

    
	
	for (int i=0; i<numeroLampadine; i++)
	{
		
	  int fdLampadinaComandi[2];
	  if(pipe(fdLampadinaComandi)==-1)
	  {
		 printf("ERRORE PIPE \n");
		 return EXIT_FAILURE;
	  }
	
	 fcntl(fdLampadinaComandi[0], F_SETFL, O_NONBLOCK);
	
	 char pipeComandi[100];
	 sprintf(pipeComandi, "%d", fdLampadinaComandi[1]); 

	   
	   //reperisco l'id dalla pipe id
	   //vettore per contenere l'id reperito
	   
       char* myfifo="./fifoID";
	   int fdFifoID=open(myfifo,O_RDWR);
	   if(fdFifoID==-1)
	   {
		   printf("\n ERRORE APERTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
		   
	   }
	
	   
	  
	   kill(pidGestoreID, SIGUSR1);
	
	   sem_wait(semaforoPipeID);
	
       char idReperito[]="XXX";
	   if(read(fdFifoID, idReperito, strlen(idReperito))==-1)
	   {
		   
		   printf("\n ERRORE LETTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	   
	   
	   
	   //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	   char *args[]={"./lampadina", idReperito, attivazione, pidLinkerString, pipeComandi, NULL};
	   
	   //creo un processo figlio
	   fflush(stdout);
	   vetPidBulb[i]=fork();
	   if(vetPidBulb[i]==-1)
	   {
		   printf("\n ERRORE CREAZIONE LAMPADINA. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	   //il figlio esegue lampadina
       if (vetPidBulb[i]==0)
	   {
		   close(fdLampadinaComandi[0]);
	      execvp(args[0], args);

	   }
	   
	   close(fdLampadinaComandi[1]);
	   fdLampadineComandi[i]=fdLampadinaComandi[0];
	 
	   close(fdFifoID);
	
	   
	   
	   
	   
	   //devo passare info al tool esterno
	   char messaggio[100];
	   
	   sprintf(messaggio, "0X0X%sX%d|",idReperito, vetPidBulb[i]);
	   
	   if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	   {
		   printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		   getchar();
	   }
	   
	  
	   
	}
	
	
	//window
	printf("\n\n");
	printf("setting dei parametri iniziali dei dispositivi \n");
	printf("inserisci il numero di finestre ");
	//numero finestre da gestire
	int numeroFinestre=0;
	scanf("%d",&numeroFinestre);
	getchar();
	
	//pulisco la shell
	//clear();
	
	//vettore dei pid delle finestre non connesse a nulla
	pid_t *vetPidWindow=(pid_t *)malloc(numeroFinestre*sizeof(pid_t));
    
int *fdFinestreComandi;
    fdFinestreComandi=(int *)malloc(numeroFinestre*sizeof(int));
    
	
	for (int i=0; i<numeroFinestre; i++)
	{
		int fdFinestraComandi[2];
	  if(pipe(fdFinestraComandi)==-1)
	  {
		 printf("ERRORE PIPE \n");
		 return EXIT_FAILURE;
	  }
	
	 fcntl(fdFinestraComandi[0], F_SETFL, O_NONBLOCK);
	
	 char pipeComandi[100];
	 sprintf(pipeComandi, "%d", fdFinestraComandi[1]); 

	   
	   
	   //reperisco l'id dalla pipe id
	   //vettore per contenere l'id reperito
	   
       char* myfifo="./fifoID";
	   int fdFifoID=open(myfifo,O_RDWR);
	   if(fdFifoID==-1)
	   {
		   printf("\n ERRORE APERTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	
	   
	  
	   kill(pidGestoreID, SIGUSR1);
	
	   sem_wait(semaforoPipeID);
	
       char idReperito[]="XXX";
	   if(read(fdFifoID, idReperito, strlen(idReperito))==-1)
	   {
		   printf("\n ERRORE LETTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
		   
	   }
	   
	   
	   
	   //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	   char *args[]={"./window", idReperito, attivazione, pidLinkerString, pipeComandi, NULL};
	   
	   //creo un processo figlio
	   fflush(stdout);
	   vetPidWindow[i]=fork();
	   if(vetPidWindow[i]==-1)
	   {
		   printf("\n ERRORE CREAZIONE FINESTRA. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	   //il figlio esegue finestra
       if (vetPidWindow[i]==0)
	   {
		  close(fdFinestraComandi[0]);
	      execvp(args[0], args);

	   }
	 
	 
	 close(fdFinestraComandi[1]);
	   fdFinestreComandi[i]=fdFinestraComandi[0];
	 
	 close(fdFifoID);
	
	  //devo passare info al tool esterno
	   char messaggio[100];
	   
	   sprintf(messaggio, "0X1X%sX%d|",idReperito, vetPidWindow[i]);
	   
	   if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	   {
		   printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		   getchar();
	   }
	   
	   printf("%s \n", messaggio);
	   
	}
	
	//fridge
	printf("\n\n");
	printf("setting dei parametri iniziali dei dispositivi \n");
	printf("inserisci il numero di frigoriferi ");
	//numero frigoriferi da gestire
	int numeroFrigoriferi=0;
	scanf("%d",&numeroFrigoriferi);
	getchar();
	
	//pulisco la shell
	//clear();
	
	//vettore dei pid dei frigoriferi non connessi a nulla
	pid_t *vetPidFridge=(pid_t *)malloc(numeroFrigoriferi*sizeof(pid_t));
    
    int *fdFrigoriferiComandi;
    fdFrigoriferiComandi=(int *)malloc(numeroFrigoriferi*sizeof(int));

	
	for (int i=0; i<numeroFrigoriferi; i++)
	{
		int fdFrigoriferoComandi[2];
	  if(pipe(fdFrigoriferoComandi)==-1)
	  {
		 printf("ERRORE PIPE \n");
		 return EXIT_FAILURE;
	  }
	
	 fcntl(fdFrigoriferoComandi[0], F_SETFL, O_NONBLOCK);
	
	 char pipeComandi[100];
	 sprintf(pipeComandi, "%d", fdFrigoriferoComandi[1]); 

		
	   
	   
	   //reperisco l'id dalla pipe id
	   //vettore per contenere l'id reperito
	   
       char* myfifo="./fifoID";
	   int fdFifoID=open(myfifo,O_RDWR);
	   if(fdFifoID==-1)
	   {
		   printf("\n ERRORE APERTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	
	   
	  
	   kill(pidGestoreID, SIGUSR1);
	
	   sem_wait(semaforoPipeID);
	
       char idReperito[]="XXX";
	   if(read(fdFifoID, idReperito, strlen(idReperito))==-1)
	   {
		   printf("\n ERRORE LETTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
		   
	   }
	   
	   
	   
	   //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	   char *args[]={"./fridge", idReperito, attivazione, pidLinkerString, pipeComandi, NULL};
	   
	   //creo un processo figlio
	   fflush(stdout);
	   vetPidFridge[i]=fork();
	   if(vetPidFridge[i]==-1)
	   {
		  printf("\n ERRORE CREAZIONE FRIGORIFERO. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	   //il figlio esegue frigorifero
       if (vetPidFridge[i]==0)
	   {
		   close(fdFrigoriferoComandi[0]);
	      execvp(args[0], args);

	   }
	 
	 
	 
	 close(fdFrigoriferoComandi[1]);
	   fdFrigoriferiComandi[i]=fdFrigoriferoComandi[0];
	 
	 close(fdFifoID);
	
	//devo passare info al tool esterno
	   char messaggio[100];
	   
	   sprintf(messaggio, "0X2X%sX%d|",idReperito, vetPidFridge[i]);
	   
	   if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	   {
		   printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		   getchar();
	   }
	   
	   printf("%s \n", messaggio);
	   
	   
	}
	
	
	//termostato
	printf("\n\n");
	printf("setting dei parametri iniziali dei dispositivi \n");
	printf("inserisci il numero di termostati ");
	//numero termostati da gestire
	int numeroTermostati=0;
	scanf("%d",&numeroTermostati);
	getchar();
	
	//pulisco la shell
	//clear();
	
	//vettore dei pid dei termostati non connessi a nulla
	pid_t *vetPidTermostato=(pid_t *)malloc(numeroTermostati*sizeof(pid_t));
    int *fdTermostatiComandi;
    fdTermostatiComandi=(int *)malloc(numeroTermostati*sizeof(int));

    
	
	for (int i=0; i<numeroTermostati; i++)
	{
		
		int fdTermostatoComandi[2];
	  if(pipe(fdTermostatoComandi)==-1)
	  {
		 printf("ERRORE PIPE \n");
		 return EXIT_FAILURE;
	  }
	
	 fcntl(fdTermostatoComandi[0], F_SETFL, O_NONBLOCK);
	
	 char pipeComandi[100];
	 sprintf(pipeComandi, "%d", fdTermostatoComandi[1]); 

		
	   
	   
	   //reperisco l'id dalla pipe id
	   //vettore per contenere l'id reperito
	   
       char* myfifo="./fifoID";
	   int fdFifoID=open(myfifo,O_RDWR);
	   if(fdFifoID==-1)
	   {
		   printf("\n ERRORE APERTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	
	   
	  
	   kill(pidGestoreID, SIGUSR1);
	
	   sem_wait(semaforoPipeID);
	
       char idReperito[]="XXX";
	   if(read(fdFifoID, idReperito, strlen(idReperito))==-1)
	   {
		   printf("\n ERRORE LETTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
		   
	   }
	   
	   
	   //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	   char *args[]={"./termostato", idReperito, attivazione, pidLinkerString, pipeComandi, NULL};
	   
	   //creo un processo figlio
	   fflush(stdout);
	   vetPidTermostato[i]=fork();
	   if(vetPidTermostato[i]==-1)
	   {
		   printf("\n ERRORE CREAZIONE TERMOSTATO. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	   //il figlio esegue termostato
       if (vetPidTermostato[i]==0)
	   {
		   close(fdTermostatoComandi[0]);
	      execvp(args[0], args);

	   }
	 
	 
	 close(fdTermostatoComandi[1]);
	   fdTermostatiComandi[i]=fdTermostatoComandi[0];
	 
	 close(fdFifoID);
	
	//devo passare info al tool esterno
	   char messaggio[100];
	   
	   sprintf(messaggio, "0X4X%sX%d|",idReperito, vetPidTermostato[i]);
	   
	   if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	   {
		   printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		   getchar();
	   }
	   
	   printf("%s \n", messaggio);
	   
	   
	}
	
	//tende
	printf("\n\n");
	printf("setting dei parametri iniziali dei dispositivi \n");
	printf("inserisci il numero di tende ");
	//numero tende da gestire
	int numeroTende=0;
	scanf("%d",&numeroTende);
	getchar();
	
	//pulisco la shell
	//clear();
	
	//vettore dei pid delle tende non connesse a nulla
	pid_t *vetPidTenda=(pid_t *)malloc(numeroTende*sizeof(pid_t));
    
    int *fdTendeComandi;
    fdTendeComandi=(int *)malloc(numeroTende*sizeof(int));

	
	for (int i=0; i<numeroTende; i++)
	{
		
int fdTendaComandi[2];
	  if(pipe(fdTendaComandi)==-1)
	  {
		 printf("ERRORE PIPE \n");
		 return EXIT_FAILURE;
	  }
	
	 fcntl(fdTendaComandi[0], F_SETFL, O_NONBLOCK);
	
	 char pipeComandi[100];
	 sprintf(pipeComandi, "%d", fdTendaComandi[1]); 
	   
	   
	   //reperisco l'id dalla pipe id
	   //vettore per contenere l'id reperito
	   
       char* myfifo="./fifoID";
	   int fdFifoID=open(myfifo,O_RDWR);
	   if(fdFifoID==-1)
	   {
		   printf("\n ERRORE APERTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	
	   
	  
	   kill(pidGestoreID, SIGUSR1);
	
	   sem_wait(semaforoPipeID);
	
       char idReperito[]="XXX";
	   if(read(fdFifoID, idReperito, strlen(idReperito))==-1)
	   {
		   printf("\n ERRORE LETTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
		   
	   }
	   
	   
	   
	   //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	   char *args[]={"./tenda", idReperito, attivazione, pidLinkerString, pipeComandi, NULL};
	   
	   //creo un processo figlio
	   fflush(stdout);
	   vetPidTenda[i]=fork();
	   if(vetPidTenda[i]==-1)
	   {
		   printf("\n ERRORE CREAZIONE TENDA. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	   //il figlio esegue tenda
       if (vetPidTenda[i]==0)
	   {
		   close(fdTendaComandi[0]);
	      execvp(args[0], args);

	   }
	 
	 
	 close(fdTendaComandi[1]);
	   fdTendeComandi[i]=fdTendaComandi[0];
	 
	 
	 close(fdFifoID);
	
	//devo passare info al tool esterno
	   char messaggio[100];
	   
	   sprintf(messaggio, "0X3X%sX%d|",idReperito, vetPidTenda[i]);
	   
	   if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	   {
		   printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		   getchar();
	   }
	   
	   printf("%s \n", messaggio);
	   
	   
	}
	
	//di controllo
	
	//hub
	printf("\n\n");
	printf("setting dei parametri iniziali dei dispositivi \n");
	printf("inserisci il numero di hub ");
	//numero hub da gestire
	int numeroHub=0;
	scanf("%d",&numeroHub);
	getchar();
	
	printf("numero hub schermo %d \n", numeroHub);
	
	//pulisco la shell
	//clear();
	
	//vettore dei pid degli hub non connessi a nulla
	pid_t *vetPidHub=(pid_t *)malloc(numeroHub*sizeof(pid_t));
    int *fdHubComandi;
    fdHubComandi=(int *)malloc(numeroHub*sizeof(int));

    
	
	for (int i=0; i<numeroHub; i++)
	{
		
int fdHub1Comandi[2];
	  if(pipe(fdHub1Comandi)==-1)
	  {
		 printf("ERRORE PIPE \n");
		 return EXIT_FAILURE;
	  }
	
	 fcntl(fdHub1Comandi[0], F_SETFL, O_NONBLOCK);
	
	 char pipeComandi[100];
	 sprintf(pipeComandi, "%d", fdHub1Comandi[1]); 
	   
	   
	   //reperisco l'id dalla pipe id
	   //vettore per contenere l'id reperito
	   
       char* myfifo="./fifoID";
	   int fdFifoID=open(myfifo,O_RDWR);
	   if(fdFifoID==-1)
	   {
		   printf("\n ERRORE APERTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	
	   
	  
	   kill(pidGestoreID, SIGUSR1);
	
	   sem_wait(semaforoPipeID);
	
       char idReperito[]="XXX";
	   if(read(fdFifoID, idReperito, strlen(idReperito))==-1)
	   {
		   printf("\n ERRORE LETTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
		   
	   }
	   
	   
	   
	   //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	   char *args[]={"./hub", idReperito, attivazione, pidLinkerString, pipeComandi,  NULL};
	   
	   //creo un processo figlio
	   fflush(stdout);
	   vetPidHub[i]=fork();
	   if(vetPidHub[i]==-1)
	   {
		   printf("\n ERRORE CREAZIONE HUB. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	   //il figlio esegue hub
       if (vetPidHub[i]==0)
	   {
		   close(fdHub1Comandi[0]);
	      execvp(args[0], args);

	   }
	 
	 
	 close(fdHub1Comandi[1]);
	   fdHubComandi[i]=fdHub1Comandi[0];
	 
	 close(fdFifoID);
	
	
	//devo passare info al tool esterno
	   char messaggio[100];
	   
	   sprintf(messaggio, "0X6X%sX%d|",idReperito, vetPidHub[i]);
	   
	   if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	   {
		   printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		   getchar();
	   }
	   
	   printf("%s \n", messaggio);
	   
	   
	}
	
	//timer
	printf("\n\n");
	printf("setting dei parametri iniziali dei dispositivi \n");
	printf("inserisci il numero di timer ");
	//numero timer da gestire
	int numeroTimer=0;
	scanf("%d",&numeroTimer);
	getchar();
	
	//pulisco la shell
	//clear();
	
	//vettore dei pid dei timer non connessi a nulla
	pid_t *vetPidTimer=(pid_t *)malloc(numeroTimer*sizeof(pid_t));
    int *fdTimerComandi;
    fdTimerComandi=(int *)malloc(numeroTimer*sizeof(int));

    
	
	for (int i=0; i<numeroTimer; i++)
	{
		
int fdTimer1Comandi[2];
	  if(pipe(fdTimer1Comandi)==-1)
	  {
		 printf("ERRORE PIPE \n");
		 return EXIT_FAILURE;
	  }
	
	 fcntl(fdTimer1Comandi[0], F_SETFL, O_NONBLOCK);
	
	 char pipeComandi[100];
	 sprintf(pipeComandi, "%d", fdTimer1Comandi[1]); 
	   
	   
	   //reperisco l'id dalla pipe id
	   //vettore per contenere l'id reperito
	   
       char* myfifo="./fifoID";
	   int fdFifoID=open(myfifo,O_RDWR);
	   if(fdFifoID==-1)
	   {
		   printf("\n ERRORE APERTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	
	   
	  
	   kill(pidGestoreID, SIGUSR1);
	
	   sem_wait(semaforoPipeID);
	
       char idReperito[]="XXX";
	   if(read(fdFifoID, idReperito, strlen(idReperito))==-1)
	   {
		   printf("\n ERRORE LETTURA FIFO ID NEL PROCESSO INIZIALE. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
		   
	   }
	   
	   
	   
	   //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	   char *args[]={"./timer", idReperito, attivazione, pidLinkerString, pipeComandi, NULL};
	   
	   //creo un processo figlio
	   fflush(stdout);
	   vetPidTimer[i]=fork();
	   if(vetPidTimer[i]==-1)
	   {
		   printf("\n ERRORE CREAZIONE TIMER. PREMERE UN PULSANTE PER TERMINARE \n");
		   getchar();
		   return EXIT_FAILURE;
	   }
	   //il figlio esegue timer
       if (vetPidTimer[i]==0)
	   {
		   close(fdTimer1Comandi[0]);
	      execvp(args[0], args);

	   }
	 
	 
	 close(fdTimer1Comandi[1]);
	   fdTimerComandi[i]=fdTimer1Comandi[0];
	 
	 close(fdFifoID);
	
	//devo passare info al tool esterno
	   char messaggio[100];
	   
	   sprintf(messaggio, "0X5X%sX%d|",idReperito, vetPidTimer[i]);
	   
	   if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	   {
		   printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		   getchar();
	   }
	   
	   printf("%s \n", messaggio);
	   
	   
	}
	
	//IMPOSTAZIONI INIZIALI CENTRALINA
	
	/*
	 * la centralina può esser spenta all'inizio?
	 * 
	 */ 
	
	printf("\n\n");
	printf("setting dei parametri iniziali della centralina \n");
	printf("inserire la posizione del suo interruttore di accensione \n");
	printf("ON/OFF: ");
	//questo vettore memorizza il setting iniziale dello stato 
    char interruttore[100];
	scanf("%s", interruttore);
	//puliamo il buffer dall'escape
	getchar();
	
	
	if((strcmp(interruttore, "ON")!=0)&&(strcmp(interruttore, "OFF")!=0))//controllo valore inserito
	{
		
		//copio off nell interruttore
		strcpy(interruttore, "OFF");
		
		//indico errore e chiamo terminazione
		printf("\n è stato inserito un valore di interruttore non valido. il programma continua con valore OFF \n");
		printf("\n premere un pulsante qualunque per continuare \n");
		getchar();
		
	}

    clear();
    
   
	
	pid_t pidController;
	
	int fdControllerComandi[2]; //da notare che anche se il processo iniziale è un padre questo viene usato in scrittura
	
	if(pipe(fdControllerComandi)==-1)
	{
		printf("ERRORE PIPE \n");
		return EXIT_FAILURE;
	}
	
	
	
	char pipeComandi[100];
	sprintf(pipeComandi, "%d", fdControllerComandi[0]);
	
	pidController=fork();
	
	
	printf("iniziale il pid linker che passo a centralina è %s \n", pidLinkerString);
	
	char *args[]={"./centralina", "0", interruttore , bufferPidGestoreID, pidLinkerString, pipeComandi, NULL};
    if (pidController==0)
    {
	   execvp(args[0], args);
	}
	
	close(fdControllerComandi[0]);
	
	
	//ATTESA DI TERMINAZIONE PROCESSI CENTRALINA ETC
	
	/*
	 * direi a priori che sia sufficiente la terminazione della centralina 
	 * tra l altro penso che sia necessario servire la centralina in continuo quindi forse si dovrà anche toglierla sta parte
	 */ 
	
	
	
	//cicli operativi
	/*
	 * per intanto così ma plausibilmente andrà modificata l'uscita per renderla centralizzata
	 * 
	 */ 
	
    //dichiarazione della maschera
    sigset_t maschera;
    //il set viene riempito con tutti i segnali di sistema
    sigfillset(&maschera);
    // con questa funzione tolgo il segnale SIGUSR1 che mi gestisce il cambio di valore
    sigdelset(&maschera, SIGUSR1);
     // con questa funzione tolgo il segnale SIGUSR1 che mi gestisce il cambio di valore
    sigdelset(&maschera, SIGUSR2);
    // con questa funzione tolgo il segnale SIGUSR1 che mi gestisce il cambio di valore
    sigdelset(&maschera, SIGKILL);
    // con questa funzione tolgo il segnale SIGUSR1 che mi gestisce il cambio di valore
    sigdelset(&maschera, SIGTERM);
    
    //handler per la gestione del delete
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_sigaction=&handlerAck;
	act.sa_flags=SA_SIGINFO;
	sigaction(SIGUSR2, &act, NULL);
    
   struct sigaction act1;
	memset(&act, '\0', sizeof(act1));
	act1.sa_sigaction=&handlerLetturaFifo;
	act1.sa_flags=SA_RESTART;
	sigaction(SIGUSR1, &act1, NULL);
        //imposto l'handler di lettura FIFO	
	//signal(SIGUSR1, handlerLetturaFifo);
	
	while(1)
	{
		
		printf("IL NUMERO DI LAMPADINE IN INIZIALE E' %d \n", numeroLampadine);
		
		//ripristino checkValue
		checkValue=0;
		
		//calcolo del num
		int num=numeroLampadine+numeroFrigoriferi+numeroFinestre+numeroHub+numeroTimer;
		
		sigsuspend(&maschera);
		
		printf("processo iniziale esce suspend \n");
		
		//apro la fifo comandi
		//fdFifoComandi=open(fifoComandi,O_RDWR);
		
		
		sem_getvalue(semaforoFifoComandi, &value);
	    printf("valore prima del wait in iniziale %d \n", value);
		
		printf("processo iniziale in attesa di post centralina \n");
	    sem_wait(semaforoFifoComandi);
	    printf("processo iniziale esce attesa post\n");
	    
	    sem_getvalue(semaforoFifoComandi, &value);
	    printf("valore dopo del wait in iniziale %d \n", value);
		
		if(read(fdFifoComandi, communication, strlen(communication))==-1)
		{
			printf("ERRORE LETTURA PIPE DA PARTE DI PADRE \n");
			getchar();
			return EXIT_FAILURE;
		}
		//qui non serve replicare il comando sulla pipe
		
		printf("letta pipe %s \n", communication);
		
		//scomposizione del comando
		comand=communication[0];
		id1[0]=communication[1];
		id1[1]=communication[2];
		id1[2]=communication[3];
		device=communication[4];
		id2[0]=communication[5];
		id2[1]=communication[6];
		id2[2]=communication[7];
		pos=communication[8];
		
		//gestione di add
		if(comand=='1')
		{
			
			//reperisco l'id dalla pipe id
	        //vettore per contenere l'id reperito
	   
            char* myfifo="./fifoID";
	        int fdFifoID=open(myfifo,O_RDWR);
	        if(fdFifoID==-1)
	        {
		       printf("ERRORE APERTURA FIFO \n");
	        }
	
	   
	        kill(pidGestoreID, SIGUSR1);
	
	        sem_wait(semaforoPipeID);
	
            char idReperito[]="XXX";
	        if(read(fdFifoID, idReperito, strlen(idReperito))==-1)
	        {
		       printf("ERRORE LETTURA PIPE \n");
		   
	        }
	   
	        int control=0; //variabile per controllo 
	        pid_t pidNuovoDispositivo;//pid del nuovo dispositivo
	   
			//controllo il device che si vuol aggiungere al sistema
			if(device=='0')
			{
				fflush(stdout);
				pidNuovoDispositivo=fork();
				
				if(pidNuovoDispositivo==-1)//gestione errore
	            {
					
		            control=0;//setto il controllo a zero
				}else
				{
				
	       			//aggiorno il numero di lampadine 
				
			    	numeroLampadine=numeroLampadine+1;//aumento il numero di lampadine
				
				    //creo un vettore dinamico di lampadine con gli stessi pid di prima ma piu lungo di uno
				    pid_t *vetPidBulb1=(pid_t *)malloc(numeroLampadine*sizeof(pid_t));
				    for(int i=0; i<(numeroLampadine-1); i++)
				    {
					   vetPidBulb1[i]=vetPidBulb[i];
				    }
				
				    //libero la memori puntata in precedenza dal vettore lampadine
				    free(vetPidBulb);
				
				    //scambio i riferimenti tra l originale vettore pid della lampadina e quell nuovo creato
				    vetPidBulb=vetPidBulb1;
				
				    //creo un figlio nell ultimo indice del vettore
			       	
				    vetPidBulb[numeroLampadine-1]=pidNuovoDispositivo;
	            
	           
	           
	            
	                 //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	                 char *args[]={"./lampadina", idReperito, attivazione, pidLinkerString, NULL};
	   
	                //il figlio esegue lampadina
                    if (vetPidBulb[numeroLampadine-1]==0)
	                {
	                   execvp(args[0], args);
	                }
	                
	                //devo passare info al tool esterno
	               char messaggio[100];
	   
	               sprintf(messaggio, "0X0X%sX%d|",idReperito, vetPidBulb[numeroLampadine-1]);
	   
	               if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	               {
		                printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		               getchar();
	               }
	   
	            
	                control=1;//operazione andata a buon fine imposto control a 1   
			    }
	            
				
			}else if(device=='1')
	        {
				 //aggiorno il numero di finestre come per lampadine
				fflush(stdout);
				pidNuovoDispositivo=fork();
				
				if(pidNuovoDispositivo==-1)//gestione errore
	            {
					
		            control=0;//setto il controllo a zero
				}else
				{
					
				    numeroFinestre=numeroFinestre+1;
				    pid_t *vetPidWindow1=(pid_t *)malloc(numeroFinestre*sizeof(pid_t));
				    for(int i=0; i<(numeroFinestre-1); i++)
			     	{
					   vetPidWindow1[i]=vetPidWindow[i];
				    }
				
				    free(vetPidWindow);
				
				    vetPidWindow=vetPidWindow1;
				
				
				    
				    vetPidWindow[numeroFinestre-1]=pidNuovoDispositivo;
	            
	           
	           
	            
	                //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	                char *args[]={"./finestra", idReperito, attivazione, pidLinkerString, NULL};
	   
                    if (vetPidWindow[numeroFinestre-1]==0)
	                {
	                   execvp(args[0], args);
	                }
	                
	                //devo passare info al tool esterno
	               char messaggio[100];
	   
	               sprintf(messaggio, "0X1X%sX%d|",idReperito, vetPidWindow[numeroFinestre-1]);
	   
	               if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	               {
		                printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		               getchar();
	               }
	            
	                control=1;
	            
			      }
		     }else if(device=='2')
		     {
				 //aggiorno il numero di frigoriferi come per lampadine 
				fflush(stdout);
				pidNuovoDispositivo=fork();
				
				if(pidNuovoDispositivo==-1)//gestione errore
	            {
					
		            control=0;//setto il controllo a zero
				
				}else
				{
				    numeroFrigoriferi=numeroFrigoriferi+1;
				    pid_t *vetPidFridge1=(pid_t *)malloc(numeroFrigoriferi*sizeof(pid_t));
				    for(int i=0; i<(numeroFrigoriferi-1); i++)
				    {
					   vetPidFridge1[i]=vetPidFridge[i];
				    }
				
				
				    free(vetPidFridge);
				
				    vetPidFridge=vetPidFridge1;
				
				
				    vetPidFridge[numeroFrigoriferi-1]=pidNuovoDispositivo;
	            
	           
	           
	            
	                //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	                char *args[]={"./frigorifero", idReperito, attivazione, pidLinkerString,NULL};
	   
                   if (vetPidFridge[numeroFrigoriferi-1]==0)
	               {
	                  execvp(args[0], args);
			       }
			       
			       //devo passare info al tool esterno
	               char messaggio[100];
	   
	               sprintf(messaggio, "0X2X%sX%d|",idReperito, vetPidFridge[numeroFrigoriferi-1]);
	   
	               if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	               {
		                printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		                getchar();
	               }
			   
			       control=1;
		         }
	               
		     }else if(device=='3')
		     {
				 //aggiorno il numero di hub come per lampadine
				fflush(stdout);
				pidNuovoDispositivo=fork();
				
				if(pidNuovoDispositivo==-1)//gestione errore
	            {
					
		            control=0;//setto il controllo a zero
				
				}else
				{
				
				    numeroHub=numeroHub+1;
				    pid_t *vetPidHub1=(pid_t *)malloc(numeroHub*sizeof(pid_t));
				    for(int i=0; i<(numeroHub-1); i++)
				    {
					    vetPidHub1[i]=vetPidHub[i];
				    }
				
				    free(vetPidHub);
				
				    vetPidHub=vetPidHub1;
				
				
				    
				    vetPidHub[numeroHub-1]=pidNuovoDispositivo;
	            
	           
	           
	            
	                //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	                char *args[]={"./lampadina", idReperito, bufferPidGestoreID, attivazione, pidLinkerString,NULL};
	   
                    if (vetPidHub[numeroHub-1]==0)
	                {
	                   execvp(args[0], args);
			        }
			        
			        //devo passare info al tool esterno
	               char messaggio[100];
	   
	               sprintf(messaggio, "0X6X%sX%d|",idReperito, vetPidHub[numeroHub-1]);
	   
	               if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	               {
		                printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		               getchar();
	               }
			  
			        control=1;
			   
		         }
				 
		     }else if(device=='4')
		     {
			  //aggiorno il numero di timer come per lampadine
				fflush(stdout);
				pidNuovoDispositivo=fork();
				
				if(pidNuovoDispositivo==-1)//gestione errore
	            {
					
		            control=0;//setto il controllo a zero
				}else
				{
				   numeroTimer=numeroTimer+1;
				   pid_t *vetPidTimer1=(pid_t *)malloc(numeroTimer*sizeof(pid_t));
				   for(int i=0; i<(numeroTimer-1); i++)
				   {
					   vetPidTimer1[i]=vetPidTimer[i];
				   }
				
				   free(vetPidTimer);
				
				   vetPidTimer=vetPidTimer1;
				
				
				  
				   vetPidTimer[numeroTimer-1]=pidNuovoDispositivo;
	            
	           
	            
	               //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	               char *args[]={"./timer", idReperito, bufferPidGestoreID, attivazione, pidLinkerString, NULL};
	   
                   if (vetPidTimer[numeroTimer-1]==0)
	               {
	                  execvp(args[0], args);
			       }
			       
			       //devo passare info al tool esterno
	               char messaggio[100];
	   
	               sprintf(messaggio, "0X5X%sX%d|",idReperito, vetPidTimer[numeroTimer-1]);
	   
	               if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	               {
		                printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		               getchar();
	               }
			    
			       control=1;
		       } 
			   
		     }else if(device=='5')
		     {
				 //aggiorno il numero di termostati come per lampadine
			    fflush(stdout);
				pidNuovoDispositivo=fork();
				
				if(pidNuovoDispositivo==-1)//gestione errore
	            {
					
		            control=0;//setto il controllo a zero
				
				}else
				{
				   numeroTermostati=numeroTermostati+1;
				   pid_t *vetPidTermostato1=(pid_t *)malloc(numeroTermostati*sizeof(pid_t));
				   for(int i=0; i<(numeroTermostati-1); i++)
				   {
					   vetPidTermostato1[i]=vetPidTermostato[i];
				   }
				
				   free(vetPidTermostato);
				
				   vetPidTermostato=vetPidTermostato1;
				
				
				   vetPidTermostato[numeroTermostati-1]=pidNuovoDispositivo;
	            
	           
	           
	            
	               //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	               char *args[]={"./termostato", idReperito, bufferPidGestoreID, attivazione, pidLinkerString, NULL};
	   
                   if (vetPidTermostato[numeroTermostati-1]==0)
	               {
	                  execvp(args[0], args);
			       }
			       
			       //devo passare info al tool esterno
	               char messaggio[100];
	   
	               sprintf(messaggio, "0X4X%sX%d|",idReperito, vetPidTermostato[numeroTermostati-1]);
	   
	               if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	               {
		                printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		               getchar();
	               }
			       
			       control=1;
		        }
		      
			 }else if(device=='6')
			 {
				
				 //aggiorno il numero di tende come per lampadine
				 fflush(stdout);
				pidNuovoDispositivo=fork();
				
				if(pidNuovoDispositivo==-1)//gestione errore
	            {
					
		            control=0;//setto il controllo a zero
				
				}else
				{
				   numeroTende=numeroTende+1;
				   pid_t *vetPidTenda1=(pid_t *)malloc(numeroTende*sizeof(pid_t));
				   for(int i=0; i<(numeroTende-1); i++)
			     	{
	    				vetPidTenda1[i]=vetPidTenda[i];
     				}
				
				    free(vetPidTenda);
				
				    vetPidTenda=vetPidTenda1;
				
				
				   
				    vetPidTenda[numeroTende-1]=pidNuovoDispositivo;
	           
	           
	           
	            
	                 //inserisco l'id reperito in un vettore (buffer) da passare come argomento al figlio tramite la funzione sprintf
	                char *args[]={"./tenda", idReperito, bufferPidGestoreID, attivazione, pidLinkerString, NULL};
	   
                    if (vetPidTenda[numeroTende-1]==0)
	                {
	                   execvp(args[0], args);
			        }
			        
			        //devo passare info al tool esterno
	               char messaggio[100];
	   
	               sprintf(messaggio, "0X3X%sX%d|",idReperito, vetPidTenda[numeroTende-1]);
	   
	               if(write(fdFifoIdEsterno, messaggio, strlen(messaggio))==-1)
	               {
		                printf("ERRORE SCRITTURA FIFO ID ESTERNO DA PARTE CENTRALINA POTREBBE ESSER IMPOSSIBILE COMUNICARE COL TOOL ESTERNO \n");
		               getchar();
	               }
			   
			        control=1;
			   
		         }
		     
			 } 
			
			 //verifico se inserimento è andato a buon fine e avviso la centralina
			  if(control==0)
			  {
				  kill(pidController, SIGUSR2);
			  }else
			  {
				  kill(pidController, SIGUSR1);
			  }
			 
			
			  //chiudo la fifo id
			  close(fdFifoID);
			
			
			
	    }else if(comand=='4'){
			
			
			sem_post(semaforoFifoComandi);
			
			 if(write(fdFifoComandi, communication, strlen(communication))==-1)
			 {
				  printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA \n");
				  return EXIT_FAILURE;
			  }
			  
			  
		   
					  
		  
					  					    
			  
			  
			   
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
					      
					     if(write(fdControllerComandi[1], informazioni, strlen(informazioni))==-1)
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
					      
					     if(write(fdControllerComandi[1], informazioni, strlen(informazioni))==-1)
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
					      
					     if(write(fdControllerComandi[1], informazioni, strlen(informazioni))==-1)
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
					      
					     if(write(fdControllerComandi[1], informazioni, strlen(informazioni))==-1)
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
					      
					     if(write(fdControllerComandi[1], informazioni, strlen(informazioni))==-1)
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
					      
					     if(write(fdControllerComandi[1], informazioni, strlen(informazioni))==-1)
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
					      
					     if(write(fdControllerComandi[1], informazioni, strlen(informazioni))==-1)
					     {
						   printf("ERRORE LETTURA PIPE LAMPADINE\n");
					     }
						  
					 }
			     
			     
			     }
				  
				  
				  
				   
			 
			   
			  
			
			 
			
			 //riristina checkValue
			checkValue=0;   
			//ripristina communication una volta terminato
			strcpy(communication, "XXXXXXXXX");
		
             
			
			//segnalo al controller che ho terminato la stampa dei dispositivi non connessi
			kill(pidController, SIGUSR1);
			
		}else if(comand=='3')
		{
			//l'implementazione è pressocché identica a quella della centralina
			
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
						   
						   
						   
						   if(write(fdControllerComandi[1], informazioni, strlen(informazioni)+1)==-1)
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
						   
						   
						   
						   if(write(fdControllerComandi[1], informazioni, strlen(informazioni)+1)==-1)
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
						   
						   
						   
						   if(write(fdControllerComandi[1], informazioni, strlen(informazioni)+1)==-1)
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
						   
						   
						   
						   if(write(fdControllerComandi[1], informazioni, strlen(informazioni)+1)==-1)
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
						   
						   
						   
						   if(write(fdControllerComandi[1], informazioni, strlen(informazioni)+1)==-1)
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
						   
						   
						   
						   if(write(fdControllerComandi[1], informazioni, strlen(informazioni)+1)==-1)
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
						   
						   if(read(fdLampadineComandi[indice], informazioni, 500)==-1)
						   {
							   printf("errore lettura pipe lampadina\n");
						   }
						   
						   printf("ho letto %s \n", informazioni);
						   
						   
						   
						   if(write(fdControllerComandi[1], informazioni, strlen(informazioni)+1)==-1)
						   {
							   printf("errore scrittura \n");
						   }
						   
						   
						   
					   }
					   
					   //ripristino pidSender
					   pidSender=0;
				   }
						   
					          
			                    
					   printf("scrivo sulla pipe comandi \n");
				   
				//riscrivo sulla piipe
				if(write(fdFifoComandi, communication, strlen(communication))==-1)
			    {
				    printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				    return EXIT_FAILURE;
			    }	   
			    
			    if(communication[0]=='0')
			    {
					kill(pidController,SIGUSR2);
				}else
				{
					kill(pidController,SIGUSR1);
				}
				
				checkValue=0;
			
			
		}else if(comand=='6')
		{
			
			
			printf("processo padre scrive sulla pipe %s \n", communication);
			
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
			printf("processo iniziale dopo post ha ottenuto questo valore semaforo %d", value);  
			
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
			       
			       printf("processo iniziale ha letto questo %s \n", communication);
			       
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
				   
				  printf("processo iniziale scrive sulla pipe %s \n", communication); 
				   
				//riscrivo sulla piipe
				             if(write(fdFifoComandi, communication, strlen(communication))==-1)
			                 {
				                printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				                return EXIT_FAILURE;
			                  }	   
			printf("processo iniziale notiffica la centralina \n");
			
			kill(pidController,SIGUSR1);
			
		}else if(comand=='2')
		{
			
		    //l'implementazione è pressocché identica a quella della centralina
			
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
						   
					          
			                    
					   
				   
				//riscrivo sulla piipe
				if(write(fdFifoComandi, communication, strlen(communication))==-1)
			    {
				    printf("ERRORE SCRITTURA SU PIPE DA PARTE DI CENTRALINA");
				    return EXIT_FAILURE;
			    }	   
			
			kill(pidController,SIGUSR1);
			
			
			
		}else if(comand=='5')
		{
			printf("processo iniziale si trova nel comando link \n");
		   
		   //imposto la verifica anchq eui
		   char communicationVerifica[]="XXX";
		   if(read(fdFifoComandi, communicationVerifica, strlen(communicationVerifica))==-1)
	        {
		       printf("ERRORE LETTURA PIPE \n");
		   
	        }
		   
		   printf("iniziale verifica letto %s \n", communicationVerifica);
			     
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
			     
			     printf(" processo iniziale scritto su pipe %s %s \n", communication, communicationVerifica);
			     
			     
			     sem_getvalue(semaforoFifoComandi, &value);
	    printf("valore prima del post in iniziale %d \n", value);
			     
			     //conesnto a tutti i dispositivi connesse di legger pipe
			      sem_post(semaforoFifoComandi);
			     
			     sem_getvalue(semaforoFifoComandi, &value);
	    printf("valore dopo del post in iniziale %d \n", value);
			     
			     fflush(stdout);
			     
			     //a differenza degli altri comandi qui facciamo una verifica solo alla fine
			     
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
			     
			     
			     printf("processo iniziale in wait notifica al controller \n");
			     printf("da notare che il numero lampadine attuale è %d \n", numeroLampadine);
			     //controllo che tutti i dispositivi connessi siano ritornati
			    fflush(stdout);
			    
		
			    checkValue=0;
			    //attesa ritorno di tutti
			     while(checkValue<num)
			     {
					 sigsuspend(&maschera);
				 }
				 
				 	    //qui notifico dopo alla centralina cosi manda i suoi segnali
			    kill(pidController, SIGUSR1);
				 
				 fflush(stdout);
				 printf("processo iniziale esce suspend per operare rimozione dopo aver notificato alla centralina/n");
				 
				 //qui non serve una verifica sulla correttezza dei comandi perche in caso non notificano
				 
					 
					
					 //a questo punto verifichiamo se eliminare
					 if((pidSender!=0))
					 {
						 
						 /*
						  * come nella centralina senza controlli
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
					 
					 
				 
				 
			     
			 
			  
			  
			  
			  //ripristina communication
			  strcpy(communication, "XXXXXXXXX");
			  
			  
		  
       }
		
		//chiudo la fifo comandi
	    //close(fdFifoComandi);
	}
	
	
	return 0;
}

void handlerLetturaFifo(int signum)
{
	/*
	 * NOTA
	 * anche qui non so cosa metterci
	 */ 
     	 
	 checkValue=checkValue+1;
	 
	 
	 printf("checkvalue vale ora %d \n", checkValue);
}
void handlerAck(int signum, siginfo_t *siginfo, void *context)
{
	 pidSender=siginfo->si_pid;
	 checkValue=checkValue+1;


     	 printf("checkvalue vale ora %d \n", checkValue);

}
