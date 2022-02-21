/*
 * lampadina
 * al suo interno vi è un gestore che gestisce un segnale esterno che cambia il valore dell'interruttore
 * attende di legger sulla pipe dei comandi all'arrivo di un segnale
 * all'arrivo di ogni segnale allinea lo stato al valore dell'interruttore inoltre all accensione immagazzina il 
 * momento di accensione
 * 
 * NOTE
 * devo ancora aggiungere la gestione dei comandi
 * non sono sicuro sulla gestione dell'interruttore
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
//handler per il segnale utente 2.  cambia il valore dell'interruttore .
void handlerCambiaInterruttore(int signum);
//gestore per il segnale utente 1 che permette il continuo dell esecuzione
void handlerLetturaPipe(int signum);

//VARIABILI GLOBALI
//interruttore
//fd della pipe interruttore
int fdPadreInterruttore;
//variabile che rappresenta un interruttore
char interruttore[]="OFF";
//per il funzionamento specifico
//struttura definita in time.h che ci servirà a memorizzare i valori temporali reperiti dal sistema
struct tm tempoAccensione;
//per i comandi centralina
//variabile che indica se il dispositivo è connesso a qualcosa
int connected=0;
//pipe padre
int pipePadre;
//questo forse eliminabile
char idString[]="XXX";
//regola se leggere o servire segnale cambio interruttore
int lettura=0;
//per comunicazione con tool esterno
int flagEsterno=0;



int main(int argc, char **argv)
{
	
	//pipe comandi
	int pipeComandi=atoi(argv[4]);
	
    //pid del linker
    pid_t pidLinker=atoi(argv[3]);
    
	
	
	//id del dispositivo
	int id;
	
//stato del dispositivo, inzialmente a 0
	int stato=0;	
	

	
	if(strcmp(argv[2], "standard")==0)
	{//reperimento dell'id dagli argomenti del programma
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
	 
    
    }else if (strcmp(argv[2], "added")==0)
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
	 
	 //comunica l'aggiunta della lampadina al sistema
	printf("added bulb id %d \n", id);
		
	}else if(strcmp(argv[2], "linking")==0)
	{
		//il dispositivo deve esser connesso
		connected=1;
		
		//processo di lettura
		
		printf("sono una lampadina creata col linking \n");
		fflush(stdout);
		
		//scrive il suo id
		        char* fifoLink="./fifoLink";
	            int fdFifoLink=open(fifoLink,O_RDWR);
		        if(fdFifoLink==-1)
		        {
					printf("ERRORE APERTURA FIFO \n");
					return EXIT_FAILURE;
				}
		        
		        printf("fin qui va bene \n");
		        fflush(stdout);
		        
		        
	            if(read(fdFifoLink, idString, strlen(idString))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            printf("idString letta %s \n", idString);
	            fflush(stdout);
	            
	            int counter=3;//parto da tre (solo valori numerici passati a riga di comando)
	for(int i=0; i<3; i++)
	{
		  if(idString[i]=='X')
		  {
			  counter=counter-1;//se individuo una X decremento di uno il contatore
		  }
	 }
	 char idStringa[counter];//genero una stringa con lunghezza pari al valore numerico pasato
	 for(int i=0; i<(3-counter); i++)
	 {
		 idStringa[i]=idString[i];//inizializzo la stringa suddetta con i soli valori numerici trovati nella riga di comando
	 }
	 id=atoi(idStringa);//inserisco in id la conversione in intero del vettore di sopra
	            
	           
	            
	            printf("il mio id è %d \n", id);
	            fflush(stdout);
	            
	            //scrive il suo interruttore
	            if(read(fdFifoLink, interruttore, strlen(interruttore))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            printf("ho letto l'interrutore %s \n", interruttore);
	            fflush(stdout);
	            
	            char statoString[]="0";
	          
	           
	            if(read(fdFifoLink, statoString, strlen(statoString))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            stato=atoi(statoString);
	           
	           printf("il mio stato è %d \n", stato);
	            fflush(stdout);
	           
	            
	            if(stato==1)
	            {
					printf("devo copiarmi tutta la parte dello stato \n");
					
					char dayBuffer[]="XX";

	               if(read(fdFifoLink, dayBuffer, strlen(dayBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
	            
	            tempoAccensione.tm_mday=atoi(dayBuffer);
					
			        char yearBuffer[]="XXXX";
			        
			        if(read(fdFifoLink, yearBuffer, strlen(yearBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
	            
	            tempoAccensione.tm_year=atoi(yearBuffer);
			        
			        char hourBuffer[]="XX";

			        if(read(fdFifoLink, hourBuffer, strlen(hourBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
			        
			        tempoAccensione.tm_hour=atoi(hourBuffer);
			        
			        char minBuffer[]="XX";
			        
			        if(read(fdFifoLink, minBuffer, strlen(minBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
	            
	            tempoAccensione.tm_min=atoi(minBuffer);
	      
	                char secBuffer[]="XX";
			        
			        
			        if(read(fdFifoLink, secBuffer, strlen(secBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
	            
	            tempoAccensione.tm_sec=atoi(secBuffer);
	            
	            printf("esempio tempo accensione %d \n", tempoAccensione.tm_hour);
	            printf("esempio tempo accensione %d \n", tempoAccensione.tm_sec);
	            
				}
		
		//chiudo la fifo
		close(fdFifoLink);
		
		//avviso linker che è terminata lettura
		//kill(pidLinker, SIGUSR1);
		printf("avviso il padre che ho concluso \n");
		
		//avviso il padre che ho concluso
		kill(getppid(), SIGUSR1);
		
		//qui non serve gestire i sottoelementi
		
		/*
		 * qui serve il codice del mirroring
		 * -apertura pipe
		 * -scrittura valore
		 * -invio segnale al padre
		 * 
		 */ 
		
		pipePadre=atoi(argv[4]);
		
		
		
		sprintf(statoString, "%d", stato);
		
		write(pipePadre, statoString, strlen(statoString));
		
		write(pipePadre, idString, strlen(idString));
		
		kill(getppid(), SIGIO);
		
	}else if(strcmp(argv[2],"test")==0)
	{
		
		connected=1;
		
		stato=1;
		strcpy(interruttore, "ON");
		
		printf("lampadina entra in test \n");
		printf("in argv1 c'e %s \n", argv[1]);
		id=atoi(argv[1]);
		printf("sono una lampadina %d \n", id);
		
		/*
		 * qui serve codice di mirroring di sopra
		 * 
		 */ 
		
		pipePadre=atoi(argv[4]);
		
		char statoString[]="0";
		
		sprintf(statoString, "%d", stato);
		
		write(pipePadre, statoString, strlen(statoString));
		
		write(pipePadre, argv[1], strlen(argv[1]));
		
		//kill(getppid(), SIGIO);
		
	}else if(strcmp(argv[2],"test1")==0)
	{
		
		printf("lampadina test 1 avviata \n");
		
			//pipe comandi
	pipeComandi=atoi(argv[5]);
	
	//pipe interruttore
	fdPadreInterruttore=atoi(argv[6]);
	
	printf("LA MIA PIPE E' %d \n", pipeComandi);
	
		connected=1;
		
		stato=0;
		strcpy(interruttore, "OFF");
		
		printf("lampadina entra in test \n");
		printf("in argv1 c'e %s \n", argv[1]);
		id=atoi(argv[1]);
		printf("sono una lampadina %d \n", id);
		
		
		
		pipePadre=atoi(argv[4]);
		
		
		      //stringa in cui inserisco lo stato del dispositivo da mandare in pipe
		      char messaggio[100];
		      sprintf(messaggio, "%dX%d", stato, id);
		      //scrivo lo stato del dispositivo sulla pipe
		      write(pipePadre, messaggio, strlen(messaggio)+1);
		      //invio il segnale al padre
		      kill(getppid(), SIGIO);
	      
		
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
    
    
    /*
     * per il testing
     * 
     */ 
     
     if(id==1)
     {
		 stato=1;
		 
		 //calcolo della data attuale
		  time_t data;
	      struct tm* tempoReperito;
	      time(&data);
	    
	      //il tempo reperito dal sistema viene inserito in una struttura tm dalla funzione localtime che a noi restituisce un puntatore ad essa relativa 
	      //teniamo traccia di questa struttura inizializzando il nostro puntatore al puntatore restituito
	      tempoReperito=localtime(&data);
	    
	      //inseriamo nella nostra struttura tm i valori reperiti
	      tempoAccensione.tm_mday=tempoReperito->tm_mday;
	      tempoAccensione.tm_mon=tempoReperito->tm_mon;
	      tempoAccensione.tm_year=tempoReperito->tm_year;
	      tempoAccensione.tm_hour=tempoReperito->tm_hour;
	      tempoAccensione.tm_min=tempoReperito->tm_min;
	      tempoAccensione.tm_sec=tempoReperito->tm_sec;
	 }
    
    while(1)
    {
		lettura=0;
			
		printf("la lampadina %d entra in suspend \n", id);	
				
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
		
		if(comand=='4')//implementazione di list
		{
			//come con info otteniamo una stringa informazione costituita da sottostringhe che contengono il nome e l'id e lo stato
			
			char nome[]="bulb";			
			char statoS[]="spento";  
			if(stato==1)
			{
		        strcpy(statoS, "acceso");
			}
			
			char informazioni[100]; 
		 
		    sprintf(informazioni, "%s %d %s \n", nome, id, statoS);
		    
		    if(write(pipeComandi, informazioni, strlen(informazioni))==-1)
	        {
				printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA %d \n", id);
				getchar();
				return EXIT_FAILURE;
	        }
		    
		    if(write(fdFifoComandi, communication, strlen(communication))==-1)
	        {
				  printf("ERRORE SCRITTURA SU FIFO COMUNICAZIONE DA PARTE DI LAMPADINA %d \n", id);
				  getchar();
				  return EXIT_FAILURE;
	        }
	            
			kill(getppid(), SIGUSR1);
			sem_post(semaforoFifoComandi);
			
		}else if(comand=='3')//implementazione di info
		{
		   
		  int idDaVerificare=atoi(id1);//converto l'id in interp
		  
		  if(idDaVerificare==id)//verifico se l 'id corrisponde a quello del dispositivo
		  {
			 strcpy(communication, "0XXXXXXXX");//avviso sulla pipe principale che l'id è stato trovato
			 
			 char informazioni[100];//vettore in cui inserisco  le informazioni da inviare nella pipe che comunica col padre
			  	  
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
                   sprintf(informazioni, "lampadina %d %s tempo di accensione: anni %d mesi %d giorni %d ore %d minuti %d secondi %d ", id, statoAusiliare, anni, mesi, giorni, ore, minuti, secondi);
					//mando in pipe
			       if(write(pipeComandi, informazioni, strlen(informazioni)+1)==-1)
			       {
				       printf("ERRORE SCRITTURA PIPE PADRE DA PARTE DI LAMPADINA. INSERISCI UN QUALSIASI CARATTERE PER TERMINARE \n");
				       getchar();
				       return EXIT_FAILURE;
			       }	  
	  
                 }else//se lo stato è 0 scrivo i valori reperiti prima
                 {
					 sprintf(informazioni, "lampadina %d %s ", id, statoAusiliare);
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
				
		}else if(comand=='6')//implementazione di switch
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
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA %d \n", id);
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
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA %d \n", id);
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
		}else if(comand=='5')
		{
			printf("lampadina %d si trova in comando link \n", id);
		    fflush(stdout);
		
		    char communicationVerifica[]="XXX";
				
		    if(read(fdFifoComandi, communicationVerifica, strlen(communicationVerifica))==-1)
		    {
			   printf("ERRORE LETTURA PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
			    getchar();
			   return EXIT_FAILURE;
		    }
			
			printf("lampadina %d ha letto communication %s communicationVerifica %s  \n", id, communication, communicationVerifica);
			 fflush(stdout);
			 
			if (id==atoi(id2))
			{
				communicationVerifica[1]='0';
				communicationVerifica[2]='1';
				printf("lampadina %d ha riscontrato errore \n", id);
				
				if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            if(write(fdFifoComandi, communicationVerifica, strlen(communicationVerifica))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            sem_post(semaforoFifoComandi);
	            
	            //avviso il linker se serve
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
	            
	            //avviso il padre
	            kill(getppid(), SIGUSR1);
				
				fflush(stdout);
				
			}else if(id==atoi(id1))
			{
				
				//cambio codice
				communicationVerifica[0]='5';
				
				printf("communicationVerifica è ora %s \n", communicationVerifica );
				 fflush(stdout);
				 
					if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            if(write(fdFifoComandi, communicationVerifica, strlen(communicationVerifica))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            int value;
	            sem_post(semaforoFifoComandi);
	            sem_getvalue(semaforoFifoComandi, &value);
	          printf("lampadina ha scritto %s %s con semaforo %d \n", communication, communicationVerifica, value);
	             fflush(stdout);
				
				//alla fine avviso il padre
				kill(getppid(), SIGUSR2);
				printf("lampadina avvisa processo iniziale \n");	
				
				
				printf("lampadina %d ha identificato il proprio  id \n", id);
				 fflush(stdout);
				
					/*qui fa tutte le sue operazioni*/
					
				
				char* fifoLink="./fifoLink";
	            int fdFifoLink=open(fifoLink,O_RDWR);
	             if(fdFifoLink==-1)
	            {
					printf("ERRORE APERTURA FIFO LINK DA PARTE LAMPADINA \n");
					getchar();
					return EXIT_FAILURE;
				}
	            
	            
	            sem_t* semaforoLink;
	            semaforoLink=sem_open("LkSm", O_CREAT, 0666, 0);
	             if(semaforoLink==-1)
	            {
					printf("ERRORE APERTURA SEMAFORO LINK DA PARTE LAMPADINA \n");
					getchar();
					return EXIT_FAILURE;
				}	
	            
	            printf("lampadina a wait del semaforo linker \n ");
	       fflush(stdout);      
	            
	            sem_wait(semaforoLink);
	            
	            printf("lampadina esce wait del semaforo linker \n ");
	             fflush(stdout);
	            
	            if(write(fdFifoLink, "1", strlen("1"))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            printf("scritto %d tipo dispositivo al gestore link \n ", id);
	             fflush(stdout);
	            printf("voglio inviare al linker a questo pid %d", pidLinker);
	             fflush(stdout);
	             
	            kill(pidLinker, SIGUSR2);//invio il segnale al linker	
				
				printf("lampadina %d entra suspend \n", id);
				 fflush(stdout);
				
				sigsuspend(&maschera);	//aspetta feedback
				
				printf("lampadina esce suspend \n");
				 fflush(stdout);
				 
				sem_wait(semaforoLink);	
				
				printf("lampadina esce wait \n");
				 fflush(stdout);
				
				//scrive la tipologia, 0 lampadina etc
				if(write(fdFifoLink, "0", strlen("0"))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }	
	            
	            //scrive il suo id
	            if(write(fdFifoLink, argv[1], strlen(argv[1]))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            //scrive il suo interruttore
	            if(write(fdFifoLink, interruttore, strlen(interruttore))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            char statoBuffer[]="0";
	            sprintf(statoBuffer, "%d", stato);
	            //scrive il suo stato
	            if(write(fdFifoLink, statoBuffer, strlen(statoBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            if (stato==1) //se lo stato è 1 deve scrivere le info orarie
	            {
					printf("LAMPADINA DEVE SCRIVERE STATO \n");
					
					char dayBuffer[]="XX";
	               sprintf(dayBuffer, "%d", tempoAccensione.tm_mday);
	               
	               if(write(fdFifoLink, dayBuffer, strlen(dayBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
					
			        char yearBuffer[]="XXXX";
			        sprintf(yearBuffer, "%d", tempoAccensione.tm_year);
			        
			        if(write(fdFifoLink, yearBuffer, strlen(yearBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
			        
			        char hourBuffer[]="XX";
			        sprintf(hourBuffer, "%d", tempoAccensione.tm_hour);
			        if(write(fdFifoLink, hourBuffer, strlen(hourBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
			        
			        
			        char minBuffer[]="XX";
			        sprintf(minBuffer, "%d", tempoAccensione.tm_min);
			        if(write(fdFifoLink, minBuffer, strlen(minBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
	      
	                char secBuffer[]="XX";
			        sprintf(secBuffer, "%d", tempoAccensione.tm_sec);
			        
			        if(write(fdFifoLink, secBuffer, strlen(secBuffer))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA");
				  return EXIT_FAILURE;
	            }
	      
	      
					
				}
				
				/*
				 * non essendo un dispositivo controllo posso terminare subito
				 * 
				 */ 
					
				//alla fine avviso il linker
				kill(pidLinker, SIGUSR1);
				printf("lampadina ho avvisanto il linker con pidLiner %d \n", pidLinker);	
				fflush(stdout);	
					
								
				
		
				
			}else
			{
				if(write(fdFifoComandi, communication, strlen(communication))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            if(write(fdFifoComandi, communicationVerifica, strlen(communicationVerifica))==-1)
	            {
				  printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA \n");
				  return EXIT_FAILURE;
	            }
	            
	            printf("lampadina %d non ha identificato id ha scritto %s %s \n", id, communication, communicationVerifica);
	            
	            fflush(stdout);
	            
	            int value;
	            sem_getvalue(semaforoFifoComandi, &value);
	            printf("lampadina %d valore prima post %d \n", id, value);
	            
	            sem_post(semaforoFifoComandi);
	            
	            sem_getvalue(semaforoFifoComandi, &value);
	            printf("lampadina %d valore prima post %d \n", id, value);
	            
	            kill(getppid(), SIGUSR1);
	            
	            fflush(stdout);
			}
			
		}else if(comand=='0')//c'è gia stato un feedback positivo
		{
			
			//il dispositivo propaga unicamente il messaggio
			
			if(write(fdFifoComandi, communication, strlen(communication))==-1)
	        {
			    printf("ERRORE SCRITTURA SU PIPE COMUNICAZIONE DA PARTE DI LAMPADINA %d \n", id);
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
	
	
	
    //se un segnale esterno cambia l'interruttore lo stato della lampadina si adegua
    if((stato==0) && (strcmp(interruttore, "ON")==0))
	{
		
			
	      //cambia il valore di stato
		  stato=1;
		
		  //calcolo della data attuale
		  time_t data;
	      struct tm* tempoReperito;
	      time(&data);
	    
	      //il tempo reperito dal sistema viene inserito in una struttura tm dalla funzione localtime che a noi restituisce un puntatore ad essa relativa 
	      //teniamo traccia di questa struttura inizializzando il nostro puntatore al puntatore restituito
	      tempoReperito=localtime(&data);
	    
	      //inseriamo nella nostra struttura tm i valori reperiti
	      tempoAccensione.tm_mday=tempoReperito->tm_mday;
	      tempoAccensione.tm_mon=tempoReperito->tm_mon;
	      tempoAccensione.tm_year=tempoReperito->tm_year;
	      tempoAccensione.tm_hour=tempoReperito->tm_hour;
	      tempoAccensione.tm_min=tempoReperito->tm_min;
	      tempoAccensione.tm_sec=tempoReperito->tm_sec;
	      
	      
	      //check mirroring
	      if((connected==1))//se il dispositivo è connesso a qualcosa ed è stato modificato esternamente bisogna gestire mirroring
	      {
		      //stringa in cui inserisco lo stato del dispositivo da mandare in pipe
		      char messaggio[100];
		      sprintf(messaggio, "%dX%d", stato, id);
		      //scrivo lo stato del dispositivo sulla pipe
		      write(pipePadre, messaggio, strlen(messaggio));
		      //invio il segnale al padre
		      kill(getppid(), SIGIO);
	      }
	      
	      
	}else if((stato==1) && (strcmp(interruttore, "OFF")==0))
	{
			
		stato=0;
	
		
		//check mirroring
	      if((connected==1))//se il dispositivo è connesso a qualcosa ed è stato modificato esternamente bisogna gestire mirroring
	      {
		      //stringa in cui inserisco lo stato del dispositivo da mandare in pipe
		      char messaggio[100];
		      sprintf(messaggio, "%dX%d", stato, id);
		      //scrivo lo stato del dispositivo sulla pipe
		      write(pipePadre, messaggio, strlen(messaggio)+1);
		      //invio il segnale al padre
		      kill(getppid(), SIGIO);
	      }
		
	  }
	    
	     

	}

    	
	return 0;
}

//IMPLEMENTAZIONE FUNZIONI
//implementazione dell handler per il cambio del valore dell interruttore.
void handlerCambiaInterruttore(int signum)
{
	
	if(flagEsterno==0)
	{
		
		printf("lampadina legge pipe interruttore \n");
		
	char value[]="X";
	
	read(fdPadreInterruttore, value, strlen(value)+1);
	
	printf("ho reperito questo valore %s \n ", value);
	
	if(strcmp(value, "0")==0)
	{
		strcpy(interruttore, "OFF");
		printf("il mio interruttore è ora a %s \n", interruttore);
	}else
	{
		strcpy(interruttore, "ON");
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

//implementazione dell handler per la lettura dalla fifo comandi
void handlerLetturaPipe(int signum)
{
	//setto a uno il flag della lettura
	lettura=1;
}
