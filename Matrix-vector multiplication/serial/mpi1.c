#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Matrix {
	int row;
	int col;
	int* data;

}Matrix;


int MatrixNew(Matrix *m, int x, int y)
{
	m->row = x;
	m->col = y;
	m->data = (int*)calloc(m->row*m->col , sizeof(int));

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


#define NRA 9		           /* number of rows in matrix A */
#define NCA 9                 /* number of columns in matrix A */
#define NCB 1                  /* number of columns in matrix B */
#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

int main(int argc, char *argv[])
{
	int	numtasks,              /* number of tasks in partition */
		taskid,                /* a task identifier */
		numworkers,            /* number of worker tasks */
		source,                /* task id of message source */
		dest,                  /* task id of message destination */
		mtype,                 /* message type */
		rows,                  /* rows of matrix A sent to each worker */
		averow, extra, offset, /* used to determine rows sent to each worker */
		i, j, k, rc;           /* misc */

	Matrix Mat, Vect, Res;
	
	if (MatrixNew(&Mat, NRA, NCA) != 1)
		printf("creating matrix fail\n");
	if (MatrixNew(&Vect, NCA, NCB) != 1)
		printf("creating vect fail\n");
	if (MatrixNew(&Res, NRA, NCB) != 1)
		printf("creating vect fail\n");
	//	double	a[NRA][NCA],           /* matrix A to be multiplied */
	//		b[NCA][NCB],           /* matrix B to be multiplied */
	//		c[NRA][NCB];           /* result matrix C */
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	if (numtasks < 2) {
		printf("Need at least two MPI tasks. Quitting...\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
		exit(1);
	}
	numworkers = numtasks - 1;


	/**************************** master task ************************************/
	if (taskid == MASTER)
	{
		printf("mpi_mm has started with %d tasks.\n", numtasks);
		printf("Initializing arrays...\n");
		for (i = 0; i < Mat.row; i++)
		{
			for (j = 0; j <Mat.col; j++)
			{
				Mat.data[i*Mat.col + j] = 1;
			}
		}
		PrintMatrix(&Mat);

		for (i = 0; i < Vect.row; i++)
		{
			for (j = 0; j <Vect.col; j++)
			{
				Vect.data[i*Vect.col + j] =0;
			}
		}
		PrintMatrix(&Vect);


		/* Send matrix data to the worker tasks */
		averow = NRA / numworkers;
		extra = NRA%numworkers;
		offset = 0;
		mtype = FROM_MASTER;
		for (dest = 1; dest <= numworkers; dest++)
		{
			rows = (dest <= extra) ? averow + 1 : averow;
			printf("Sending %d rows to task %d offset=%d\n", rows, dest, offset);
			MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&Mat.data[offset*Mat.col + 0], rows*NCA, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&Vect.data, NCA*NCB, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
			offset = offset + rows;
		}

		/* Receive results from worker tasks */
		mtype = FROM_WORKER;
		for (i = 1; i <= numworkers; i++)
		{
			source = i;
			MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
			MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
			MPI_Recv(&Res.data[offset*Res.col + 0], rows*NCB, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
			printf("Received results from task %d\n", source);
		}

		/* Print results */
		printf("******************************************************\n");
		printf("Result Matrix:\n");
		PrintMatrix(&Res);
		printf("\n******************************************************\n");
		printf("Done.\n");

	}


	/**************************** worker task ************************************/
	if (taskid > MASTER)
	{
		mtype = FROM_MASTER;
		MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(Mat.data, rows*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(Vect.data, NCA*NCB, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);

		for (k = 0; k<NCB; k++)
			for (i = 0; i<rows; i++)
			{
				Res.data[i*Res.col + k] = 0;
				for (j = 0; j<NCA; j++)
					Res.data[i*Res.col + k] += Mat.data[i*Mat.col + j] * Vect.data[j*Vect.col + k];
			}
		mtype = FROM_WORKER;
		MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
		MPI_Send(&Res.data, rows*NCB, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	
}