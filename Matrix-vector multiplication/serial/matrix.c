/* Matrix Vector multiplication, serial version								  */
/* Ngakan Putu Ariastu														  */
/*	get time  = ~15s										                  */
/*  tested on Intel i7-3517u 4 core											  */
/******************************************************************************/

/* Source Code:                                                               */
/* Include all library we need                                                */  
#include <stdio.h>  
#include <stdlib.h>  
#include <windows.h> 

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
int MatrixNew(Matrix *m, int x, int y);
int VectorNew(Vector *m, int x);
void MVmult(Matrix *m, Vector *n, Vector *res);
void PrintMatrix(Matrix *m);
void PrintVector(Vector *m);

/* Declare all function prototype                                             */
#define NRM 10000		       /* number of rows in matrix */
#define NCM 10000              /* number of columns in matrix */
#define NRV 10000              /* number ofrow in vector */
volatile DWORD dwStart;

/* Begin the Main Function                                                    */
int main()
{
	Matrix Mat;
	Vector Vect;
	Vector Res;

	dwStart = GetTickCount();
	/* create new matrix and random data */
	if (MatrixNew(&Mat, NRM, NCM) != 1)
		printf("creating matrix fail\n");

	for (int i = 0; i < Mat.row; i++)
	{
		for (int j = 0; j <Mat.col; j++)
		{
			Mat.data[i*Mat.col + j] = rand();
		}
	}
	//PrintMatrix(&Mat);

	/* create new vector and random data */
	if (VectorNew(&Vect, NRV) != 1)
		printf("creating vect fail\n");

	for (int i = 0; i < Vect.row; i++)
	{
		Vect.data[i] = rand();
	}
	//PrintVector(&Vect);

	/* calculate the multiplication result */
	if (VectorNew(&Res, NRM) != 1)
		printf("creating vect to store result fail\n");
	MVmult(&Mat, &Vect, &Res);

	//	PrintVector(&Res);

	/*dont forget to clear memory used */
	free(Mat.data);
	Mat.data = NULL;
	free(Vect.data);
	Vect.data = NULL;
	free(Res.data);
	Res.data = NULL;
	printf("time taken, %d milliseconds\n", GetTickCount() - dwStart);
	/* End Main Function                                                          */
}

/* function implementation                                                        */

int MatrixNew(Matrix *m, int x, int y)
{
	m->row = x;
	m->col = y;
	m->data = (int*)calloc(m->row*m->col, sizeof(int));

	if (m->data)
		return 1;
	else
		return 0;
}

int VectorNew(Vector *m, int x)
{
	m->row = x;
	m->data = (int*)calloc(m->row, sizeof(int));

	if (m->data)
		return 1;
	else
		return 0;
}

void MVmult(Matrix *m, Vector *n, Vector *res)
{
	int i, j;

	for (i = 0; i < m->row; i++)
	{
		for (j = 0; j < m->col; j++)
		{
			res->data[i] += m->data[i*m->col + j] * n->data[j];
		}
	}

}

void PrintMatrix(Matrix *m)
{
	int i, j;
	for (i = 0; i < m->row; i++)
	{
		for (j = 0; j < m->col; j++)
		{
			printf("%d ", m->data[i*m->col + j]);
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