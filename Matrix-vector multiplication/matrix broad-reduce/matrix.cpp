/* Matrix Vector multiplication, modified parallel using MPI broadcat reduce  */
/* Ngakan Putu Ariastu														  */
/*	get time not using MPI = ~15s							                  */
/*  after using MPI		   = ~15s										      */
/*  tested on Intel i7-3517u 4 core											  */
/******************************************************************************/

/* Source Code:                                                               */
/* Include all library we need  											  */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

/* create data type for matrix and vector									  */
typedef struct Matrix {
	int row;
	int col;
	int* data;

}Matrix;

typedef struct Vector {
	int row;
	int* data;

}Vector;

/* Declare all function prototype                                             */
int VectorNew(Vector *m, int row);
int MatrixNew(Matrix *m, int row, int col);
void PrintMatrix(Matrix *m);
void PrintVector(Vector *m);

/* Declare all function prototype                                             */
#define NRM 10000		       /* number of rows in matrix */
#define NCM 10000              /* number of columns in matrix */
#define NRV 10000             /* number ofrow in vector */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */
volatile DWORD dwStart;

int main(int argc, char *argv[])
{
	
	int	numtasks,              /* number of tasks in partition */
		taskid,                /* a task identifier */
		avecol,				   /* used to determine rows sent to each worker */
		i, j, rc,				   /* misc */
		local_Res=0;
		
	/* vector used in every process */
	Vector Vect;			   
	Vector Res;			

	dwStart = GetTickCount();
	if (VectorNew(&Res, NRM) != 1)
	{
		printf("creating local main result on worker %d fail\n", taskid);
		MPI_Abort(MPI_COMM_WORLD, rc);
		exit(1);
	}


	/* Initializing MPI environment */
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	if (numtasks < 2) {
		printf("Need at least two MPI tasks. Quitting...\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
		exit(1);
	}

	/* dividing work per worker */
	avecol = NCM / 4;

	if (taskid == 0)
	{
		/* Initializing vector to broadcast */
		if (VectorNew(&Vect, NRV) != 1)
		{
			printf("creating local vect on worker %d fail\n",taskid);
			MPI_Abort(MPI_COMM_WORLD, rc);
			exit(1);
		}
		for (i = 0; i < Vect.row; i++)
		{
			Vect.data[i] = rand();
		}

		/* send vector to other worker */
		MPI_Bcast(Vect.data, Vect.row, MPI_INT, 0, MPI_COMM_WORLD);
		

		/* Create local main Matrix and Vector*/
		Matrix Mat;
		if (MatrixNew(&Mat, NRM, NCM) != 1)
		{
			printf("creating local main matrix on worker %d fail\n",taskid);
			MPI_Abort(MPI_COMM_WORLD, rc);
			exit(1);
		}

	
		/* Initializing local main matrix*/
		for (i = 0; i < Mat.row; i++)
		{
			for (j = 0; j <Mat.col; j++)
			{
				Mat.data[i*Mat.col + j] = rand();
			}
		}
		
		/* Start Caclculation together per row */
		for (i = 0; i < Mat.row; i++)
		{
			MPI_Bcast(Mat.data + Mat.col*i , Mat.col, MPI_INT, 0, MPI_COMM_WORLD);
			for (j = taskid*avecol; j < avecol*(taskid + 1); j++)
			{
				local_Res += Mat.data[j] * Vect.data[j];
			}
			MPI_Reduce(&local_Res, Res.data+i, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
			//printf("Result worker %d:%d\n",taskid,Res.data[i]);
			local_Res = 0;

		}
		/* dont forget to free memory */
		//PrintVector(&Res);
		free(Mat.data);
		free(Res.data);
		free(Vect.data);
		Mat.data = NULL;
		Res.data = NULL;
		Vect.data = NULL;
		printf_s("time taken, %d milliseconds\n", GetTickCount() - dwStart);
	}

	else 
	{
		/* Initializing vector to broadcast */
		if (VectorNew(&Vect, NRV) != 1)
		{
			printf("creating local vect at worker %d fail\n", taskid);
			MPI_Abort(MPI_COMM_WORLD, rc);
			exit(1);
		}

		/*Receive Vector broadcasted*/
		MPI_Bcast(Vect.data, Vect.row, MPI_INT, 0, MPI_COMM_WORLD);
		//printf("vector at worker %d :\n",taskid);
		//PrintVector(&Vect);

		/* local Matrix to store data from worker 0 */
		Matrix local_Mat;
		if (MatrixNew(&local_Mat, 1, NCM) != 1)
		{
			printf("creating local matrix on worker %d fail\n",taskid);
			MPI_Abort(MPI_COMM_WORLD, rc);
			exit(1);
		}

		/* Start Caclculation together per row */
		for (i = 0; i < NRM; i++)
		{
			MPI_Bcast(local_Mat.data, local_Mat.col, MPI_INT, 0, MPI_COMM_WORLD);
			for (j = taskid*avecol; j < avecol*(taskid + 1); j++)
			{
				local_Res += local_Mat.data[j] * Vect.data[j];
			}
			MPI_Reduce(&local_Res,Res.data+i, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
			local_Res = 0;
			
		}
		/* dont forget to free memory */
		free(local_Mat.data);
		free(Vect.data);
		Vect.data = NULL;
		local_Mat.data = NULL;
	}


	free(Res.data);
	Res.data = NULL;

	MPI_Finalize();

	

/* End Main Function                                                          */
}

/* function implementation                                                     */

int VectorNew(Vector *m, int row)
{
	m->row = row;
	m->data = (int*)calloc(m->row, sizeof(int));

	if (m->data)
		return 1;
	else
		return 0;
}


int MatrixNew(Matrix *m, int row, int col)
{
	m->row = row;
	m->col = col;
	m->data = (int*)calloc(m->row*m->col, sizeof(int));

	if (m->data)
		return 1;
	else
		return 0;
}


void PrintMatrix(Matrix *m)
{
	int i, j;
	for (i = 0; i < m->row; i++)
	{
		for (j = 0; j < m->col; j++)
		{
			printf("%d	", m->data[i*m->col + j]);
		}
		printf("\n");
	}
}

void PrintVector(Vector *m)
{
	int i;
	for (i = 0; i < m->row; i++)
	{
		printf("%d\n", m->data[i]);
	}
}