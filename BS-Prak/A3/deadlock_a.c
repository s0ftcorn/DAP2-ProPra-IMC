#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>


#define DOZENT_A 0
#define DOZENT_B 1


enum STATUS {FREIZEIT, WARTEN, BENUTZEN};


pthread_t dozenten[2];
enum STATUS dozenten_status[2];
/* Deklaration des Semaphors für die Geräte */
sem_t sem_gerat;


/* Funktionen deklarieren */
void *dozenten_thread(void *);
void freizeit(int, char);
void deadlock_erkennung(void);
void programmabbruch(int sig);




int main (void) {


	int id_a = DOZENT_A;
	int id_b = DOZENT_B;


	/*
	 * Signalhandler registrieren.
	 * Z.B. bei CTRL-C aus der Shell wird die Funktion programmabbruch ausgefuehrt.
	 */
	struct sigaction aktion;
	aktion.sa_handler = &programmabbruch;
	aktion.sa_flags = 0;
	sigemptyset(&aktion.sa_mask);

	if (sigaction(SIGINT, &aktion, NULL) == -1) {
		perror("set actionhandler");
		exit(EXIT_FAILURE);
	}

	/*
	 * Der Status der Dozenten wird zunaechst auf FREIZEIT gesetzt.
	 */
	dozenten_status[DOZENT_A] = FREIZEIT;
	dozenten_status[DOZENT_B] = FREIZEIT;

	/*Initialisieren des Semaphors mit 2 */
	if( sem_init(&sem_gerat,0,2) == -1){
		/*Auslesen des Fehlerwertes aus errno */
		perror("Fehler beim initialisieren des Semaphors!\n");
		exit(0);
	}
	
	/*
	 * Die Dozenten-Threads werden nun erzeugt.
	 */
	if (pthread_create(&dozenten[DOZENT_A], NULL, &dozenten_thread, (void *) &id_a) != 0) {
		perror("Create DOZENT_A");
		exit(EXIT_FAILURE);
	}

	if (pthread_create(&dozenten[DOZENT_B], NULL, &dozenten_thread, (void *) &id_b) != 0) {
		perror("Create DOZENT_B");
		exit(EXIT_FAILURE);
	}


	/* 
	 * Der Vaterprozess uebernimmt die Deadlockerkennung.
	 */
	deadlock_erkennung();
	
	return 0;
}




/*
 * Die Funktion der beiden Threads (Dozent_A und Dozent_B).
 * Hier findet das Belegen der Ressourcen und die Veranstaltung statt.
 */

void *dozenten_thread(void *id) {


	int dozent = *((int *) id);
	char c = (dozent == DOZENT_A) ? 'A' : 'B';


	/* Dozent_B benoetigt etwas mehr Zeit... */
	if (dozent == DOZENT_B) {
		sleep(4);
	}


	printf("Dozent_%c: Wurde gestartet.\n", c);


	while (1) {

		printf("Dozent_%c: Mal schauen, ob das Notebook oder der Beamer verfuegbar ist.\n", c);

		/*Semaphor um eins dekrementieren, da eine Resource belegt ist*/
		sem_wait(&sem_gerat);
		printf("Dozent_%c: Jetzt habe ich schon einmal den Notebook oder den Beamer.\n", c);
		
		/*Zustände entsprechend aendern*/
		if(c == 'A'){
			dozenten_status[DOZENT_A] = WARTEN;
			printf("Dozent_A: Ich warte auf Notebook oder Beamner.\n");
		}else{
			dozenten_status[DOZENT_B] = WARTEN;
			printf("Dozent_B: Ich warte auf Notebook oder Beamner.\n");
		}

		/*Dozenten brauchen unterschiedlich viel Zeit*/
		if(c == 'A'){
			sleep(3);
			
		}else{
			sleep(5);
		}

		printf("Dozent_%c: Mal schauen, ob das zweite Geraet auch verfuegbar ist.\n", c);
		
		/*Jeweilige Dozenten nehmen sich das andere Geraet und brauchen dafür unterschiedlich viel Zeit*/
		if(c == 'A'){
			sem_wait(&sem_gerat);
			sleep(3);
			
		}else{
			sem_wait(&sem_gerat);
			sleep(5);
		}

		printf("Dozent_%c: Jetzt habe ich den Notebook und den Beamer! Ich kann nun meine Veranstaltung durchfuehren!\n", c);

		if(c == 'A'){
			dozenten_status[DOZENT_A] = BENUTZEN;
			printf("Dozent_A: Arbeit arbeit.\n");
		}else{
			dozenten_status[DOZENT_B] = BENUTZEN;
			printf("Dozent_B: Arbeit arbeit.\n");
		}

		/* Die Veranstaltung dauert 5 Sekunden */

		sleep(5);

		printf("Dozent_%c: Die Veranstaltung ist beendet, nun bringe ich beide Geraet wieder zurueck!\n", c);
		
		/*Geraete wieder "zurueckgeben" und Semaphor inkrementieren*/
		sem_post(&sem_gerat);
		sem_post(&sem_gerat);

		if(c == 'A'){
			dozenten_status[DOZENT_A] = FREIZEIT;
			printf("Dozent_A: Ich warte auf Notebook oder Beamner.\n");
		}else{
			dozenten_status[DOZENT_B] = FREIZEIT;
			printf("Dozent_B: Ich warte auf Notebook oder Beamner.\n");
		}

		/* 
		 * Hier wird die Freizeit eines Dozenten-Threads berechnet. Veraendert den
		 * Inhalt der Funktion besser nicht, sonst bekommt ihr keine Deadlocks
		 * mehr, oder muesst sehr lange darauf warten. 
		 */

		freizeit(dozent, c);
	}

	
	pthread_exit(NULL);
}




/*
 * Die Funktion fuer einen Dozenten-Thread.
 * Hier wird der Feierabend implementiert.
 */

void freizeit(int dozent, char c) {
	static int i = 0;

	/* 
	 * Dozent_A kommt nach 8 Zeiteinheiten wieder, Dozent_B in immer schnelleren
	 * Abstaenden. Durch diesen zeitlichen Ablauf wird der Deadlock provoziert.
	 */

	if (dozent == DOZENT_A) {
		printf("Dozent_%c: Jetzt fahre ich nach Hause. In 8 Stunden bin ich aber wieder da!\n", c);
		sleep(8);
	} else {
		if ((8 - i) < 0) {
			i = 0;
		}
		printf("Dozent_%c: Jetzt fahre ich nach Hause. In %d Stunden bin ich aber wieder da!\n", c, 8 - i);
		sleep(8 - i);
		i += 6;
  }
}




/*
 * Die Funktion des Vaterprozesses.
 * Hier findet die Deadlockerkennung und -beseitigung statt.
 */

void deadlock_erkennung(void) {
	while (1) {
		sleep(12);

		/* HIER MUSS EUER CODE EINGEFUEGT WERDEN Aufgabenteil b): */

	}
}




/*
 * Die Funktion, die bei einem Programmabbruch ausgefuehrt wird (z.B. bei
 * CTRL-c in der Shell). Hier sollen die Semaphore geloescht werden.
 * Darueberhinaus sollen die Dozenten-Threads aufgeraeumt werden.
 */

void programmabbruch(int sig) {

	/* HIER MUSS EUER CODE EINGEFUEGT WERDEN Aufgabenteil b): */
	/*Vernünftiges zerstören des Semaphors */
	sem_destroy(&sem_gerat);
	exit(0);
}
