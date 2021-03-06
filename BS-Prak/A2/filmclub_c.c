#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int kontostand;
/*Mutex deklarieren*/
pthread_mutex_t lock;

void* filmebestellen(void *pv);
void* getraenkeundspeisen(void *pv);
void* ueberweisen(void *pv);

int main(int argc, char **argv){
	int status_film;
	pthread_t thread_film;
	int status_nahrung;
	pthread_t thread_nahrung;
	int status_uberweiung;
	pthread_t thread_uberweisung;
	
	kontostand = 1000;
	
	/*Mutex initialisieren und Fehler abfangen*/
	if( pthread_mutex_init(&lock, NULL) != 0 ){
		printf("Fehler beim initialisieren des Mutex\n");
	}
	
	printf("Mutex beim Start des Programms: %d\n",lock);
	
	/*Threads starten, aufsammeln und Fehler abfangen*/
	status_film = pthread_create(&thread_film, NULL, &filmebestellen, NULL);
	if(status_film) printf("Fehler beim Filmthread\n");
	
	status_nahrung = pthread_create(&thread_nahrung, NULL, &getraenkeundspeisen, NULL);
	if(status_nahrung) printf("Fehler beim Getraenke- und Speisenthread\n");
	
	status_uberweiung = pthread_create(&thread_uberweisung, NULL, &ueberweisen, NULL);
	if(status_uberweiung) printf("Fehler beim Uberweisungsthread\n");
	
	status_film = pthread_join(thread_film, NULL);
	if(status_uberweiung) printf("Fehler beim aufsammeln des Filmthreads\n");
	
	status_nahrung = pthread_join(thread_nahrung, NULL);
	if(status_nahrung) printf("Fehler beim aufsammeln des Nahrungsthreads\n");
	
	status_uberweiung = pthread_join(thread_uberweisung, NULL);
	if(status_uberweiung) printf("Fehler beim aufsammeln des Uberweisungsthread\n");
	
	printf("Endkontostand: %d\n",kontostand);
	
	/*Mutex freigeben*/
	if( pthread_mutex_destroy(&lock) != 0) printf("Konnte Mutex nicht zerstören\n");
	/*Alles okay, 0 zurückgeben*/
	return 0;
}

void* filmebestellen(void *pv){
	int localkonto;
	int counter = 0;
	while(counter < 5){
		
		/*Wir versuchen den Mutex zu sperren, sollte das nicht klappen warten wir 2 Sekunden*/
		if( pthread_mutex_trylock(&lock)  != 0){
			printf("Mutex konnte nicht gelockt werden\n");
			printf("warte...\n");
			sleep(2);
		}else{
			printf("Bestelle Filme\n");
			localkonto = kontostand;
			printf("Kontostand vorher: %d\n",localkonto);
			localkonto -= 200;
			kontostand = localkonto;
			printf("Kontostand nachher: %d\n",localkonto);
			/*Mutex wieder freigeben und Fehler abfangen*/
			if( pthread_mutex_unlock(&lock) != 0) printf("Mutex konnte nicht geunlockt werden\n");
			counter++;
		}
	}
	
	pthread_exit(NULL);
}

void* getraenkeundspeisen(void *pv){
	int localkonto;
	int counter = 0;
	while(counter < 5){
		/*Wir versuchen den Mutex zu locken, falls das nicht klappt, warten wir 2 Sekunden*/
		if( pthread_mutex_trylock(&lock) != 0 ){
			printf("Mutex konnte nicht gelockt werden\n");
			printf("warte...\n");
			sleep(3);
		}else{
			printf("Kaufe Getraenke und Speisen\n");
			localkonto = kontostand;
			printf("Kontostand vorher %d\n",localkonto);
			localkonto -= 400;
			kontostand = localkonto;
			printf("Kontostand nachher: %d\n",localkonto);
			/*Mutex wieder freigeben und Fehler abfangen*/
			if( pthread_mutex_unlock(&lock) != 0 ){
				printf("Mutex konnte nicht entsperrt werden\n");
			}
			counter++;
		}
	}
	
	pthread_exit(NULL);
}

void* ueberweisen(void *pv){
	int localkonto;
	int counter = 0;
	while(counter < 2){
		/*Wir versuchen den Mutex zu sperren, sollte das nicht klappen warten wir 2 Sekunden*/
		if( pthread_mutex_trylock(&lock) != 0){
			printf("Mutex konnte nicht gelockt werden\n");
			printf("warte...\n");
			sleep(8);
		}else{
			/* printf("Gelockter Mutex %d\n",lock); */
			printf("Ueberweise Geld\n");
			localkonto = kontostand;
			printf("Kontostand vorher %d\n",localkonto);
			localkonto += 2000;
			kontostand = localkonto;
			printf("Kontostand nachher %d\n",localkonto);
			/*Mutex freigeben und Fehler abfangen*/
			if( pthread_mutex_unlock(&lock) != 0 ){
				printf("Mutex konnte nicht geunlockt werden\n");
			}
			counter++;
		}
	}
	
	pthread_exit(NULL);
}