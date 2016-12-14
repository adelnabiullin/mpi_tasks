#include "mpi.h"
#include <ctime>
#include <algorithm>
#include <iostream>

using namespace std;

#define messageTag 1
#define readyTag 2
#define timeTag 3

int main() {
    int size, rank, count;
    MPI_Status status;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    srand(time(nullptr));

    clock_t start, finish;

    int *buffer; // buffer to receive msg into it

    int iter_count = 10000;
    
    if (rank == 0) {
        cout << "check PingPong:\n";
        int *msg;
        int tmp[] = { 0 };
        for (int i = 0; i < iter_count;) {
            msg = new int[++i];
            buffer = new int[i];
            for (int j = 0; j < i; ++j) {
                msg[j] = rand() % 60;
            }
            MPI_Send(tmp, 1, MPI_INT, 1, readyTag, MPI_COMM_WORLD);
            MPI_Recv(tmp, 1, MPI_INT, 1, readyTag, MPI_COMM_WORLD, &status);
            start = clock();
            MPI_Send(msg, i, MPI_INT, 1, messageTag, MPI_COMM_WORLD);
            MPI_Recv(buffer, i, MPI_INT, 1, messageTag, MPI_COMM_WORLD, &status);
            finish = clock();
            delete [] msg;
            delete [] buffer;
            cout << finish - start << ' ';
        }
    } else if (rank == 1) {
        int tmp[] = { 0 };
        for (int i = 0; i < iter_count;) {
            buffer = new int[++i];
            MPI_Recv(tmp, 1, MPI_INT, 0, readyTag, MPI_COMM_WORLD, &status);
            MPI_Send(tmp, 1, MPI_INT, 0, readyTag, MPI_COMM_WORLD);
            MPI_Recv(buffer, i, MPI_INT, 0, messageTag, MPI_COMM_WORLD, &status);
            MPI_Send(buffer, i, MPI_INT, 0, messageTag, MPI_COMM_WORLD);
            delete [] buffer;
        }
    }

    int target;
    if (rank == 0) {
        puts("\n");
        cout << "check PingPing:\n";
        target = 1;
    } else {
        target = 0;
    }
    int *msg;
    int times[2];
    for (int i = 0; i < iter_count;) {
        buffer = new int[++i];
        msg = new int[i];
        for (int j = 0; j < i; ++j) {
            msg[j] = rand() % 60;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        start = clock();
        MPI_Send(msg, i, MPI_INT, target, messageTag, MPI_COMM_WORLD);
        MPI_Recv(buffer, i, MPI_INT, target, messageTag, MPI_COMM_WORLD, &status);
        finish = clock();
        if (rank == 1) {
            times[0] = start, times[1] = finish;
            MPI_Send(times, 2, MPI_INT, target, timeTag, MPI_COMM_WORLD);
        } else {
            MPI_Recv(times, 2, MPI_INT, target, timeTag, MPI_COMM_WORLD, &status);
            cout << std::max((int)(finish - start), times[1] - times[0]) << ' ';
        }
        delete [] buffer;
        delete [] msg;
    }

    MPI_Finalize();
    return 0;
}
