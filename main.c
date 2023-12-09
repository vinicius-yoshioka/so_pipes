#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define COMANDO_JOGAR "jogar"
#define JOGADA_PEDRA "pedra"
#define JOGADA_PAPEL "papel"
#define JOGADA_TESOURA "tesoura"

void client(char *name, int readfd, int writefd);
void server(int pipeFilho1[], int pipeFilho2[]);
void calcular_resultado(char *resposta1, char *resposta2, char **resultado1, char **resultado2);

int main()
{
	int pipePaiParaFilho1[2], pipeFilho1ParaPai[2]; // Comunicação pai <-> filho 1
	int pipePaiParaFilho2[2], pipeFilho2ParaPai[2]; // Comunicação pai <-> filho 2
	if (pipe(pipePaiParaFilho1) < 0 || pipe(pipeFilho1ParaPai) < 0 || pipe(pipePaiParaFilho2) < 0 || pipe(pipeFilho2ParaPai) < 0)
	{
		printf("Erro na chamada PIPE\n");
		exit(0);
	}

	int descritorFilho1, descritorFilho2;
	descritorFilho1 = fork();
	if (descritorFilho1 > 0)
		descritorFilho2 = fork();
	if (descritorFilho1 < 0 || descritorFilho2 < 0)
	{
		printf("Erro na chamada FORK\n");
		exit(0);
	}

	// Processo pai
	if (descritorFilho1 > 0 && descritorFilho2 > 0)
	{
		close(pipePaiParaFilho1[0]); // fecha leitura no pipePaiParaFilho1
		close(pipeFilho1ParaPai[1]); // fecha escrita no pipeFilho1ParaPai
		close(pipePaiParaFilho2[0]); // fecha leitura no pipePaiParaFilho2
		close(pipeFilho2ParaPai[1]); // fecha escrita no pipeFilho2ParaPai

		int pipeFilho1[2] = {pipeFilho1ParaPai[0], pipePaiParaFilho1[1]};
		int pipeFilho2[2] = {pipeFilho2ParaPai[0], pipePaiParaFilho2[1]};
		server(pipeFilho1, pipeFilho2);

		close(pipePaiParaFilho1[1]); // fecha escrita no pipePaiParaFilho1
		close(pipeFilho1ParaPai[0]); // fecha leitura no pipeFilho1ParaPai
		close(pipePaiParaFilho2[1]); // fecha escrita no pipePaiParaFilho2
		close(pipeFilho2ParaPai[0]); // fecha leitura no pipeFilho2ParaPai
		exit(0);
	}

	// Processo filho 1
	if (descritorFilho1 == 0)
	{
		close(pipePaiParaFilho1[1]); // fecha escrita no pipePaiParaFilho1
		close(pipeFilho1ParaPai[0]); // fecha leitura no pipeFilho1ParaPai

		client("1", pipePaiParaFilho1[0], pipeFilho1ParaPai[1]);

		close(pipePaiParaFilho1[0]); // fecha leitura no pipePaiParaFilho1
		close(pipeFilho1ParaPai[1]); // fecha escrita no pipeFilho1ParaPai
		exit(0);
	}

	// Processo filho 2
	if (descritorFilho2 == 0)
	{
		close(pipePaiParaFilho2[1]); // fecha escrita no pipePaiParaFilho2
		close(pipeFilho2ParaPai[0]); // fecha leitura no pipeFilho2ParaPai

		client("2", pipePaiParaFilho2[0], pipeFilho2ParaPai[1]);

		close(pipePaiParaFilho2[0]); // fecha leitura no pipePaiParaFilho2
		close(pipeFilho2ParaPai[1]); // fecha escrita no pipeFilho2ParaPai
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
	char buffer[BUFFER_SIZE] = "\0";

	while (1)
	{
		read(readfd, buffer, BUFFER_SIZE);
		if (strcmp(buffer, COMANDO_JOGAR) == 0)
		{
			printf("Client %s: jogando...\n", name);

			srand(time(NULL) ^ (getpid() << 16));
			int jogada = rand() % 3;
			switch (jogada)
			{
			case 0:
				strcpy(buffer, JOGADA_PEDRA);
				break;
			case 1:
				strcpy(buffer, JOGADA_PAPEL);
				break;
			case 2:
				strcpy(buffer, JOGADA_TESOURA);
				break;
			}

			printf("Client %s: jogada %s\n", name, buffer);
			write(writefd, buffer, strlen(buffer) + 1);
		}
		// TODO implementar recebimento de resultado do jogo
	}
}

/*
 * Executa no processo pai
 *
 * @param pipeFilho1: pipe para comunicação com o filho 1
 * @param pipeFilho2: pipe para comunicação com o filho 2
 **/
void server(int pipeFilho1[], int pipeFilho2[])
{
	int readfd1 = pipeFilho1[0];
	int writefd1 = pipeFilho1[1];
	int readfd2 = pipeFilho2[0];
	int writefd2 = pipeFilho2[1];

	char buffer1[BUFFER_SIZE] = "\0";
	char buffer2[BUFFER_SIZE] = "\0";

	char resultado1[BUFFER_SIZE] = "\0";
	char resultado2[BUFFER_SIZE] = "\0";

	int rodada = 1;
	while (1)
	{
		printf("Rodada: %d\n", rodada++);

		strcpy(buffer1, COMANDO_JOGAR);
		printf("Server > Client 1: %s\n", buffer1);
		write(writefd1, buffer1, strlen(buffer1) + 1);

		strcpy(buffer2, COMANDO_JOGAR);
		printf("Server > Client 2: %s\n", buffer2);
		write(writefd2, buffer2, strlen(buffer2) + 1);

		read(readfd1, buffer1, BUFFER_SIZE);
		printf("Client 1 > Server: %s\n", buffer1);

		read(readfd2, buffer2, BUFFER_SIZE);
		printf("Client 2 > Server: %s\n", buffer2);

		calcular_resultado(buffer1, buffer2, &resultado1, &resultado2);

		if (rodada > 1)
		{
			break;
		}
	}
}

void calcular_resultado(char *resposta1, char *resposta2, char **resultado1, char **resultado2)
{
	if (strcmp(resposta1, resposta2) == 0)
	{
		strcpy(resultado1, RESULTADO_EMPATE);
		strcpy(resultado2, RESULTADO_EMPATE);
		printf("Server: Empate!\n");
	}
	else if (strcmp(resposta1, JOGADA_PEDRA) == 0 && strcmp(resposta2, JOGADA_TESOURA) == 0)
	{
		strcpy(resultado1, RESULTADO_VITORIA);
		strcpy(resultado2, RESULTADO_DERROTA);
		printf("Server: Pedra quebra tesoura. Client 1 venceu!\n");
	}
	else if (strcmp(resposta1, JOGADA_TESOURA) == 0 && strcmp(resposta2, JOGADA_PEDRA) == 0)
	{
		strcpy(resultado1, RESULTADO_DERROTA);
		strcpy(resultado2, RESULTADO_VITORIA);
		printf("Server: Pedra quebra tesoura. Client 2 venceu!\n");
	}
	else if (strcmp(resposta1, JOGADA_PAPEL) == 0 && strcmp(resposta2, JOGADA_PEDRA) == 0)
	{
		strcpy(resultado1, RESULTADO_VITORIA);
		strcpy(resultado2, RESULTADO_DERROTA);
		printf("Server: Papel embrulha pedra. Client 1 venceu!\n");
	}
	else if (strcmp(resposta1, JOGADA_PEDRA) == 0 && strcmp(resposta2, JOGADA_PAPEL) == 0)
	{
		strcpy(resultado1, RESULTADO_DERROTA);
		strcpy(resultado2, RESULTADO_VITORIA);
		printf("Server: Papel embrulha pedra. Client 2 venceu!\n");
	}
	else if (strcmp(resposta1, JOGADA_TESOURA) == 0 && strcmp(resposta2, JOGADA_PAPEL) == 0)
	{
		strcpy(resultado1, RESULTADO_VITORIA);
		strcpy(resultado2, RESULTADO_DERROTA);
		printf("Server: Tesoura corta papel. Client 1 venceu!\n");
	}
	else if (strcmp(resposta1, JOGADA_PAPEL) == 0 && strcmp(resposta2, JOGADA_TESOURA) == 0)
	{
		strcpy(resultado1, RESULTADO_DERROTA);
		strcpy(resultado2, RESULTADO_VITORIA);
		printf("Server: Tesoura corta papel. Client 2 venceu!\n");
	}
}
