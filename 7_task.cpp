#include "mpi.h"

int main() {
    int size, rank;
    MPI_Status status;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Finalize();
    return 0;
}