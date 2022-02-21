
/*
 * termostato
 * lo stato e l interruttore sono normali
 * contiene la temperatura
 * un sottoprocesso interviene in caso di temperatura inferiore al max per riportarlo a regime
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


//VARIABILI GLOBALI
//DI BASE
//variabile che rappresenta l interruttore nel caso della tenda rimane settato a on per un tot tempo e incrementa mano a mano
char interruttore[]="OFF";
//struttura definita in time.h che ci servirà a memorizzare i valori temporali reperiti dal sistema
struct tm tempoAccensione;/*tra l altro dovrebbe chiamarsi time ma non puo*/
//AUSILIARE
//variabile che indica se il dispositivo è connesso a qualcosa
int connected=0;
//pipe padre
int pipeMirroringPadreLettura;
int pipeMirroringPadreScrittura;
//regola se leggere o servire segnale cambio interruttore
int lettura=0;


int temp = 0;

int main(int argc, char **argv)
{
  //VARIABILI PRINCIPALI DEL DISPOSITIVO
	//id del dispositivo
	int id;
    //stato del dispositivo, inzialmente a 0
	int stato=0;	
	//temperatura registrata
	float tempReg=0;
	//temperatura a cui adeguarsi
	float tempImp=0;
	



	while(1)
	{
		if(comand=='3')//implementazione di info
	   {
		   
		  int idDaVerificare=atoi(id1);//converto l'id in interp
		  
		  if(idDaVerificare==id)//verifico se l 'id corrisponde a quello del dispositivo
		  {
			 strcpy(communication, "0XXXXXXXX");//avviso sulla pipe principale che l'id è stato trovato
			 
			 char informazioni[200];//vettore in cui inserisco  le informazioni da inviare nella pipe che comunica col padre
			  	  
			 char statoAusiliare[]="spento";//variabile che contiene il valore dello stato inzialmente settata a spento
			 if(stato==1)//reperisco lo stato attuale
			 {
				strcpy(statoAusiliare, "acceso");
			 }  
			  
			 //se lo stato è uno si calcola il tempo di accensione 
			 //ovviamente fornire questa informazione ha senso solo se lo stato è settato a 1
             if(stato==1)
             {
				   
	               //calcolo della data attuale
		           time_t data;
	               struct tm* tempoReperito;
	               time(&data);
	    
	               //il tempo reperito dal sistema viene inserito in una struttura tm dalla funzione localtime che a noi restituisce un puntatore ad essa relativa 
	               //teniamo traccia di questa struttura inizializzando il nostro puntatore al puntatore restituito
	               tempoReperito=localtime(&data);

                   //memorizzandolo in una variabile intera che poi stamperò a schermo calcolo la differenza tra i valori ottenuti e quelli memorizzati        
                   int anni=(tempoReperito->tm_year)-(tempoAccensione.tm_year);
        
                   int mesi=(tempoReperito->tm_mon)-(tempoAccensione.tm_mon);
        
                   int giorni=(tempoReperito->tm_mday)-(tempoAccensione.tm_mday);
        
                   int ore=(tempoReperito->tm_hour)-(tempoAccensione.tm_hour);
	 
	               int minuti=(tempoReperito->tm_min)-(tempoAccensione.tm_min);
	 
	               int secondi=(tempoReperito->tm_sec)-(tempoAccensione.tm_sec);
	               
	               //inserisco i valori reperiti nel vettore informazioni
                   sprintf(informazioni, "termostato %d %s \ntempo di accensione: anni %d mesi %d giorni %d ore %d minuti %d secondi %d \ntemperatura attuale %f temperatura impostata %f ", id, statoAusiliare, anni, mesi, giorni, ore, minuti, secondi, tempReg, tempImp);
					//mando in pipe
			       if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
			       {
				       printf("ERRORE SCRITTURA PIPE PADRE DA PARTE DI LAMPADINA. INSERISCI UN QUALSIASI CARATTERE PER TERMINARE \n");
				       getchar();
				       return EXIT_FAILURE;
			       }	  
	  
                 }else//se lo stato è 0 scrivo i valori reperiti prima
                 {
					 sprintf(informazioni, "termostato %d %s ", id, statoAusiliare);
			         if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
			         {
					   printf("ERRORE SCRITTURA PIPE PADRE DA PARTE DI LAMPADINA. INSERISCI UN QUALSIASI CARATTERE PER TERMINARE \n");
				       getchar();
				       return EXIT_FAILURE;            
			         }
		
				 }
			  
			  
		  }
		  
		  //ripeto il comando sulla fifo comando		  
		  if(write(fdFifoComandi, communication, strlen(communication))==-1)
	      {
			 printf("ERRORE SCRITTURA FIFO COMANDI DA PARTE DI LAMPADINA. INSERISCI UN QUALSIASI CARATTERE PER TERMINARE \n");
			  getchar();
		     return EXIT_FAILURE; 
	      }
	            
	      if(communication[0]=='0')//se il dispositivo individuato dall id è questo invio il segnale utente due al padre in alternativa il segnale uno
		  {
			 kill(getppid(),SIGUSR2);
			 
		  }else
		  {
			 kill(getppid(),SIGUSR1);
		  }
				
		   sem_post(semaforoFifoComandi);//posto il semaforo concedendo l accesso alla pipe comandi agli altri dispositivi
	      
				
		}
	}
}

/*non so che cazzo faccia è incomprensibile
void handlerImpostaTemp(int signum)
{
  int t;
  int fd [2], bytesRead;
  char message [MAXLEN];
  pipe (fd); 
  if (fork () > 0) { 
    close(fd[READ
    write (fd[WRITE], msg, strlen(msg)+1
    close (fd[WRITE]);
} else { 
    close (fd[WRITE]); 
    bytesRead = read (fd[READ], message, MAXLEN);
    t = atoi(message);
    close (fd[READ]); 
  }


  if(t == 0){
    printf("in temperatura\n");
  }else{
  if(t > temp){
    while(temp != t){
      sleep(1);
      temp++;
      printf("%d\n",temp);
    }
    printf("in temperatura\n");
  }else{
    while(temp != t){
      sleep(1);
      temp--;
      printf("%d\n",temp);
    }
    printf("in temperatura\n");
  }
}
* }
*/




