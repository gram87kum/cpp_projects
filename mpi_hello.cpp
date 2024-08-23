/*
Description: Hello world program for MPI. Program involves printing.
             "Hello world followed by process id used for printing"
Author: gram87k
Date: 8/23/2024
*/


#include <stdio.h>
#include <string.h>
#include "mpi.h"

main(int argc, char* argv[]){
    int my_rank; //rank of process
    int p; //number of processors
    int source; //rank of sender
    int dest; //rank of dest
    int tag = 0; // Tag - used to indicate what behavior to follow
    char message[100]; //Storage for message
    MPI_Status status; //Return status for receive

    MPI_Init(&argc, &argv); //Starting point of MPI program.
    //Args are pointers to main function arg for special setup if any

    //Find out process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    //Find out number of processes
    MPI_Comm_size(MPI_COMM_WORLD,&p);

    if(my_rank !=0){
        /*Create message*/
        sprintf(message, "Greetings from process %d!",my_rank)
        dest = 0;
        //Use strlen+1 to transmit 'null' EoM
        MPI_Send(message,strlen(message)+1,MPI_CHAR,dest,tag,MPI_COMM_WORLD);

    }
    else{
        for(source=1;source < p;source++){
            MPI_Recv(message,100,MPI_CHAR,source,tag,MPI_COMM_WORLD,&status);
            printf("%s\n",message);
        }

    }
    MPI_Finalize();
}
