#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define TIPO_JOGAR "jogar"
#define TIPO_JOGADA "resposta"
#define TIPO_RESULTADO "resultado"
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

void client(char *name, int readfd, int writefd);
void server(int pipeFilho1[], int pipeFilho2[]);
void calcular_resultado(Mensagem *resposta1, Mensagem *resposta2);

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
				printf("Client %s: %s\n", name, RESULTADO_VITORIA);
			else if (strcmp(buffer.valor, RESULTADO_DERROTA) == 0)
				printf("Client %s: %s\n", name, RESULTADO_DERROTA);
			else
				printf("Client %s: %s\n", name, RESULTADO_EMPATE);
		}
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

	Mensagem buffer1;
	Mensagem buffer2;

	int rodada = 1;
	while (1)
	{
		printf("Rodada: %d\n", rodada++);

		strcpy(buffer1.tipo, TIPO_JOGAR);
		strcpy(buffer1.valor, "\0");
		printf("Server > Client 1: %s\n", buffer1.valor);
		write(writefd1, &buffer1, sizeof(buffer1));

		strcpy(buffer2.tipo, TIPO_JOGAR);
		strcpy(buffer2.valor, "\0");
		printf("Server > Client 2: %s\n", buffer2.valor);
		write(writefd2, &buffer2, sizeof(buffer2));

		read(readfd1, &buffer1, sizeof(Mensagem));
		printf("Client 1 > Server: %s\n", buffer1.valor);

		read(readfd2, &buffer2, sizeof(Mensagem));
		printf("Client 2 > Server: %s\n", buffer2.valor);

		calcular_resultado(&buffer1, &buffer2);

		printf("Server > Client 1: %s\n", buffer1.valor);
		write(writefd1, &buffer1, sizeof(buffer1));

		printf("Server > Client 2: %s\n", buffer2.valor);
		write(writefd2, &buffer2, sizeof(buffer2));

		if (rodada > 1)
		{
			break;
		}
	}
}

void calcular_resultado(Mensagem *mensagem1, Mensagem *mensagem2)
{
	strcpy(mensagem1->tipo, TIPO_RESULTADO);
	strcpy(mensagem2->tipo, TIPO_RESULTADO);

	if (strcmp(mensagem1->valor, mensagem2->valor) == 0)
	{
		strcpy(mensagem1->valor, RESULTADO_EMPATE);
		strcpy(mensagem2->valor, RESULTADO_EMPATE);
		printf("Server: Empate!\n");
	}
	else if (strcmp(mensagem1->valor, JOGADA_PEDRA) == 0 && strcmp(mensagem2->valor, JOGADA_TESOURA) == 0)
	{
		strcpy(mensagem1->valor, RESULTADO_VITORIA);
		strcpy(mensagem2->valor, RESULTADO_DERROTA);
		printf("Server: Pedra quebra tesoura. Client 1 venceu!\n");
	}
	else if (strcmp(mensagem1->valor, JOGADA_TESOURA) == 0 && strcmp(mensagem2->valor, JOGADA_PEDRA) == 0)
	{
		strcpy(mensagem1->valor, RESULTADO_DERROTA);
		strcpy(mensagem2->valor, RESULTADO_VITORIA);
		printf("Server: Pedra quebra tesoura. Client 2 venceu!\n");
	}
	else if (strcmp(mensagem1->valor, JOGADA_PAPEL) == 0 && strcmp(mensagem2->valor, JOGADA_PEDRA) == 0)
	{
		strcpy(mensagem1->valor, RESULTADO_VITORIA);
		strcpy(mensagem2->valor, RESULTADO_DERROTA);
		printf("Server: Papel embrulha pedra. Client 1 venceu!\n");
	}
	else if (strcmp(mensagem1->valor, JOGADA_PEDRA) == 0 && strcmp(mensagem2->valor, JOGADA_PAPEL) == 0)
	{
		strcpy(mensagem1->valor, RESULTADO_DERROTA);
		strcpy(mensagem2->valor, RESULTADO_VITORIA);
		printf("Server: Papel embrulha pedra. Client 2 venceu!\n");
	}
	else if (strcmp(mensagem1->valor, JOGADA_TESOURA) == 0 && strcmp(mensagem2->valor, JOGADA_PAPEL) == 0)
	{
		strcpy(mensagem1->valor, RESULTADO_VITORIA);
		strcpy(mensagem2->valor, RESULTADO_DERROTA);
		printf("Server: Tesoura corta papel. Client 1 venceu!\n");
	}
	else if (strcmp(mensagem1->valor, JOGADA_PAPEL) == 0 && strcmp(mensagem2->valor, JOGADA_TESOURA) == 0)
	{
		strcpy(mensagem1->valor, RESULTADO_DERROTA);
		strcpy(mensagem2->valor, RESULTADO_VITORIA);
		printf("Server: Tesoura corta papel. Client 2 venceu!\n");
	}
}
