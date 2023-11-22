#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXBUFF 1024

int main()
{
	int pipe1[2]; // comunicacao pai -> filho
	int pipe2[2]; // comunicacao filho -> pai

	if (pipe(pipe1) < 0 || pipe(pipe2) < 0)
	{
		printf("Erro na chamada PIPE");
		exit(0);
	}

	int descritor = fork();
	if (descritor < 0)
	{
		printf("Erro na chamada FORK");
		exit(0);
	}

	// PROCESSO PAI
	if (descritor > 0)
	{
		close(pipe1[0]); // fecha leitura no pipe1
		close(pipe2[1]); // fecha escrita no pipe2

		// Chama CLIENTE no PAI
		client(pipe2[0], pipe1[1]);

		close(pipe1[1]); // fecha escrita no pipe1
		close(pipe2[0]); // fecha leitura no pipe2
		exit(0);
	}
	// PROCESSO FILHO
	else
	{
		close(pipe1[1]); // fecha escrita no pipe1
		close(pipe2[0]); // fecha leitura no pipe2

		// Chama SERVIDOR no FILHO
		server(pipe1[0], pipe2[1]);

		close(pipe1[0]); // fecha leitura no pipe1
		close(pipe2[1]); // fecha escrita no pipe2
		exit(0);
	}

	return 0;
}

/*
 * Função Client: Executa no processo PAI
 *
 * Envia o nome do arquivo para o FILHO
 * Recebe os dados do FILHO e imprime na tela
 *
 * @param readfd: leitura do pipe2[0]
 * @param writefd: escrita no pipe1[1]
 */
void client(int readfd, int writefd)
{
	char buff[MAXBUFF];

	while (1)
	{
		printf(" \n Client->");
		gets(buff);
		write(writefd, buff, 10);

		read(readfd, buff, 10);
		printf(" \n Client <- %s", buff);
	}
}

/*
 * Função Server: Executa no processo FILHO
 *
 * Abre o arquivo solicitado e envia seu conteudo para o PAI
 *
 * @param readfd: leitura do pipe1[0]
 * @param writefd: escrita no pipe2[1]
 */
void server(int readfd, int writefd)
{
	char buff[MAXBUFF];

	while (1)
	{
		read(readfd, buff, 10);
		printf(" \n Server<- %s", buff);

		printf(" \n Server->");
		gets(buff);
		write(writefd, buff, 10);
	}
}
