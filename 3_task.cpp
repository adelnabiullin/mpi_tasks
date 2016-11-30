#include <mpi.h>
#include <cstring>

#define messageTag 1

int main() {
  int size, rank, count;
  MPI_Status status;
  MPI_Init(nullptr, nullptr);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if( size < 2 ) {
    if(rank == 0)
      printf("Error: at least two processes required, abort\n");

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
    return -1;
  }

  if (rank == 1) {
    char c_arr[] = "Hello there, i see you.";
    for (int i = 0; i < size; i++) {
      if (i != rank) {
        MPI_Send(
          c_arr, 
          sizeof(c_arr) / sizeof(c_arr[0]),
          MPI_CHAR,
          i,
          messageTag,
          MPI_COMM_WORLD);
      }
    }
  } else {
    MPI_Probe(1, messageTag, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_CHAR, &count);
    char *oneDimArr = new char[count];
    MPI_Recv(oneDimArr, count, MPI_CHAR, 1, messageTag, MPI_COMM_WORLD, &status);
    printf("process %d: %s\n", rank, oneDimArr);
    delete [] oneDimArr;
  }

  MPI_Finalize();
  return 0;
}
