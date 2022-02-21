/*
 *window
 *  
 */


#include <stdio.h>

int main(int argc, char **argv)
{
	
	
	while(1)
	{
		if(comand=='3')//implementazione di info
	   {
		   
		  int idDaVerificare=atoi(id1);//converto l'id in interp
		  
		  if(idDaVerificare==id)//verifico se l 'id corrisponde a quello del dispositivo
		  {
			 strcpy(communication, "0XXXXXXXX");//avviso sulla pipe principale che l'id è stato trovato
			 
			 char informazioni[200];//vettore in cui inserisco  le informazioni da inviare nella pipe che comunica col padre
			  	  
			 char statoAusiliare[]="chiusa";//variabile che contiene il valore dello stato inzialmente settata a spento
			 if(stato==1)//reperisco lo stato attuale
			 {
				strcpy(statoAusiliare, "aperta");
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
                   sprintf(informazioni, "finestra %d %s \ntempo di apertura: anni %d mesi %d giorni %d ore %d minuti %d secondi %d ", id, statoAusiliare, anni, mesi, giorni, ore, minuti, secondi);
					//mando in pipe
			       if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
			       {
				       printf("ERRORE SCRITTURA PIPE PADRE DA PARTE DI LAMPADINA. INSERISCI UN QUALSIASI CARATTERE PER TERMINARE \n");
				       getchar();
				       return EXIT_FAILURE;
			       }	  
	  
                 }else//se lo stato è 0 scrivo i valori reperiti prima
                 {
					 sprintf(informazioni, "finestra %d %s ", id, statoAusiliare);
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
	
	return 0;
}

