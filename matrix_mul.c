#include <stdio.h>
#include <stdlib.h>
#include "funcs.h"
#include <unistd.h>
#include <math.h>
#include <mpi.h>
#include <stdbool.h>

int main(int argc, char *argv[]){
    int size, rank, N, P;
    double start_time, end_time;

    // MPI setup
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank==0 && argc==3){
        N = atoi(argv[1]);
        P = atoi(argv[2]);
    }
    else if (rank==0){
        printf("Insufficient/extra input provided!\n");
        exit(1);
    }

    // size of the submatrices
    int N_part;

    if (rank==0){
        // split the workload setup
        N_part = N / P;
    }

    // share the N, P, N_part values with other cores
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&P, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N_part, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // create submatrices, assign values, and send them to other cores
    // since we can't send pointers through MPI I will define C-style arrays
    // without dynamic memory allocation

    double A_array[P*N_part*N_part];
    double B_array[P*N_part*N_part];

    if (rank==0){
        // initialize matrices
        double ** A = INIT_MAT(N, true);    
        // wait for a second to make sure the random number generator is different
        sleep(1);
        double ** B = INIT_MAT(N, true);
        
        start_time = MPI_Wtime();
        
        // send values to other cores
        for (int destination_rank = 1; destination_rank < size; destination_rank++){
            // assign values to nonzero cores
            for (int i=0; i<P; i++){
                for (int j=0; j<N_part; j++){
                    for (int k=0; k<N_part; k++){
                        A_array[i*N_part*N_part+j*N_part+k] = A[destination_rank*N_part+j][i*N_part+k];
                    }
                }
            }

            for (int i=0; i<P; i++){
                for (int j=0; j<N_part; j++){
                    for (int k=0; k<N_part; k++){
                        B_array[i*N_part*N_part+j*N_part+k] = B[i*N_part+j][destination_rank*N_part+k];
                    }
                }
            }

            // send A_array
            MPI_Send(A_array, P*N_part*N_part, MPI_DOUBLE, destination_rank, destination_rank, MPI_COMM_WORLD);

            // send B_array
            MPI_Send(B_array, P*N_part*N_part, MPI_DOUBLE, destination_rank, destination_rank+size, MPI_COMM_WORLD);
        }

        // assign values to the arrays of core 0
        for (int i=0; i<P; i++){
            for (int j=0; j<N_part; j++){
                for (int k=0; k<N_part; k++){
                    A_array[i*N_part*N_part+j*N_part+k] = A[j][i*N_part+k];
                }
            }
        }

        for (int i=0; i<P; i++){
            for (int j=0; j<N_part; j++){
                for (int k=0; k<N_part; k++){
                    B_array[i*N_part*N_part+j*N_part+k] = B[i*N_part+j][k];
                }
            }
        }

        // free memory by getting rid of A, B
        DELETE_MAT(A,N);
        DELETE_MAT(B,N);
    }
    else{
        // receive A_array and B_array
        MPI_Recv(A_array, P*N_part*N_part, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(B_array, P*N_part*N_part, MPI_DOUBLE, 0, rank+size, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    // initialize the P submatrices of the final answer (column submatrices)
    double C_array[P*N_part*N_part];

    int count = 0;

    while (true){
        // multiplying matricess
        
        for (int i=0; i<N_part; i++){
            for (int j=0; j<N_part; j++){
                C_array[((count+rank)%P)*N_part*N_part+i*N_part+j] = 0;
                for (int k=0; k<N_part; k++){
                    for (int products=0; products<P; products++){
                        C_array[((count+rank)%P)*N_part*N_part+i*N_part+j] += A_array[products*N_part*N_part+i*N_part+k] * B_array[products*N_part*N_part+k*N_part+j];
                    }
                }
            }
        }

        count++;

        if (count==P){
            break;
        }

        // sending and receiving
        double A_array_new[P*N_part*N_part];
        if (rank % 2 == 0){
            MPI_Send(A_array, P*N_part*N_part, MPI_DOUBLE, (rank+size-1)%size, rank, MPI_COMM_WORLD);
            MPI_Recv(A_array_new, P*N_part*N_part, MPI_DOUBLE, (rank+1)%size, (rank+1)%size, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else{
            MPI_Recv(A_array_new, P*N_part*N_part, MPI_DOUBLE, (rank+1)%size, (rank+1)%size, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(A_array, P*N_part*N_part, MPI_DOUBLE, (rank+size-1)%size, rank, MPI_COMM_WORLD);
        }
  
        for (int i=0; i<P*N_part*N_part; i++){
            A_array[i] = A_array_new[i];
        }  
    }

    // gather all submatrices and print the result
    if (rank==0){
        double C[N*N];
        MPI_Gather(C_array, P*N_part*N_part, MPI_DOUBLE, C, P*N_part*N_part, MPI_DOUBLE, rank, MPI_COMM_WORLD);
        
        end_time = MPI_Wtime();

        printf("Total runtime %.08f.\n", end_time-start_time);
    }
    else{
        MPI_Gather(C_array, P*N_part*N_part, MPI_DOUBLE, NULL, 0, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    // finalize MPI
    MPI_Finalize();
}