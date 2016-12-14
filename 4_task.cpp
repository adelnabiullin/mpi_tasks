#include <mpi.h>
#include <cmath>

#define xTag 1
#define yTag 2
#define sumTag 3
#define multTag 4
#define rankTag 5

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
        int x[] = { 1, 2, 3, 4, 5 };
        int y[] = { 8, 7, 6, 5, 4 };
        // z_sum = { 9, 9, 9, 9, 9 }
        // z_mult = { 8, 14, 18, 20, 20 }
        
        int vect_size = sizeof(x) / sizeof(x[0]);
        int block_size = ceil(vect_size / (double)(size - 1));
        int need_procs = ceil(vect_size / (double) block_size);
        int max_rank[] = { need_procs };
        for(int i = 1; i < size; ++i)
            MPI_Send(max_rank, 1, MPI_INT, i, rankTag, MPI_COMM_WORLD);

        int* z_sum = new int[vect_size];
        int* z_mult = new int[vect_size];

        for(int i = 0, proc_num = 1; i < vect_size; ++proc_num) {
            if (block_size > vect_size - i)
                block_size = vect_size - i;

            int* x_block = new int[block_size];
            int* y_block = new int[block_size];
            for(int j = 0; j < block_size; ++j, ++i) {
                x_block[j] = x[i];
                y_block[j] = y[i];
            }
            MPI_Send(
                x_block,
                block_size,
                MPI_INT,
                proc_num,
                xTag,
                MPI_COMM_WORLD
                );
            printf("Process 0: x-block was sent to proc. %d\n", proc_num);
            MPI_Send(
                y_block,
                block_size,
                MPI_INT,
                proc_num,
                yTag,
                MPI_COMM_WORLD
                );
            printf("Process 0: y-block was sent to proc. %d\n", proc_num);
            delete [] x_block;
            delete [] y_block;
        }
        block_size = ceil(vect_size / (double)(size - 1));
        for(int i = 1; i <= need_procs; ++i) {
            // get block with sum from proc. i
            MPI_Probe(i, sumTag, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &count);
            MPI_Recv(
                z_sum + block_size * (i - 1),
                count,
                MPI_INT,
                i,
                sumTag,
                MPI_COMM_WORLD,
                &status);
            printf("Process 0: sum-block was received from proc. %d\n", i);

            // get block with multiplication result from proc. i
            MPI_Probe(i, multTag, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &count);
            MPI_Recv(
                z_mult + block_size * (i - 1),
                count,
                MPI_INT,
                i,
                multTag,
                MPI_COMM_WORLD,
                &status);
            printf("Process 0: mult-block was received from proc. %d\n", i);
        }
        printf("Vector x:\n");
        for(int i = 0; i < vect_size; ++i) {
            printf("%d ", x[i]);
        }
        printf("\nVector y:\n");
        for(int i = 0; i < vect_size; ++i) {
            printf("%d ", y[i]);
        }
        printf("\nVector x+y:\n");
        for(int i = 0; i < vect_size; ++i) {
            printf("%d ", z_sum[i]);
        }
        printf("\nVector x*y:\n");
        for(int i = 0; i < vect_size; ++i) {
            printf("%d ", z_mult[i]);
        }
        puts("");
        delete [] z_sum;
        delete [] z_mult;
    } else {
        int max_rank[1];
        MPI_Recv(max_rank, 1, MPI_INT, 0, rankTag, MPI_COMM_WORLD, &status);
        if (rank <= max_rank[0]) {

            // catch block of 'x' elems
            MPI_Probe(0, xTag, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &count);
            int *x_arr = new int[count];
            MPI_Recv(x_arr, count, MPI_INT, 0, xTag, MPI_COMM_WORLD, &status);
            printf("Process %d: x-block was received from proc. 0\n", rank);

            // catch block of 'y' elems
            int *y_arr = new int[count];
            MPI_Recv(y_arr, count, MPI_INT, 0, yTag, MPI_COMM_WORLD, &status);
            printf("Process %d: y-block was received from proc. 0\n", rank);

            int *z_sum = new int[count];
            int *z_mult = new int[count];
            for(int i = 0; i < count; ++i) {
                z_sum[i] = x_arr[i] + y_arr[i];
                z_mult[i] = x_arr[i] * y_arr[i];
            }

            // send back computed blocks
            MPI_Send(z_sum, count, MPI_INT, 0, sumTag, MPI_COMM_WORLD);
            printf("Process %d: sum-block was sent to proc. 0\n", rank);
            MPI_Send(z_mult, count, MPI_INT, 0, multTag, MPI_COMM_WORLD);
            printf("Process %d: mult-block was sent to proc. 0\n", rank);

            delete [] x_arr;
            delete [] y_arr;
            delete [] z_sum;
            delete [] z_mult;
        }
    }

    MPI_Finalize();
    return 0;
}
