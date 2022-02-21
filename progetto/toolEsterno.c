/*
 *tool esterno 
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
	printf("sono un tool esterno \n");
	
		//apro o creo il semaforo per lettura stdin
	sem_t* semaforoEsterno=sem_open("EsSm", O_CREAT, 0666, 0);
	if(semaforoEsterno==-1)
	{
		printf("ERRORE APERTURA SEMAFORO \n");
	}
	
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


    //handler
        
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_sigaction=&handlerEsterno;
	act.sa_flags=SA_RESTART;
	sigaction(SIGUSR1, &act, NULL);

    while(1)
    {
		
		sigsuspend(&maschera);
		
	    sem_wait(semaforoEsterno);
	
		
		printf("tool esterno disponibile a una lettura comando la centralina risulta disabilitata temporaneamente dall'inserimento dei comandi\n");
		
        
        scanf("%[^\n]s",cmd);
        
        
        printf("esterno legge COMANDO %s \n", cmd);
        
        
       //separo il comando nei suoi costituenti con la funzione separa
       separa(cmd, comando, parametro1, parametro2, parametro3);
       
       getchar();
       
       
       sem_post(semaforoEsterno);
        
	}


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


void handlerEsterno(int signum)
{
}
