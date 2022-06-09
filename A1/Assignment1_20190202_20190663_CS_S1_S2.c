#include "mpi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH 100000
int grades2[MAX_LENGTH];
int names2[MAX_LENGTH];
int grades[MAX_LENGTH];
int names[MAX_LENGTH];

int main(int argc, char *argv[]) {
    int i;
    int count, partial_count;
    int number_of_elements;
    int avg_per_process;
    int avg_per_process_reference;
    int number_of_recieved;
    int start_index;
    int my_rank;
    int process;
    int source;
    int number_of_students;
    int student;
    int grade;
    int remainder;
    int track;
    int tag = 0;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process);
    
    if (my_rank != 0) {
        MPI_Recv(&avg_per_process_reference, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&names2, avg_per_process_reference, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&grades2, avg_per_process_reference, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

        number_of_recieved = avg_per_process_reference;
        partial_count = 0;
        for (i = 0; i < number_of_recieved; i++) {
            if (grades2[i] >= 60) {
                partial_count++;
                printf("%d Passed The Exam %d\n", names2[i],my_rank);
            } else {
                printf("%d Failed. Please Repeat The Exam %d\n", names2[i],my_rank);
            }
        }
        MPI_Send(&partial_count, 1, MPI_INT,0, tag, MPI_COMM_WORLD);
    } else {
        FILE *ptr;
        ptr = fopen("grades.txt", "r");

        if (NULL == ptr) {
            printf("file can't be opened \n");
        }
        number_of_students = 0;
        while (fscanf(ptr, "%d %d", &student, &grade) == 2) {
            names[number_of_students] = student;
            grades[number_of_students] = grade;
            number_of_students++;
        }
        fclose(ptr);
        if (number_of_students > MAX_LENGTH) {
            printf("Total number of students exceeded \n");
            exit(1);
        }
        
        avg_per_process = number_of_students / (process-1);
        remainder = number_of_students % (process-1);
        track = 0;

        for (source = 1; source < process; source++) {
            start_index = (source-1) * avg_per_process + track;
            if (remainder) {
                number_of_elements = avg_per_process + 1;
                remainder--;
                track++;
            } else number_of_elements = avg_per_process;

            MPI_Send(&number_of_elements, 1, MPI_INT, source, tag, MPI_COMM_WORLD);
            MPI_Send(&names[start_index], number_of_elements, MPI_INT, source, tag, MPI_COMM_WORLD);
            MPI_Send(&grades[start_index], number_of_elements, MPI_INT, source, tag, MPI_COMM_WORLD);
        }

        count = 0;
     
        for (i = 1; i < process; i++) {
            MPI_Recv(&partial_count, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
            count += partial_count;
        }
        printf("Total number of students passed the exam is %d out of %d\n", count,number_of_students);
    }

    MPI_Finalize();
}
