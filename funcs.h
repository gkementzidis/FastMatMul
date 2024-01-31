#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

// initialize a matrix of size N with random values
double** INIT_MAT(int N, bool random){
    double** matrix = (double**)malloc(N * sizeof(double*));
    if (matrix == NULL) {
        fprintf(stderr, "Memory allocation failed for matrix\n");
        exit(1); 
    }
    for (int i = 0; i < N; i++) {
        matrix[i] = (double*)malloc(N * sizeof(double));
        if (matrix[i] == NULL) {
            fprintf(stderr, "Memory allocation failed for matrix row %d\n", i);
            exit(1);
        }
    }

    if (random==true){
        srand(time(NULL));
        for(int i = 0; i<N; i++){
            for(int j = 0; j<N; j++){
                double randomValue = (double)rand() / RAND_MAX;
                matrix[i][j] = 2.0*randomValue-1.0;
            }
        }
    }
    return matrix;
}

// free the memory allocated for the matrix
void DELETE_MAT(double ** mat, int N){
    for (int i = 0; i < N; i++) {
        free(mat[i]);
    }
    free(mat);
}

// print the matrix
void PRINT_MAT(double ** mat, int N){
    for (int i=0; i<N; i++){
        for (int j=0; j<N; j++){
            printf("%f ", mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// multiply two matrices
double ** MULTIPLY_MAT(double ** A, double ** B, int N){
    // multiplying two square matrices of equal size
    double** matrix = (double**)malloc(N * sizeof(double*));
    if (matrix == NULL) {
        fprintf(stderr, "Memory allocation failed for matrix\n");
        exit(1); 
    }
    for (int i = 0; i < N; i++) {
        matrix[i] = (double*)malloc(N * sizeof(double));
        if (matrix[i] == NULL) {
            fprintf(stderr, "Memory allocation failed for matrix row %d\n", i);
            exit(1);
        }
    }

    for(int i = 0; i<N; i++){
        for(int j = 0; j<N; j++){
            double sum = 0;
            for (int k = 0; k<N; k++){
                sum += A[i][k]*B[k][j];
            }
            matrix[i][j] = sum;
        }
    }
    return matrix;
}