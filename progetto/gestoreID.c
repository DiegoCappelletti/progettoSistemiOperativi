/*
 * generatore di id
 * quando è richiamato genera un id e lo inserisce su una pipe. dopodiche incrementa un semaforo per l'accesso alla pipe consentendo 
 * al richiedente di leggerla. 
 * 
 * NOTA 
 * è il suo unico scopo fin ora non so se ce ne possono esser altri
 * non so i privilegi accesso ai semafori o file
 */

//LIBRERIE
//funzioni di libreria
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


//DICHIARAZIONE FUNZIONI
//handler che gestisce la richiesta di un id
void handlerRichiestaID(int signum);

int main(int argc, char **argv)
{

    //l'id di partenza per il gestore id. 0 è l'id assegnato alla centralina in automatico quindi parte da 1
    int id=1;
        
	
	//gestione della fifo degli id
    int fdFifoID; //variabile contente fd fifo
    char* percorsoFifoID="./fifoID";
    //creo la fifo con privilegi di lettura e scrittura
    mkfifo(percorsoFifoID, 0666);
    //apro la fifo
	fdFifoID=open(percorsoFifoID,O_RDWR);
	//verifico corretta apertura fifo
	if(fdFifoID==-1)
	{
		clear();
		printf("ERRORE APERTURA FIFO ID DA PARTE GESTORE ID \n");
		getchar();
		return EXIT_FAILURE;
	}
	
	//dichiarazione della maschera
    sigset_t maschera;
    //il set viene riempito con tutti i segnali di sistema
    sigfillset(&maschera);
    // con questa funzione tolgo il segnale SIGUSR1 che gestisce scrittura sulla pipe id
    sigdelset(&maschera, SIGUSR1);
    // con questa funzione tolgo il segnale SIGKILL
    sigdelset(&maschera, SIGKILL);
    // con questa funzione tolgo il segnale SIGTERM
    sigdelset(&maschera, SIGTERM);
    
    //imposto l'handler di richiesta id	
	signal(SIGUSR1, handlerRichiestaID);
	
	//genero il named semaphore usato per regolare gli accessi alla pipe o lo apro se gia creato
	sem_t* semaforoPipeID;
	semaforoPipeID=sem_open("IdSm", O_CREAT, 0666, 0);
	
	
	//ciclo di attesa 
	while(1)
	{
		//sospendi fino all arivo del segnale
	    sigsuspend(&maschera);
	    
	    //trasformo il valore intero id in una stringa da inserire nella pipe
	    char idValue[]="XXX";
     	sprintf(idValue, "%d",id );
	
	    //scrivo sulla pipe, in caso di errore termino con un messaggio appropriato
		if(write(fdFifoID, idValue, strlen(idValue))==-1)
	    {
		   clear();
		   printf("ERRORE SCRITTURA FIFO ID DA PARTE GESTORE ID \n");
		   getchar();
		   return EXIT_FAILURE;
	    }
	 
	    //incremento l'id progressivo
	    id=id+1;
	    
	    //segnalo al semaforo di modo che chiunque abbia richiesto all'id possa leggere
	    sem_post(semaforoPipeID);
    }
	
	return 0;
}

//IMPLEMENTAZIONE FUNZIONI
//implementazione handler segnale 1
void handlerRichiestaID(int signum)
{
	
	/*NOTA
	 * per ora non fa nulla non so in seguito
	 * 
	 */
}
