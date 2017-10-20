#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>

#define CICLOS 10

// Definir una estructura que se va a usar para mandar los mensajes
typedef struct {
	long msg_type;		// Tipo de mensaje, debe ser long
	char mensaje[100];	// Contenido
	} MSGTYPE;

char *pais[3]={"Peru","Bolvia","Colombia"};
int *g;

int msgqid; //Identificador del mensaje adquirido
key_t keyval; //Llave para solicitar mensaje
MSGTYPE msgbuff; //Buffer para estructura de mensaje


void msgInit(); //Inicializa el identificador de mensaje utilizando la llave
void emisor(); //Agrega un mensaje la pila de mensajes
void receptor(); //espera recibir un mensaje de la pila

void msgInit(){
	keyval = ftok("mensajes.c", 'W');  //Inicializa la llave utilizando el nombre del archivo y un valor de 8 bits
	msgqid = msgget( keyval, IPC_CREAT | 0660 ); // Retorna el identificador de una pila de mensajes nueva

	msgbuff.msg_type = 1;  //Define el tipo de mensaje
	strncpy(msgbuff.mensaje , "hello", 100);  // Define el contenido del mensaje
}

void emisor(){
 	msgsnd(msgqid, (MSGTYPE *)&msgbuff, sizeof(msgbuff), 0); //Agrega un mensaje a la pila
}

void receptor(){
	msgrcv(msgqid, (MSGTYPE *)&msgbuff, sizeof(msgbuff), 0, 0);	//Espera a recibir mensaje
}

void proceso(int i)
{
	int k;
	int l;
	for(k=0;k<CICLOS;k++)
	{
		// Entrada a la sección crítica
		receptor(); //Detiene el flujo de proceso hasta ver un mensaje en la pila

		printf("Entra %s",pais[i]);
		fflush(stdout);
		sleep(rand()%3);
		printf("- %s Sale\n",pais[i]);

		// Salida de la sección crítica
		emisor(); //Envia mensaje para notificar que ha salido de la sección critica	

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
	msgInit(); //Inicializa pila de mensajes
	emisor(); //Transmite el primer mensaje para que el primer proceso en llegar no detenga su flujo 

	for(i=0;i<3;i++)
	{
		// Crea un nuevo proceso hijo que ejecuta la función proceso()
		pid=fork();
		if(pid==0)
			proceso(i);
	}
	for(i=0;i<3;i++)
		pid = wait(&status);

	msgctl(msgqid, IPC_RMID, NULL); //Destruye la pila de mensajes 

}
