/*
 * gestore del link
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


//FUNZIONI
//gestore per il segnale di richiesta link
void handlerRichiestaLink(int signum, siginfo_t *siginfo, void *context);
//gestore per un ack generico
void handlerAck(int signum);

//VARIABILI GLOBALI
pid_t pidSender;
int fdFifoLink;//quando apri la fifo la metti qui
char role[]="0";

pid_t pidSource;
	pid_t pidDestination;

int main(int argc, char **argv)
{
	
	//variabili
	
	printf("ciao sono gestore link \n");
	
	printf("pid gestore link %d", getpid());
	
	
	
	//creazione fifo ausiliaria
	//in questa fifo i processi si scambiano le informazioni necessarie al link
	char* fifoLink="./fifoLink";
	
	if(mkfifo(fifoLink, 0666)==-1)
	{
		printf("ERRORE CREAZIONE FIFO LINK DA PARTE DI GESTORE LINK \n");
	}
	fdFifoLink=open(fifoLink,O_RDWR);
	if(fdFifoLink==-1)
		{
			printf("ERRORE APERTURA FIFO IN GESTORE LINK \n");
			getchar();
			return EXIT_FAILURE;
		}
		
		
	//genero il named semaphore usato per regolare gli accessi alla pipe o lo apro se gia creato
	sem_t* semaforoLink;
	semaforoLink=sem_open("LkSm", O_CREAT, 0666, 0);	
		
	//dichiarazione della maschera
    sigset_t maschera;
    //il set viene riempito con tutti i segnali di sistema
    sigfillset(&maschera);
    // con questa funzione tolgo il segnale SIGUSR1 che gestisce scrittura sulla pipe id
    sigdelset(&maschera, SIGUSR1);
        // con questa funzione tolgo il segnale SIGUSR1 che gestisce scrittura sulla pipe id
    sigdelset(&maschera, SIGUSR2);
    // con questa funzione tolgo il segnale SIGKILL
    sigdelset(&maschera, SIGKILL);
    // con questa funzione tolgo il segnale SIGTERM
    sigdelset(&maschera, SIGTERM);
    
    //imposto l'handler di richiesta id	
	signal(SIGUSR1, handlerAck);
	
	   //handler per la gestione del delete
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_sigaction=&handlerRichiestaLink;
	act.sa_flags=SA_SIGINFO;
	sigaction(SIGUSR2, &act, NULL);	
	
	//incremento il semaforo di base
		sem_post(semaforoLink);	
		
		int value;
		sem_getvalue(semaforoLink, &value);
		printf("value del semaforo dopo post linker a inizio %d \n", value);
		
	printf("linker entra in operativo \n");	
	fflush(stdout);
		
	while(1)
	{
		
		
		if((pidSource==0)||(pidDestination==0)) //se uno dei due non è inizializzato non ha senso continuare attendi un nuovo pid
		{
			printf("il linker entra suspend \n");
		    fflush(stdout);
		    
		    sigsuspend(&maschera);
		}else
		{
			//cicli operativi
			printf("linker entro nel vero codice \n");
			fflush(stdout);
			
			printf("avviso il sender \n");
			//avviso il sender che puo inviare i dati
			kill(pidSource, SIGUSR1);
			
			//consento la scrittura sulla pipe
			sem_post(semaforoLink);	
			
			printf("linker attende una risposta dal sender \n");
			sigsuspend(&maschera);
			
			printf("linker avvisa receiver \n");
			
			//avviso il receiver che puo ricevere i dati
			kill(pidDestination, SIGUSR1);
			
			//consento la lettura sulla pipe
			sem_post(semaforoLink);	
			
			/*
			 * a questo punto la logica sarebbe questa
			 * fatto prima
			 * si identifica sender e receiver
			 * il sender scrive i dati del figlio
			 * il receiver legge il tipo alloca un figlio
			 * il figlio se è un dispositivo legge semplicemente i dati e ritorna un ack al padre
			 * se è di controllo se ne accorge e manda il suo pid
			 * anche l'laltro sa se è di controllo e manda un segnale al figlio che avvia una procedura di scrittura in pipe 
			 * 
			 * a questo punto il dispositivo di contollo riceve le info crea il figlio che se è una foglia termina e manda ack al padre
			 * il padre manda il suo ack di ricevimento pronto a ricevere se non ce piu nulla da ricevere viene mandato in pipe un valore zero e il dispositivo di controllo notifica al receiver
			 * 
			 */ 
			 
			//attendo fine lettura del receiver altrimneti potrebbero esserci problemi
			sigsuspend(&maschera); 
			
			//al termine gli reinizializzo
			pidSource=0;
			pidDestination=0;
		}
		
		//incremento il semaforo alla fine
		sem_post(semaforoLink);
		
		
		sem_getvalue(semaforoLink, &value);
		printf("value del semaforo dopo post linker %d \n", value);
		
	}	
	
	return 0;
}

void handlerAck(int signum)
{
}

void handlerRichiestaLink(int signum, siginfo_t *siginfo, void *context)
{
	
	printf("linker ho ricevuto una richiesta \n");
	 fflush(stdout);
	 
	 pidSender=siginfo->si_pid;
   
     if(read(fdFifoLink, role, strlen(role))==-1)
     {
			printf("ERRORE LETTURA PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
			getchar();
			return EXIT_FAILURE;
	}
	
	if(strcmp(role, "1")==0)
	{
		printf("ho ottenuto il sender \n");
		fflush(stdout);
		pidSource=pidSender;
	}else if(strcmp(role, "2")==0)
	{
		printf("ho ottenuto il receiver \n");
		fflush(stdout);
		pidDestination=pidSender;
	}else //altrimenti c'e un errore e si chiude
	{
		printf("linker notifica errore a sender \n");
		fflush(stdout);
		
		if(pidSource!=0)//questo ha senso se l'altro ha terminato di inviare
		{
			
			
			kill(pidSender, SIGUSR2);
		}
		
		pidSender=0;
		pidSource=0;
		pidDestination=0;
		
	}
 
 strcpy(role, "0");

}

