#include <mpi.h>
#include <stdlib.h>
#include <time.h>

#define arrayDataTag 1

int main() {
  int size, rank;
  MPI_Status status;
  MPI_Init(nullptr, nullptr);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (size < 2) {
    if (rank == 0)
      printf("Error: minimum two processes required instead of %d, abort\n", size);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
    return -1;
  }

  int a[10];

  if (rank == 0) {
    srand(time(nullptr));
    for (int i = 0; i < 10; i++)
      a[i] = rand();
    for (int i = 0; i < 10; i++)
      printf("%d ", a[i]);
    puts("");

    MPI_Send(a, 10, MPI_INT, 1, arrayDataTag, MPI_COMM_WORLD);
  } else if (rank == 1) {
    MPI_Recv(a, 10, MPI_INT, 0, arrayDataTag, MPI_COMM_WORLD, &status);
    for (int i = 0; i < 10; i++)
      printf("%d ", a[i]);
    puts("");
  }

  MPI_Finalize();
  return 0;
}
