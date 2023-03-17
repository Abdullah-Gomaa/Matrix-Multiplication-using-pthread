#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/time.h>
/************************************** Global Variables **********************************************/
int rowsA = 0;
int colsA = 0;
int rowsB = 0;
int colsB = 0;
int rowsC = 0;
int colsC = 0;
int** matrix_A;
int** matrix_B;
int** matrix_C;
struct args
{
    int row;
    int col;
};

/*************************************** Functions Prototypes *****************************************/
void freeMatrices(int** matrix_A, int** matrix_B, int** matrix_C);
void printMatrix(int **matrix, int rows, int cols);
/**************************************** Threads Functions *******************************************/
// Method 1: normal multiplication
void *normalMultiply()
{
    for(int i=0; i<rowsA; i++)
    {
        for (int j = 0; j < colsB; j++)
        {
            matrix_C[i][j] = 0;
            for (int k = 0; k < colsA; k++)
            {
                matrix_C[i][j] += matrix_A[i][k] * matrix_B[k][j];
            }
        }
    }
    pthread_exit(NULL);
}
/*****************************************************/
// Method 2: a thread per row
void *rowMultiply(void *arg)
{
    struct args *dim = arg;
    
    int i = dim -> row;
    for (int j = 0; j < colsB; j++)
    {
        matrix_C[i][j] = 0;
        for (int k = 0; k < colsA; k++)
        {
            matrix_C[i][j] += matrix_A[i][k] * matrix_B[k][j];
        }
    }
    pthread_exit(NULL);
}
/*****************************************************/
// Method 3: a thread per element
void *elementMultiply(void *arg)
{
    struct args *dim = arg;
    
    int i = dim -> row;
    int j = dim -> col;
    matrix_C[i][j] = 0;
    for (int k = 0; k < colsA; k++)
    {
        matrix_C[i][j] += matrix_A[i][k] * matrix_B[k][j];
    }
    pthread_exit(NULL);
}

/*************************************************** MAIN *********************************************/
int main(int argc, char* argv[])
{   
    // Init file names
    char* fileMatA = "a.txt";
    char* fileMatB = "b.txt";
    char* fileMatC_PM = "c_per_matrix.txt";
    char* fileMatC_PR = "c_per_row.txt";
    char* fileMatC_PE = "c_per_element.txt";
    char fileNames[5][50]; // 2D array to store the resulting file names
    char *temp[4];
    struct timeval stop, start;

    // Store args in temp array
    for(int i=0; i<4; i++)
        temp[i] = argv[i];

    // If entered args = 4 change the default file names
    if(argc == 4)
    {
        for (int i = 0; i < 3; i++) 
    {
        // create filenames with extension _per_matrix.txt, _per_row.txt, and _per_element.txt for element 3
        if (i == 2)
        {
            strcpy(fileNames[i], temp[i+1]);
            strcpy(fileNames[i+1], temp[i+1]);
            strcpy(fileNames[i+2], temp[i+1]);
            strcat(fileNames[i], "_per_matrix.txt");
            strcat(fileNames[i+1], "_per_row.txt");
            strcat(fileNames[i+2], "_per_element.txt");
        }
        else
        {
            // create filename with extension .txt for elements 1 and 2
            strcpy(fileNames[i], temp[i+1]);
            strcat(fileNames[i], ".txt");
        }
        
    }
    fileMatA = fileNames[0];
    fileMatB = fileNames[1];
    fileMatC_PM = fileNames[2];
    fileMatC_PR = fileNames[3];
    fileMatC_PE = fileNames[4];
    }
    
    
/********************************** Matrix A *********************************/
    
    // Open file to read matrix    
    FILE *fa = fopen(fileMatA, "r");
    if (fa == NULL)
    {
        printf("Error opening file %s.\n", fileMatA);
        exit(1);
    }
    // Read row and col from file
    fscanf(fa, "row=%d col=%d\n", &rowsA, &colsA);

    // Allocate memory for the matrixA
    matrix_A = (int**) malloc(rowsA * sizeof(int*));
    if(matrix_A == NULL)
    {
        printf("Memory Allocation FAILED!");
        return -1;
    }
    for (int i = 0; i < rowsA; i++) {
        matrix_A[i] = (int*) malloc(colsA * sizeof(int));
        if(matrix_A[i] == NULL)
        {
            printf("Memory Allocation FAILED!");
            free(matrix_A);
            return -1;
        }
    }

    // Read remaining numbers from file and store in matrix
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsA; j++) {
            fscanf(fa, "%d", &matrix_A[i][j]);
        }
    }
    fclose(fa);

    // Print the matrix
    printf("Matrix A:\n");
    printMatrix(matrix_A, rowsA, colsA);

/********************************** Matrix B *********************************/

    // Open file to read matrix
    FILE *fb = fopen(fileMatB, "r");
    if (fb == NULL)
    {
        printf("Error opening file %s.\n", fileMatB);
        free(matrix_A);
        exit(1);
    }

    // Read row and col from file
    fscanf(fb, "row=%d col=%d\n", &rowsB, &colsB);

    // Allocate memory for the matrixB
    matrix_B = (int**) malloc(rowsB * sizeof(int*));
    if(matrix_B == NULL)
    {
        printf("Memory Allocation FAILED!");
        free(matrix_A);
        return -1;
    }
    for (int i = 0; i < rowsB; i++) {
        matrix_B[i] = (int*) malloc(colsB * sizeof(int));
        if(matrix_B[i] == NULL)
        {
            printf("Memory Allocation FAILED!");
            free(matrix_A);
            free(matrix_B);
            return -1;
        }
    }

    // Read remaining numbers from file and store in matrix
    for (int i = 0; i < rowsB; i++) {
        for (int j = 0; j < colsB; j++) {
            fscanf(fb, "%d", &matrix_B[i][j]);
        }
    }
    fclose(fb);

    // Print the matrix
    printf("Matrix B:\n");
    printMatrix(matrix_B, rowsB, colsB);

/********************************** Matrix C *********************************/
    
    // Check dimension for suitable matrix multiplication
    if(colsA == rowsB)
    {
        rowsC = rowsA;
        colsC = colsB;
    }
    else
    {
        rowsC = 0;
        colsC = 0;
        printf("Dimension Error\n");
        free(matrix_A);
        free(matrix_B);
        return -2;
    }

    // Allocate memory for the matrixC
    matrix_C = (int**) malloc(rowsC * sizeof(int*));
    if(matrix_C == NULL)
    {
        printf("Memory Allocation FAILED!");
        free(matrix_A);
        free(matrix_B);
        return -1;
    }
    for (int i = 0; i < rowsC; i++) 
    {
        matrix_C[i] = (int*) malloc(colsC * sizeof(int));
        if(matrix_C[i] == NULL)
        {
            printf("Memory Allocation FAILED!");
            free(matrix_A);
            free(matrix_B);
            free(matrix_C);
            return -1;
        }
        for (int j = 0; j < colsC; j++)
        {
            matrix_C[i][j] = 0; // set all elements to zero
        }
    }

/********************************** Multiplication per matrix *********************************/

    // Normal method 
    pthread_t tid[rowsA][colsB];
    gettimeofday(&start, NULL); //start checking time
    //your code goes here
    pthread_create(&tid[0][0], NULL, normalMultiply, NULL);
    // Join all threads
    pthread_join(tid[0][0], NULL);
    gettimeofday(&stop, NULL); //end checking time

    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    

    // Print the result matrix
    printf("\nMatrix C (Normal):\n");
    printMatrix(matrix_C, rowsC, colsC);
    // Write matrixC to new text file
    FILE *fnorm = fopen(fileMatC_PM, "w");
    if (fnorm == NULL) 
    {
        printf("Error opening file %s.\n", fileMatC_PM);
        exit(1);
    }
    // Write matrixC to file
    fprintf(fnorm, "Method: A thread per matrix\n");
    fprintf(fnorm, "row=%d col=%d\n", rowsC, colsC);
    for (int i = 0; i < rowsC; i++) 
    {
        for (int j = 0; j < colsC; j++)
        {
            fprintf(fnorm, "%d ", matrix_C[i][j]);
        }
        fprintf(fnorm, "\n");
    }
    fclose(fnorm);

/********************************** Multiplication per row *********************************/

    struct args *dims;

    // Multiply matrices using row-wise method
    printf("\nRow-wise multiplication:\n");

    gettimeofday(&start, NULL); //start checking time
    for (int i = 0; i < rowsA; i++) 
    {
        dims = malloc(sizeof(struct args));
        dims->row = i;
        pthread_create(&tid[i][0], NULL, rowMultiply, dims);
    }
    // Join all threads
    for (int i = 0; i < rowsA; i++) {
        pthread_join(tid[i][0], NULL);
        free(dims);  
    }
    gettimeofday(&stop, NULL); //end checking time

    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    

    // Print the result matrix
    printf("\nMatrix C (Row-wise):\n");
    printMatrix(matrix_C, rowsC, colsC);

    // Write matrixC to new text file
    FILE *frow = fopen(fileMatC_PR, "w");
    if (frow == NULL) 
    {
        printf("Error opening file %s.\n", fileMatC_PR);
        exit(1);
    }
    // Write matrixC to file
    fprintf(frow, "Method: A thread per row\n");
    fprintf(frow, "row=%d col=%d\n", rowsC, colsC);
    for (int i = 0; i < rowsC; i++) 
    {
        for (int j = 0; j < colsC; j++)
        {
            fprintf(frow, "%d ", matrix_C[i][j]);
        }
        fprintf(frow, "\n");
    }
    fclose(frow);

/********************************** Multiplication per element *********************************/

    // Multiply matrices using element-by-element method
    printf("\nelement-by-element multiplication:\n");

    gettimeofday(&start, NULL); //start checking time
    //your code goes here
    for (int i = 0; i < rowsA; i++)
    {
        for (int j = 0; j < colsB; j++)
        {
            dims = malloc(sizeof(struct args));
            dims->row = i;
            dims->row = j;
            pthread_create(&tid[i][j], NULL, elementMultiply, dims);
        }
    }

    // Join all threads
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            pthread_join(tid[i][j], NULL);
            free(dims);
        }
    }
    gettimeofday(&stop, NULL); //end checking time

    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    
    

    // Print the result matrix
    printf("\nMatrix C (element):\n");
    printMatrix(matrix_C, rowsC, colsC);

    // Write matrixC to new text file
    FILE *felem = fopen(fileMatC_PE, "w");
    if (felem == NULL) 
    {
        printf("Error opening file %s.\n", fileMatC_PE);
        exit(1);
    }
    // Write matrixC to file
    fprintf(felem, "Method: A thread per element\n");
    fprintf(felem, "row=%d col=%d\n", rowsC, colsC);
    for (int i = 0; i < rowsC; i++) 
    {
        for (int j = 0; j < colsC; j++)
        {
            fprintf(felem, "%d ", matrix_C[i][j]);
        }
        fprintf(felem, "\n");
    }
    fclose(felem);
    freeMatrices(matrix_A, matrix_B, matrix_C);    
    return 0;
}
/*************************************** Functions Implementation *************************************/
void freeMatrices(int** matrix_A, int** matrix_B, int** matrix_C)
{
    // Free memory for matrixA
    for (int i = 0; i < rowsA; i++) {
        free(matrix_A[i]);
    }
    free(matrix_A);
    // Free memory for matrixB
    for (int i = 0; i < rowsB; i++) {
        free(matrix_B[i]);
    }
    free(matrix_B);
    // Free memory for matrixC
    for (int i = 0; i < rowsC; i++) {
        free(matrix_C[i]);
    }
    free(matrix_C);
}

void printMatrix(int **matrix, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}
