#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <semaphores.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define CICLOS 10

char *pais[3]={"Peru","Bolvia","Colombia"};
int *g;
int semaforo; //Identificador del semaforo

void seminit(); //Inicializa el identificador de semaforo y coloca su valor en 1
void semwait(int semid); //Espera a que se libere el semaforo
void semsignal(int semid); //Incremente la cuenta del semaforo

void seminit()
{

	semaforo = semget((key_t)0x1234,1,0666|IPC_CREAT); // Pedir al SO que me de un semáforo
	semctl(semaforo,0,SETVAL,1);	// Inicializa el elemento 0 del arreglo de semáforos con 1

}

void semwait(int semid)
{
	struct sembuf s;

	s.sem_num= 0;
	s.sem_op= -1;	// Wait
	s.sem_flg= 0;
	semop(semid,&s,1); //Ejecuta la operación de espera al semaforo

	return;
}

void semsignal(int semid)
{
	struct sembuf s;

	s.sem_num= 0;
	s.sem_op=  1;	// Signal
	s.sem_flg= 0;
	semop(semid,&s,1); //Ejecuta la operación de incremento al semaforo

	return;
}

void proceso(int i)
{
	int k;
	int l;
	for(k=0;k<CICLOS;k++)
	{
		// Entrada a la sección crítica
		semwait(semaforo); //Espera a que el semaforo sea liberado y al entrar lo decrementa

		printf("Entra %s",pais[i]);
		fflush(stdout);
		sleep(rand()%3);
		printf("- %s Sale\n",pais[i]);
		// Salida de la sección crítica

		semsignal(semaforo); //Incrementa la cuenta de semaforo

		// Espera aleatoria fuera de la sección crítica
		sleep(rand()%3);  
	}
	exit(0); // Termina el proceso
}

int main()
{
	int pid;
	int status;
	int args[3];
	int i;
	
	srand(getpid());

	seminit();  //Inicializa semaforo 

	for(i=0;i<3;i++)
	{
		// Crea un nuevo proceso hijo que ejecuta la función proceso()
		pid=fork();
		if(pid==0)
			proceso(i);
	}
	for(i=0;i<3;i++)
		pid = wait(&status);


	//Sem Kill 
	semctl(semaforo,0,IPC_RMID,0);
}
