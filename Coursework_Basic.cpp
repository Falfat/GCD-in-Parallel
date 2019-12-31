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
	srand(time(NULL) + id * 10);

	int tag_num = 1;


	//uncomment the code below for timing!!!!!!!!!!
#ifdef DO_TIMING
	//The timing starts here - I deliberately exclude the initialisation of MPI
	double tdif = -wTime();
#endif

	//dynamically allocate memory for variables for recieving data in processors other than 0
	int *recv_1 = new int[(10000 / p)];
	int *recv_2 = new int[(10000 / p)];
	//declare variable to store results
	int gcd_result[10000];


	//do the below in id = 0
	if (id == 0) {
		/* Begin to generate pair of random numbers*/

		//Declare the two lists to store pair of random numbers
		int rand_list1[10000];
		int rand_list2[10000];
		//fill the lists with random numbers between 1 and 1000
		for (int i = 0; i < 10000; i++) {
			rand_list1[i] = 1 + rand() % 200;
			rand_list2[i] = 1 + rand() % 200;
		}
		/* End of generating pair of random numbers*/

		//dynamically allocate memory for max requests
		MPI_Request* request = new MPI_Request[2 * (p - 1)];
		//Send data


		//loop to send a chunk (10000/p) of data to all other processors other than 0.
		for (int i = 1; i < p; i++) {
			MPI_Isend(&rand_list1[(10000 / p) * i], (10000 / p), MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[i - 1]);
			MPI_Isend(&rand_list2[(10000 / p) * i], (10000 / p), MPI_INT, i, tag_num, MPI_COMM_WORLD, &request[(i - 1) + (p - 1)]);//1 is the number of process (p)
		}

		//wait for data to be sent.
		MPI_Waitall(2 * (p - 1), request, MPI_STATUS_IGNORE);


		//set up recieve.
		for (int i = 0; i < (10000 / p); i++)
		{
			recv_1[i] = rand_list1[i];
			recv_2[i] = rand_list2[i];
		}

		//delete pointer to request.
		delete[] request;

	}



	//do the following on all processors other than zero
	else
	{
		//recieve data (10000/p) from processor 0
		MPI_Recv(recv_1, (10000 / p), MPI_INT, 0, tag_num, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(recv_2, (10000 / p), MPI_INT, 0, tag_num, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//when we send data
	}


	//dynamically allocate memory to store results of gcd 
	int *gcd_array = new int[(10000 / p)];

	//calaculate gcd for each pair of data received
	for (int n = 0; n < (10000 / p); n++)
	{
		int x = recv_1[n];
		int y = recv_2[n];

		//store result in the array
		gcd_array[n] = gcd(x, y);
	}

	//sent to processor 0 from all other processors
	if (id != 0) {
		MPI_Send(gcd_array, (10000 / p), MPI_INT, 0, tag_num, MPI_COMM_WORLD);

	}

	//do some work in processor 0 on its chunk of data
	else {


		for (int k = 0; k < (10000 / p); k++) {
			gcd_result[k] = gcd_array[k];
		}

		//dynamicaaly allocate memory for request.
		MPI_Request* request = new MPI_Request[p - 1];

		//recieve from all other processors.
		for (int k = 1; k < p; k++) {

			MPI_Irecv(&gcd_result[(10000 / p)*k], (10000 / p), MPI_INT, k, tag_num, MPI_COMM_WORLD, &request[p - 1]);

		}
		//wait to finsih recieving.
		MPI_Waitall(p - 1, request, MPI_STATUS_IGNORE);
	}

//#ifdef DO_TIMING
		//tdif += wTime();
		if (id == 0) {
			//cout << setprecision(5);
			//cout << "The code took " << tdif << "s to run" << endl;
			//print result of first 120 pairs of random numbers
	for (int i = 0; i < 120; i++) {
		cout << "The greatest common divisor of " << recv_1[i] << " and " << recv_2[i] << " is " << gcd_result[i] << endl; //print first 100 elemnts in  a
	}
	cout.flush();


	}
//#endif

	delete[] recv_1;
	delete[] recv_2;
	MPI_Finalize();

}