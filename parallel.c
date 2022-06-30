#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define NUM 1000
#define MX_TYPE long double // tipo dos elementos da matriz
#define ID_TYPE long int // tipo dos índices da matriz

MPI_Status status;

MX_TYPE m1[NUM][NUM],m2[NUM][NUM],m3[NUM][NUM];

int main()
{

  ID_TYPE i, j, k;

  /* inicio inicialização MPI */

  int rank; 	/* RANK DOS PROCESSOS */
	int size;	/* NÚMERO DE PROCESSOS */
  int linhas, offset;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  /* fim da inicialização MPI */

// Master:
  if(rank==0){

    // criando e inicializando as matrizes 1, 2 e 3 (a matriz 3 inicializada com zeros):
    printf("\n*versão serial*\n\n");
    printf("criando as matrizes 1, 2 e 3...");

    printf("OK\n");

    // inicializando os valores da matriz 1 e 2 (utilizando os valores dos índices)
    printf("inicializando matrizes 1 e 2...");

    for(i = 0; i<NUM; i++){
      for(j = 0; j<NUM; j++){
        m1[i][j] = i;
        m2[i][j] = j;
      }
    }
    printf("OK\n");
  
  // Determinando número de linhas da matriz A que serão enviadas para cada processo
    linhas = NUM/size;
  // Variável offset que determina o início
    offset = 0;

    for (i=1; i <= size-1; i++)
    {
      // Enviando o offset da matriz A
      MPI_Send(&offset, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
      // Enviando o número de linhas
      MPI_Send(&linhas, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
      // Envia as linhas da matriz A para cada processo
      MPI_Send(&m1[offset][0], linhas*NUM, MPI_DOUBLE, i,1, MPI_COMM_WORLD);
      // Envia a matriz B
      MPI_Send(&m2, NUM*NUM, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
      
      // Offset is modified according to number of rows sent to each process
      offset = offset + linhas;
    }

    // // multiplicando matrizes 1 e 2, salvando resultado na matriz 3
    // for (ID_TYPE k = 0; k<NUM; k++) {
    //   for (ID_TYPE i = 0; i<linhas; i++) {
    //     for (ID_TYPE j = 0; j<NUM; j++)
    //       m3[i][k] += m1[i][j] * m2[j][k];
    //   }
    // }

    // Root process waits untill the each slave proces sent their calculated result with message tag 2
    for (int i = 1; i <= size-1; i++)
    {
      // Receive the offset of particular slave process
      MPI_Recv(&offset, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
      // Receive the number of rows that each slave process processed
      MPI_Recv(&linhas, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
      // Calculated rows of the each process will be stored int Matrix C according to their offset and
      // the processed number of rows
      MPI_Recv(&m3[offset][0], linhas*NUM, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, &status);
    }

    // VERIFICAR O RESULTADO

  }

// Processos escravos
  if (rank > 0) {

    // Slave process waits for the message buffers with tag 1, that Root process sent
    // Each process will receive and execute this separately on their processes

    // The slave process receives the offset value sent by root process
    MPI_Recv(&offset, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    // The slave process receives number of rows sent by root process  
    MPI_Recv(&linhas, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    // The slave process receives the sub portion of the Matrix A which assigned by Root 
    MPI_Recv(&m1, linhas*NUM, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
    // The slave process receives the Matrix B
    MPI_Recv(&m2, NUM*NUM, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);

    // Matrix multiplication

    // multiplicando matrizes 1 e 2, salvando resultado na matriz 3
    for (ID_TYPE k = 0; k<NUM; k++) {
      for (ID_TYPE i = 0; i<linhas; i++) {
        m3[i][k] = 0.0;
        for (ID_TYPE j = 0; j<NUM; j++)
          m3[i][k] += m1[i][j] * m2[j][k];
      }
    }

    // Calculated result will be sent back to Root process (process 0) with message tag 2
    
    // Offset will be sent to Root, which determines the starting point of the calculated
    // value in matrix C 
    MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    // Number of rows the process calculated will be sent to root process
    MPI_Send(&linhas, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    // Resulting matrix with calculated rows will be sent to root process
    MPI_Send(&m3, linhas*NUM, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
  }

  MPI_Finalize();

  return 0;
}
