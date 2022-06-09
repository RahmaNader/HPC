#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#define expected (long double)1.202056903159594

long double power(long double x, unsigned int y)
{
    long double temp;
    if( y == 0)
        return 1;
    temp = power(x, y / 2);
    if (y % 2 == 0)
        return temp * temp;
    else
        return x * temp * temp;
}

double serial(int n){
    clock_t t;
    t = clock();
    long double serialSum = 0;
    for(int j = 0;j<n;j++){
        serialSum += (long double)1/power(j,3);
    }
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
    return time_taken;
}

int main(int argc, char *argv[]) {
    int n;
    int start = 0, finish = 0;
    int i = 0;
    int numprocs;
    int rank;
    long double error = 0;
    long double glopalSum = 0;
    long double localSum = 0;
    double Tserial;
    double localStart;
    double localFinish;
    double localElapsed;
    double Tparallel;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
        scanf("%d",&n);
        Tserial = serial(n);
    }
    
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    localStart = MPI_Wtime();
    for ( i = rank + 1; i <= n ; i += numprocs) {
        localSum += (long double)1/power(i,3);
    }
    localFinish = MPI_Wtime();
    
    localElapsed = localFinish - localStart;

    MPI_Reduce(&localSum,&glopalSum,1, MPI_LONG_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&localElapsed, &Tparallel, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(rank==0){
        printf("summation is %0.15Lf \n",glopalSum);
        error = expected - glopalSum;
        printf("error is %0.15Lf \n",error);
        printf("Ts = %0.6f seconds\n", Tserial);
        printf("Tp = %0.6f seconds\n", Tparallel);
        double speedUp = Tserial/Tparallel;
        printf("speed up = Tserial/Tparallel = %0.6f\n",speedUp);
        printf("Efficiency  = Speed up/number of processes = %0.6f\n",speedUp/numprocs);

    }
    MPI_Finalize();
    return 0;
}

