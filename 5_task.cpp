#include "mpi.h"
#include <stdlib.h>
#include <time.h>
#include <cmath>

#define rankTag 1
#define rowsTag 2
#define vectLenTag 3
#define resultTag 4

int main() {
    int size, rank, count;
    MPI_Status status;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (size < 2) {
        if (rank == 0)
            printf("Error: at least two processes required, abort\n");

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
        return -1;
    }

    if (rank == 0) {
        int A[5][5];
        srand(time(nullptr));
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j) {
                A[i][j] = rand() % 60;
                printf("%d ", (A[i][j]));
            }
            puts("");
        }
        int vect_len = sizeof(A[0]) / sizeof(A[0][0]);
        int row_count = ceil(vect_len / (double) (size - 1));
        int need_procs = ceil(vect_len / (double) row_count);

        int max_rank[] = { need_procs };
        for(int i = 1; i < size; ++i)
            MPI_Send(max_rank, 1, MPI_INT, i, rankTag, MPI_COMM_WORLD);

        int vect_len_pointer[] = { vect_len, row_count }; // here row_count = step
        for (int i = 1; i <= need_procs; ++i) {
            MPI_Send(vect_len_pointer,
                2,
                MPI_INT,
                i,
                vectLenTag,
                MPI_COMM_WORLD
                );
        }

        int rows_to_send = row_count;
        for (int i = 1; i <= need_procs; ++i) {
            if (i == need_procs) {
                rows_to_send = vect_len - row_count * (i - 1);
            }
            MPI_Send(A + (i - 1) * row_count,
                vect_len * rows_to_send,
                MPI_INT,
                i,
                rowsTag,
                MPI_COMM_WORLD
                );
        }

        int *result_arr = new int[vect_len];
        for (int i = 1; i <= need_procs; ++i) {
            MPI_Probe(i, resultTag, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &count);
            MPI_Recv(result_arr + (i - 1) * row_count,
                count,
                MPI_INT,
                i,
                resultTag,
                MPI_COMM_WORLD,
                &status
                );
        }

        printf("\nDiag vector:\n");
        for(int i = 0; i < vect_len; ++i) {
            printf("%d ", result_arr[i]);
        }
        puts("");
        delete [] result_arr;
        
    } else {
        int max_rank[1];
        MPI_Recv(max_rank, 1, MPI_INT, 0, rankTag, MPI_COMM_WORLD, &status);
        if (rank <= max_rank[0]) {
            int vect_len[2];
            MPI_Recv(vect_len, 2, MPI_INT, 0, vectLenTag, MPI_COMM_WORLD, &status);

            MPI_Probe(0, rowsTag, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &count);
            int *rows_arr = new int[count];
            MPI_Recv(rows_arr, count, MPI_INT, 0, rowsTag, MPI_COMM_WORLD, &status);
            printf("Process %d: rows were received from proc. 0\n", rank);

            int *diag_elems = new int[count / vect_len[0]];
            int j = 0;
            for (int i = 0; i < count; i += vect_len[0], ++j) {
                diag_elems[j] = rows_arr[(rank - 1) * vect_len[1] + j * vect_len[0] + j];
            }

            MPI_Send(diag_elems,
                count / vect_len[0],
                MPI_INT,
                0,
                resultTag,
                MPI_COMM_WORLD
                );

            delete [] rows_arr;
            delete [] diag_elems;
        }
    }

    MPI_Finalize();
    return 0;
}