/* Gauss Jordan Elimination, 												  */
/* Ngakan Putu Ariastu														  */
/*	get time serial = ~15s									                  */
/* 																		      */
/*  																		  */
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

/* Declare all function prototype                                             */
int MatrixNew(Matrix *m, int x, int y);
int VectorNew(Vector *m, int x);
void MVmult(Matrix *m, Vector *n, Vector *res);
void PrintMatrix(Matrix *m);
void PrintVector(Vector *m);


int main()
{
    int i,j,k,n;
    float A[20][20],c,x[10],sum=0.0;
    printf("\nEnter the order of matrix: ");
    scanf("%d",&n);
    printf("\nEnter the elements of augmented matrix row-wise:\n\n");
    for(i=1; i<=n; i++)
    {
        for(j=1; j<=(n+1); j++)
        {
            printf("A[%d][%d] : ", i,j);
            scanf("%f",&A[i][j]);
        }
    }
    for(j=1; j<=n; j++) /* loop for the generation of upper triangular matrix*/
    {
        for(i=1; i<=n; i++)
        {
            if(i>j)
            {
                c=A[i][j]/A[j][j];
                for(k=1; k<=n+1; k++)
                {
                    A[i][k]=A[i][k]-c*A[j][k];
                }
            }
        }
    }
    x[n]=A[n][n+1]/A[n][n];
    /* this loop is for backward substitution*/
    for(i=n-1; i>=1; i--)
    {
        sum=0;
        for(j=i+1; j<=n; j++)
        {
            sum=sum+A[i][j]*x[j];
        }
        x[i]=(A[i][n+1]-sum)/A[i][i];
    }
    printf("\nThe solution is: \n");
    for(i=1; i<=n; i++)
    {
        printf("\nx%d=%f\t",i,x[i]); /* x1, x2, x3 are the required solutions*/
    }
    return(0);
}