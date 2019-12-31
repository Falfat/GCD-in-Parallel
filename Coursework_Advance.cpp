#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <iomanip>
using namespace std;

#define DO_TIMING

/*	
Declare variable for processor id and total number of cores.
	id :  processor id
	p: total number of cores
	
	*/
int id, p;

/*
	Function to calculate greatest common divisor OF two integers using the Euclid algorithm;
	The algorithm can be expressed as follows:
	The greatest common divisor of x and y is y if the remainder when x is divided by y is zero otherwise
	it is the greatest common divisor of y and the remainder when x is divided by y.
	Ref:(ACSE5, Mock exam question)
*/
int gcd(int x, int y) {
	//copy integer x into c
	int c = x;
	//copy integer y into d
	int d = y;
	//declare a variable to store gcd
	int gcd_no;



	if (x == 0 || y == 0) {
		//if x or y is zero, gcd is 1
		gcd_no = 1;
	}

	else if (x%y == 0) {
		//gcd is y if the remainder  of x/y is 0
		gcd_no = y;
	}


	else

	{
		//remainder when x is divided by y
		int rem = (x%y);

		//do while remainder is less than or greater than 0
		while (rem < 0 || rem > 0)

		{
			//copy y into x
			x = y;

			//let y equals rem
			y = rem;

			//find remainder
			rem = (x%y);

		}
		//gcd is y when rem =0
		gcd_no = y;

	}


	//return gcd

	return gcd_no;



}

/*
	Function for timing
*/
double wTime()
{
	return (double)clock() / CLOCKS_PER_SEC;
}

int main(int argc, char *argv[])
{
	//setup MPI communications
	MPI_Init(&argc, &argv);
	//Reads the rank (number) of the current process
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	//Reads the total number of processes that have been assigned
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	//seeding for generating random values
	srand(time(NULL) + id * 1000);

//uncomment the code below for timing!!!!!!!!!!
/*#ifdef DO_TIMING
The timing starts here - I deliberately exclude the initialisation of MPI
	double tdif = -wTime();
#endif*/

	//declare tag number and flag
	int tag_num = 1, flag = 0;
	//dynamically allocate memory for variables for recieving data in processors other than 0
	int *recv_1 = new int[100];
	int *recv_2 = new int[100];

	//declare variable to store results
	int gcd_result[10000];

	//do the below in id = 0
	if (id == 0)
	{
		/* Begin to generate pair of random numbers*/

		//Declare the two lists to store pair of random numbers
		int rand_list1[10000];
		int rand_list2[10000];

		//fill the lists with random numbers between 1 and 1000
		for (int i = 0; i < 10000; i++) {
			rand_list1[i] = 1 + rand() % 1000;
			rand_list2[i] = 1 + rand() % 1000;
		}

		/* End of generating pair of random numbers*/

		//dynamically allocate memory for max requests
		MPI_Request* request = new MPI_Request[3*(p - 1)];

		//Declare varaiable "index" to track the number of data that has been sent and worked on.
		int index = 0;

		//loop to dend data to all other processors other than 0 and recieve from them.
		for (int i = 1; i < p; i++) {
			//send 100 pairs of the  data
			MPI_Isend(&rand_list1[index], 100, MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[i - 1]);
			MPI_Isend(&rand_list2[index], 100, MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[(i - 1) + (p - 1)]);
			//recieve data after gcd has been calculated
			MPI_Irecv(&gcd_result[index], 100, MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[i-1 + 2*(p - 1)]);
			//increment the index after each batch of data sending
			index += 100;
//#ifndef DO_TIMING
			cout << "Processsor " << id << " sent to processor " << i << endl;
//#endif
		}
		//wait to recieve 
		MPI_Waitall(p - 1, &request[2*(p - 1)], MPI_STATUS_IGNORE);

		do
		{	//loop over all other processors
			for (int i = 1; i < p; i++) 
			{
				//check if processor has finished processing the data sent to it before sending another
				if (MPI_Test(request, &flag, MPI_STATUS_IGNORE) == MPI_SUCCESS && flag == 0)
				{
					//if true, send another 100 pair of data to the processor
					MPI_Isend(&rand_list1[index], 100, MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[i - 1]);
					MPI_Isend(&rand_list2[index], 100, MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[i - 1]);
					//recieve data from the processor
					MPI_Irecv(&gcd_result[index], 100, MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[p - 1]);
					//increment the index
					index += 100;
				}
				
				//if not done, send empty data
				else 
				{
					MPI_Isend(nullptr, 0, MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[i - 1]);
				}
				
			}

			//while waiting, do some work on the zero processor
			for (int i = 0; i < 100; i++) {
				//find gcd
				gcd(rand_list1[index], rand_list2[index]);
				//increment data tracker
				index++;
			}
			
		} while (index < 10000);//do until all data are sent

		//wait for all receives 
		MPI_Waitall(p - 1, request, MPI_STATUS_IGNORE);

		//semd empty data
		for (int i = 1; i < p; i++)
			MPI_Isend(nullptr, 0, MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[i - 1]);

//#ifndef DO_TIMING
		//print result of first 120 pairs of random numbers
		for (int i = 0; i < 120; i++) {
			cout << "The greatest common divisor of " << rand_list1[i] << " and " << rand_list2[i] <<  " is " << gcd_result[i] << endl; //print first 100 elemnts in  a
		}
		//cout << endl;
		cout.flush();
//#endif
		//delete request's pointer
		delete[]request;
	}


	//do the following on all processors other than zero
	else
	{
		//declare request
		MPI_Request request;
		//dynamically allocate memory to store results of gcd 
		int *gcd_array = new int[100];
		//declare varaiable to measure the size of data recieve. to check it's not empty
		int num_recv;
		//do loop while data recieved is not empty
		do {
			//declare status
			MPI_Status status;
			//probe the data sent by processor 0
			MPI_Probe(0, tag_num, MPI_COMM_WORLD, &status);
			//fet count
			MPI_Get_count(&status, MPI_INT, &num_recv);
			//reiceve pairs of data
			MPI_Recv(recv_1, 100, MPI_INT, 0, tag_num, MPI_COMM_WORLD,MPI_STATUSES_IGNORE);
			MPI_Recv(recv_2, 100, MPI_INT, 0, tag_num, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
			
			//calaculate gcd for each pair of data
			for (int n = 0; n < 100; n++)
			{
				int x = recv_1[n];
				int y = recv_2[n];

				//store result in the array
				gcd_array[n] = gcd(x, y);
			}
			//send result to processor 0
			MPI_Isend(gcd_array, 100, MPI_INT, 0, tag_num, MPI_COMM_WORLD, &request);
			
			
		} while (num_recv > 0); //do when data recieved is not zero

		//delete pointer to gcd_array
		delete[] gcd_array;
	}
	
	
/*#ifdef DO_TIMING
	//Note that this should be done after a block in case process zero finishes quicker than the others
	//MPI_Waitall is blocking - Otherwise explicitly use MPI_Barrier
	tdif += wTime();
	if (id == 0)
	{
		cout << setprecision(5);
		cout << "The code took " << tdif << "s to run" << endl;
	}
#endif*/

	//delete pointers to rev_1 and recv_2
	delete[] recv_1;
	delete[] recv_2;

	//exit MPI gracefully
	MPI_Finalize();
}