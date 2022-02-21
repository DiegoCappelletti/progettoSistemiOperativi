/*
 * fridge
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
//handler
//handler per il segnale utente 2.  cambia il valore dell'interruttore .
void handlerCambiaInterruttore(int signum);
//gestore per il segnale utente 1 che permette il continuo dell esecuzione
void handlerLetturaPipe(int signum);

//VARIABILI GLOBALI
//DI BASE
//variabile che rappresenta l interruttore
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

#include <stdio.h>

int main(int argc, char **argv)
{
	
	//VARIABILI PRINCIPALI DEL DISPOSITIVO
	//id del dispositivo
	int id;
    //stato del dispositivo, inzialmente a 0
	int stato=0;	
	//percentuale riempimento
	float perc=0; //da 0 a 100
	//temperatura interna
	float temp=0;
	//tempo dopo cui si richiude in automatico
	int delay=0;
	
	//VARIABILI O PROCEDURE AUSILIARIE
	//pid del linker per la procedura di linking
    pid_t pidLinker=atoi(argv[3]);
    
	
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
		  
	
	
    
   
	//imposto gli handler dei segnali
	
	struct sigaction sa;
	sa.sa_handler=handlerCambiaInterruttore;
	sigaction(SIGINT, &sa, NULL);
	
	struct sigaction sa1;
	sa1.sa_handler=handlerLetturaPipe;
	sigaction(SIGUSR1, &sa1, NULL);
	
	//dichiarazione della maschera per l'attesa
    sigset_t maschera;
    //il set viene riempito con tutti i segnali di sistema
    sigfillset(&maschera);
    //rimuovo i segnali che mi interessa ricevere
    sigdelset(&maschera, SIGUSR1);
    sigdelset(&maschera, SIGUSR2);
    sigdelset(&maschera, SIGINT);
    sigdelset(&maschera, SIGTERM);
    sigdelset(&maschera, SIGKILL);
    
    
    //CICLI OPERATIVI
    
     while(1)
    {
		lettura=0;
				
		//attende un segnale
		sigsuspend(&maschera);
		
		
		if(lettura==1)//solo se lettura è settato si procede a una lettura della fifo comandi altrimenti è un segnale di cambio interruttore
		{
		
		    //apro la fifo dei comandi
    		fdFifoComandi=open(fifoComandi,O_RDWR);
	    	if (fdFifoComandi==-1)
		    {
			    printf("ERRORE APERTURA PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
			    getchar();
			    return EXIT_FAILURE;
		    }
		
		    //attendo al semaforo
    		sem_wait(semaforoFifoComandi);
		
		    //leggo la pipe
	    	if(read(fdFifoComandi, communication, strlen(communication))==-1)
		    {
			    printf("ERRORE LETTURA PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
			    getchar();
			    return EXIT_FAILURE;
		    }
		
		
    		//otteniamo il comando
	    	comand=communication[0];
		    //otteniamo la posizione del interruttore se presente
		    pos=communication[8];
		
		    //ottengo il primo id in formato numerico
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
	       
	       //scompongo l'id2 come in precedenza	
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
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	    if(comand=='6')//implementazione di switch
		{
		  
          //ottengo la label e pos		  
		  char label=communication[4];
		  char pos=communication[8];
		  
		  //identifichiamo se l'id specificato è quello del dispositivo
	      int idDaVerificare=atoi(id1);
		  //controllo
		  if(idDaVerificare==id)
		  {
			 strcpy(communication, "0XXXXXXXX");//se l'esito è positivo sostituiamo a communication un messaggio di ack
			 
			 if(label=='0')//se la label è 0 alloa è corretta
			 {
				 //sulla base del valore di pos e del valore dell interruttore attuale potrebbe aver senso cambiare l'interruttore
			 
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
			  
		    }else
		    {
				
				strcpy(communication, "01XXXXXXX");
				
			}
			  
		  } 
		  
		        if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI FRIGORIFERO %d \n", id);
				  getchar();
				  return EXIT_FAILURE;
	            }
	            
				sem_post(semaforoFifoComandi);
				kill(getppid(), SIGUSR1);
				
		  
		  
		}else if(comand=='2')//comando del
		{
			
			//verifico se l'id specificato nel comando è quello del dispositivo
			
			//innanzitutto converto l id
			int idDaVerificare=atoi(id1);
			
			//verifico
			if(id==idDaVerificare)
			{
				strcpy(communication, "0XXXXXXXX");//converto communication in una stringa di feedback positivo
				if(write(fdFifoComandi, communication, strlen(communication))==-1) //in seguito funge da ripetitore
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI FRIGORIFERO %d \n", id);
				  getchar();
				  return EXIT_FAILURE;
	            }
	            close(fdFifoComandi);
	            sem_post(semaforoFifoComandi);
				kill(getppid(), SIGUSR2);//avvisa con il segnale di tipo due il padre cosi questi saprà che dispositivo eliminare
				
			}else
			{
				//in alternativa la lampadina funge solo da ripetitore del messaggio
				
				if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA %d \n", id);
				  getchar();
				  return EXIT_FAILURE;
	            }
	            
				kill(getppid(), SIGUSR1);
				sem_post(semaforoFifoComandi);
				
	   }
	   
	   
	   }else if(comand=='3')//implementazione di info
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
                   sprintf(informazioni, "frigorifero %d %s \ntempo di accensione: anni %d mesi %d giorni %d ore %d minuti %d secondi %d \npercentuale riempimento %f \ntemperatura interna %f \ntempo di richiusura %d ", id, statoAusiliare, anni, mesi, giorni, ore, minuti, secondi, perc, temp, delay);
					//mando in pipe
			       if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
			       {
				       printf("ERRORE SCRITTURA PIPE PADRE DA PARTE DI LAMPADINA. INSERISCI UN QUALSIASI CARATTERE PER TERMINARE \n");
				       getchar();
				       return EXIT_FAILURE;
			       }	  
	  
                 }else//se lo stato è 0 scrivo i valori reperiti prima
                 {
					 sprintf(informazioni, "frigorifero %d %s ", id, statoAusiliare);
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
	      
				
		}else if(comand=='0')//c'è gia stato un feedback positivo
		{
			
			//il dispositivo propaga unicamente il messaggio
			
			if(write(fdFifoComandi, communication, strlen(communication))==-1)
	        {
			    printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI FRIGORIFERO %d \n", id);
			    getchar();
			    return EXIT_FAILURE;
	         }
	            
		    sem_post(semaforoFifoComandi);
		    kill(getppid(), SIGUSR1);
				
			
		}
	
	
	
	
	
	    close(fdFifoComandi);//chiudo la fifo comandi		
        lettura=0; //ripristino lettura
        strcpy(communication, "XXXXXXXXX");//ripristino communication
	
	
        }
    
      
      
      /*
       * manca la parte della gestione interazione
       * 
       */    
    
        
        
    }
    

        
	return 0;
}


//IMPLEMENTAZIONE FUNZIONI
//implementazione dell handler per il cambio del valore dell interruttore.
void handlerCambiaInterruttore(int signum)
{
	//semplicemente scambia il valore interruttore
	
	if(strcmp(interruttore, "OFF")==0)
	{
		strcpy(interruttore, "ON");

	}
	else
	{
		strcpy(interruttore, "OFF");
		
	 }
	 
	 
}

//implementazione dell handler per il segnale utente 1.
void handlerLetturaPipe(int signum)
{
	//imposta il flag della lettura
	lettura=1;
}

