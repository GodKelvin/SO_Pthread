//#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

/* INICIO DOS PARAMETROS GLOBAIS */
#define qtdThread 4
/* Conforme conversado em sala de aula: macroBlocos multiplos e matriz quadratica*/
#define tamMacroBloco 200
#define dimenMatriz 20000
#define rangeNumAleat 29999
#define sementeNum 9
#define macroBlocos (dimenMatriz * dimenMatriz) / (tamMacroBloco * tamMacroBloco);

int qtdPrimosGlobais = 0;
int matrizGlobal[dimenMatriz][dimenMatriz];
int qtdMacroBlocos = macroBlocos;
int linhaGlobal = 0, colunaGlobal = 0;

clock_t tempoInicial, tempoFinal;
float tempoTotal;

/* Criando os Mutex para a protecao das zonas criticas */
pthread_mutex_t mutexQtdPrimos, mutexPosicoes, mutexQtdMacroBlocos;

/* FIM DOS PARAMETROS GLOBAIS */

/*0 == False; 1 == True*/
int f_ehPrimo(int num)
{
	if (num <= 1)
	{
		return 0;
	}
	int ehPrimo = 1;
	int i = 2;
	while (i*i <= num)
	{
		if (num % i == 0)
		{
			ehPrimo = 0;
			return ehPrimo;
		}
		i++;
	}
	return ehPrimo;
}


void f_numAleat()
{
	//srand(time(NULL));
	srand(sementeNum);

	for (int i = 0; i < dimenMatriz; i++)
	{
		for (int j = 0; j < dimenMatriz; j++)
		{
			matrizGlobal[i][j] = rand() % rangeNumAleat;
		}
	}
}
/* Funcao para fins de Debug */
void f_printMatriz()
{
	for (int i = 0; i < dimenMatriz; i++)
	{
		for (int j = 0; j < dimenMatriz; j++)
		{
			printf("%d, ", matrizGlobal[i][j]);
		}
		printf("\n");
	}
}

void f_buscaSerial()
{
	for (int i = 0; i < dimenMatriz; i++)
	{
		for (int j = 0; j < dimenMatriz; j++)
		{
			if (f_ehPrimo(matrizGlobal[i][j]))
			{
				qtdPrimosGlobais++;
			}
		}
	}
}

/* Nao precisa de argumentos, exceto se for passado outro alem da Thread */
void f_percorreMacroBlocos()
{
	int linhaLocal = 0;
	int colunaLocal = 0;
	int qtdPrimosLocais = 0;

	/* Enquanto existirem macroblocos a serem verificados*/
	while (qtdMacroBlocos != 0)
	{
		/* Zona critica da variavel "qtdMacroBlocos" */
		pthread_mutex_lock(&mutexQtdMacroBlocos);
		qtdMacroBlocos--;
		pthread_mutex_unlock(&mutexQtdMacroBlocos);

		/* Zona criticas das variaveis que manipulam as posicoes */
		/*Fim das colunas. Necessario pular linhas. Voltar pro inicio das colunas*/
		pthread_mutex_lock(&mutexPosicoes);
		if (colunaGlobal + tamMacroBloco > dimenMatriz)
		{
			linhaGlobal += tamMacroBloco;
			colunaGlobal = tamMacroBloco;
			linhaLocal = linhaGlobal;
			colunaLocal = 0;

		}
		else
		{
			linhaLocal = linhaGlobal;
			colunaLocal = colunaGlobal;
			/*Aumentando para o inicio do proximo macroBloco*/
			colunaGlobal += tamMacroBloco;
		}
		pthread_mutex_unlock(&mutexPosicoes);

		/* Loop de verificacao do respectivo MacroBloco*/
		for (int i = 0; i < tamMacroBloco; i++)
		{
			for (int j = 0; j < tamMacroBloco; j++)
			{
				if (f_ehPrimo(matrizGlobal[i + linhaLocal][j + colunaLocal]))
				{
					qtdPrimosLocais++;
				}
			}
		}

		/* Zona critica da variavel "qtdPrimosGlobais" */
		/* Adicionamos na variavel global a qtd de primos encontrados no macrobloco*/
		pthread_mutex_lock(&mutexQtdPrimos);
		qtdPrimosGlobais += qtdPrimosLocais;
		pthread_mutex_unlock(&mutexQtdPrimos);

		/*Zerando a quantidade de primos locais para contabilizar novamente no prox loop*/
		qtdPrimosLocais = 0;
	}
}

void f_buscaParalela()
{
	/*Criando um vetor de Threads de acordo com o tamanho pre-definido*/
	pthread_t thread[qtdThread];

	/*Inicializando os mutex para a protecao das zonas criticas (variaveis globais)*/
	pthread_mutex_init(&mutexPosicoes, NULL);
	pthread_mutex_init(&mutexQtdPrimos, NULL);
	pthread_mutex_init(&mutexQtdMacroBlocos, NULL);

	/* Criando e disparando as Threads */
	for (int i = 0; i < qtdThread; i++)
	{
		pthread_create(&thread[i], NULL, f_percorreMacroBlocos, NULL);
	}

	/* Verificando se elas terminaram antes de prosseguir com o codigo */
	for (int i = 0; i < qtdThread; i++)
	{
		pthread_join(thread[i], NULL);
	}

	/* Destruindo os mutex */
	pthread_mutex_destroy(&mutexPosicoes);
	pthread_mutex_destroy(&mutexQtdMacroBlocos);
	pthread_mutex_destroy(&mutexQtdPrimos);

}

int main()
{
	/* Com a matriz jah criada globalmente, adicionamos nela numeros aleatorios */
	tempoInicial = clock();
	f_numAleat();
	tempoFinal = clock();
	tempoTotal = tempoFinal - tempoInicial;
	tempoTotal = ((float)tempoTotal) / CLOCKS_PER_SEC;
	printf("Tempo total de preenchimento da matriz: %.4f segundos\n\n", tempoTotal);

	/* --- BUSCA SERIAL --- */
	/* Chamada da funcao serial */
	tempoInicial = clock();
	f_buscaSerial();
	tempoFinal = clock();
	tempoTotal = tempoFinal - tempoInicial;
	tempoTotal = ((float)tempoTotal) / CLOCKS_PER_SEC;
	printf("RESULTADOS DA BUSCA SERIAL\n");
	printf("Total de numeros primos encontrados: %d\n", qtdPrimosGlobais);
	printf("Tempo total: %.4f segundos\n\n", tempoTotal);
	/* --- FIM BUSCA FINAL --- */

	/*Resetando a qtd de numeros primos para a buscaParalela*/
	qtdPrimosGlobais = 0;

	//f_printMatriz();

	/* --- BUSCA PARALELA --- */
	tempoInicial = clock();
	/* Chamada da funcao paralela*/
	f_buscaParalela();
	tempoFinal = clock();
	tempoTotal = tempoFinal - tempoInicial;
	tempoTotal = ((float)tempoTotal) / CLOCKS_PER_SEC;
	printf("RESULTADOS DA BUSCA PARALELA\n");
	printf("Total de numeros primos encontrados: %d\n", qtdPrimosGlobais);
	printf("Tempo total: %.4f segundos\n", tempoTotal);
	/* --- FIM BUSCA PARALELA --- */

	printf("\n");
	system("pause");
	return 0;
}