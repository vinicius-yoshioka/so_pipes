#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define TIPO_JOGAR "jogar"
#define TIPO_JOGADA "jogada"
#define TIPO_RESULTADO "resultado"
#define TIPO_SAIR "sair"
#define JOGADA_PEDRA "pedra"
#define JOGADA_PAPEL "papel"
#define JOGADA_TESOURA "tesoura"
#define RESULTADO_DERROTA "derrota"
#define RESULTADO_EMPATE "empate"
#define RESULTADO_VITORIA "vitoria"

typedef struct _Mensagem
{
	char tipo[BUFFER_SIZE];
	char valor[BUFFER_SIZE];
} Mensagem;

void server(int readfd, int writefd);
void *client_at_thread(void *arg);
void client_at_process(int readfd, int writefd);
void calcular_resultado(Mensagem *buffer_client_at_thread, Mensagem *buffer_client_at_process);

int main()
{
	int pipePaiParaFilho[2], pipeFilhoParaPai[2];
	if (pipe(pipePaiParaFilho) < 0 || pipe(pipeFilhoParaPai) < 0)
	{
		printf("Erro na chamada PIPE\n");
		exit(0);
	}

	int descritor = fork();
	if (descritor < 0)
	{
		printf("Erro na chamada FORK\n");
		exit(0);
	}

	// Processo pai
	if (descritor > 0)
	{
		close(pipePaiParaFilho[0]);
		close(pipeFilhoParaPai[1]);

		server(pipeFilhoParaPai[0], pipePaiParaFilho[1]);

		close(pipePaiParaFilho[1]);
		close(pipeFilhoParaPai[0]);
		exit(0);
	}

	// Processo filho
	if (descritor == 0)
	{
		close(pipePaiParaFilho[1]);
		close(pipeFilhoParaPai[0]);

		client_at_process(pipePaiParaFilho[0], pipeFilhoParaPai[1]);

		close(pipePaiParaFilho[0]);
		close(pipeFilhoParaPai[1]);
		exit(0);
	}

	return 0;
}

/*
 * Executa nos processos filhos
 *
 * @param readfd: leitura do pipe pai para filho
 * @param writefd: escrita no pipe filho para pai
 **/
void client(char *name, int readfd, int writefd)
{
	Mensagem buffer;

	while (1)
	{
		read(readfd, &buffer, sizeof(Mensagem));
		if (strcmp(buffer.tipo, TIPO_JOGAR) == 0)
		{
			strcpy(buffer.tipo, TIPO_JOGADA);

			srand(time(NULL) ^ (getpid() << 16));
			int jogada = rand() % 3;
			switch (jogada)
			{
			case 0:
				strcpy(buffer.valor, JOGADA_PEDRA);
				break;
			case 1:
				strcpy(buffer.valor, JOGADA_PAPEL);
				break;
			case 2:
				strcpy(buffer.valor, JOGADA_TESOURA);
				break;
			}

			write(writefd, &buffer, sizeof(buffer));
		}
		else if (strcmp(buffer.tipo, TIPO_RESULTADO) == 0)
		{
			if (strcmp(buffer.valor, RESULTADO_VITORIA) == 0)
				printf("Client (subprocesso): %s\n", RESULTADO_VITORIA);
			else if (strcmp(buffer.valor, RESULTADO_DERROTA) == 0)
				printf("Client (subprocesso): %s\n", RESULTADO_DERROTA);
			else
				printf("Client (subprocesso): %s\n", RESULTADO_EMPATE);
		}
		else if (strcmp(buffer.tipo, TIPO_SAIR) == 0)
		{
			printf("Client (subprocesso): saindo...\n");
			exit(0);
			break;
		}
	}
}

void calcular_resultado(Mensagem *buffer_client_at_thread, Mensagem *buffer_client_at_process)
{
	strcpy(buffer_client_at_thread->tipo, TIPO_RESULTADO);
	strcpy(buffer_client_at_process->tipo, TIPO_RESULTADO);

	if (strcmp(buffer_client_at_thread->valor, buffer_client_at_process->valor) == 0)
	{
		strcpy(buffer_client_at_thread->valor, RESULTADO_EMPATE);
		strcpy(buffer_client_at_process->valor, RESULTADO_EMPATE);
		printf("Server: Empate!\n");
	}
	else if (strcmp(buffer_client_at_thread->valor, JOGADA_PEDRA) == 0 && strcmp(buffer_client_at_process->valor, JOGADA_TESOURA) == 0)
	{
		strcpy(buffer_client_at_thread->valor, RESULTADO_VITORIA);
		strcpy(buffer_client_at_process->valor, RESULTADO_DERROTA);
		printf("Server: Pedra quebra tesoura. Client (thread) venceu!\n");
	}
	else if (strcmp(buffer_client_at_thread->valor, JOGADA_TESOURA) == 0 && strcmp(buffer_client_at_process->valor, JOGADA_PEDRA) == 0)
	{
		strcpy(buffer_client_at_thread->valor, RESULTADO_DERROTA);
		strcpy(buffer_client_at_process->valor, RESULTADO_VITORIA);
		printf("Server: Pedra quebra tesoura. Client (subprocesso) venceu!\n");
	}
	else if (strcmp(buffer_client_at_thread->valor, JOGADA_PAPEL) == 0 && strcmp(buffer_client_at_process->valor, JOGADA_PEDRA) == 0)
	{
		strcpy(buffer_client_at_thread->valor, RESULTADO_VITORIA);
		strcpy(buffer_client_at_process->valor, RESULTADO_DERROTA);
		printf("Server: Papel embrulha pedra. Client (thread) venceu!\n");
	}
	else if (strcmp(buffer_client_at_thread->valor, JOGADA_PEDRA) == 0 && strcmp(buffer_client_at_process->valor, JOGADA_PAPEL) == 0)
	{
		strcpy(buffer_client_at_thread->valor, RESULTADO_DERROTA);
		strcpy(buffer_client_at_process->valor, RESULTADO_VITORIA);
		printf("Server: Papel embrulha pedra. Client (subprocesso) venceu!\n");
	}
	else if (strcmp(buffer_client_at_thread->valor, JOGADA_TESOURA) == 0 && strcmp(buffer_client_at_process->valor, JOGADA_PAPEL) == 0)
	{
		strcpy(buffer_client_at_thread->valor, RESULTADO_VITORIA);
		strcpy(buffer_client_at_process->valor, RESULTADO_DERROTA);
		printf("Server: Tesoura corta papel. Client (thread) venceu!\n");
	}
	else if (strcmp(buffer_client_at_thread->valor, JOGADA_PAPEL) == 0 && strcmp(buffer_client_at_process->valor, JOGADA_TESOURA) == 0)
	{
		strcpy(buffer_client_at_thread->valor, RESULTADO_DERROTA);
		strcpy(buffer_client_at_process->valor, RESULTADO_VITORIA);
		printf("Server: Tesoura corta papel. Client (subprocesso) venceu!\n");
	}
}
