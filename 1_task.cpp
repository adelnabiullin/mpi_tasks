#include <mpi.h>

int main() {
  int size, rank;
  MPI_Init(nullptr, nullptr);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0)
		printf("Total processes count = %d\n", size);

	printf("Rank in MPI_COMM_WORLD = %d\n", rank);

  MPI_Finalize();
  return 0;
}
