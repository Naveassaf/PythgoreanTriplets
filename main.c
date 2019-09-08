/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex3
Description:	The main module contains three functions. The first is the notorious main - which here calls true_main and returns its return val.
				The second is the true main which deals with creation and timing of threads along with error handling and memory allocation and freeing.
				The last is a help function which the main uses to free and close used resources. All more specific desription may be found in functions.
*/

//includes
#include "FileIO.h"

//Constants
#define MAX_THREADS_PER_WAIT 64

//Function prototypes
int true_main(int argc, char *argv[]);
int clean_exit(char state, int num_of_threads, int buffer_size, HANDLE *triplet_handles, HANDLE *sort_handle, char *out_path, int threads_created, int num_limit);

//Function Implementations
int main(int argc, char *argv[])
{
	/*
	Function: main

	Inputs: int argc - number of arguments provided in command line call
			char* argv[] - array of char* containing the command line arguments provided

	Outputs: return value = FUNC_SUCCESS (0) for successful run, FUNC_FAIL (-1) for unsuccessful run

	Functionality: calls true main and returnes its return value...
	*/
	return true_main(argc, argv);
}

int true_main(int argc, char *argv[])
{
	/*
	Function: true_main

	Inputs: int argc - number of arguments provided in command line call
			char* argv[] - array of char* containing the command line arguments provided

	Outputs: return value = FUNC_SUCCESS (0) for successful run, FUNC_FAIL (-1) for unsuccessful run

	Functionality: The main function of this project orchestrates the functioning, creation, error handling, and "rendezvous" of the differenct threads.
					1) It allocates memory and initializes the output buffer and mutex array to be used for shared resource protection
					2) It allocated memory and creates the triplet generating threads and sorting thread
					3) Waits for all threads to finish running and executes writing of file
					4) Frees all dynamic memory and closes all creates handles
					5) Returns value representing successful/unsuccessful run.
	*/
	int num_limit, buffer_size, num_of_threads, out_len;
	int i, j, cur_min = 0, cur_max = 0;
	char* output_path = NULL;
	HANDLE *triplet_thread_handles = NULL, *sort_thread_handle = NULL, temp_handles[MAX_THREADS_PER_WAIT];
	DWORD thread_id;
	sort_params inputs;

	//global linked list's head (to be used for sorting)
	ll_head = NULL;
	done_triplet_generation = 0;

	//ensure correct number or arguments provided
	if (5 != argc)
	{
		printf("Wrong number of arguments provided: %d instead of 4", argc - 1);
		return FUNC_FAIL;
	}
	
	//Reading in and casting of cmd line arguments. Assumes correct input given correct num of arguments.
	num_limit = atoi(argv[1]);
	num_of_threads = atoi(argv[2]);
	buffer_size = atoi(argv[3]);
	out_len = strlen(argv[4]);
	if (NULL == (output_path = (char*)malloc(sizeof(char)*out_len+1)))
	{
		printf("Memory allocation failed!");
		return FUNC_FAIL;
	}
	strncpy(output_path, argv[4], out_len+1);
	inputs.buffer_size = buffer_size;
	inputs.num_limit = num_limit;

	//initialization of output buffer
	if (FUNC_SUCCESS != initialize_buffer(buffer_size))
	{
		clean_exit('G', num_of_threads, buffer_size, triplet_thread_handles, sort_thread_handle, output_path,0, num_limit);
		printf("Memory allocation failed!");
		return FUNC_FAIL;
	}

	//initialization of mutex_array and used array
	if (FUNC_SUCCESS != initialize_mutex_array(num_limit))
	{
		clean_exit('F', num_of_threads, buffer_size, triplet_thread_handles, sort_thread_handle, output_path, 0, num_limit);
		printf("Failed to create mutex");
		return FUNC_FAIL;
	}

	//Allocation and creation of triplet generation threads
	if (NULL == (triplet_thread_handles = (HANDLE*)malloc(sizeof(HANDLE)*num_of_threads)))
	{
		clean_exit('E', num_of_threads, buffer_size, triplet_thread_handles, sort_thread_handle, output_path, 0, num_limit);
		printf("Memory allocation failed!");
		return FUNC_FAIL;
	}
	for (i = 0; i < num_of_threads; i++)
	{
		triplet_thread_handles[i] = CreateThread(
			NULL,                                /* default security attributes */
			0,                                   /* use default stack size */
			FindTriplets,						 /* thread function */
			&inputs,							 /* argument to thread function  */
			0,                                   /* use default creation flags */
			&thread_id                           /* returns the thread identifier  */
		);

		if (NULL == triplet_thread_handles[i])
		{
			printf("Failed to create thread");
			clean_exit('C', num_of_threads, buffer_size, triplet_thread_handles, sort_thread_handle, output_path, i, num_limit);

		}
	}

	//Allocation and creating of sort_handle thread
	if (NULL == (sort_thread_handle = (HANDLE*)malloc(sizeof(HANDLE))))
	{
		clean_exit('C', num_of_threads, buffer_size, triplet_thread_handles, sort_thread_handle, output_path, num_of_threads, num_limit);
		printf("Memory allocation failed!");
		return FUNC_FAIL;
	}
		*sort_thread_handle = CreateThread(
		NULL,                                /* default security attributes */
		0,                                   /* use default stack size */
		sort,								 /* thread function */
		&inputs,							 /* argument to thread function  */
		0,                                   /* use default creation flags */
		&thread_id                           /* returns the thread identifier  */
		);
	if (NULL == sort_thread_handle)
	{
		printf("Failed to create sort thread");
		clean_exit('H', num_of_threads, buffer_size, triplet_thread_handles, sort_thread_handle, output_path, num_of_threads, num_limit);

	}

	//Wait upon triplet generating threads (in several iteration if more threads than MAX_THREADS_PER_WAIT)
	i = 0;
	while(1)
	{
		cur_min = i;
		if (cur_min + 63 < num_of_threads - 1)
			cur_max = i + 63;
		else
			cur_max = num_of_threads - 1;

		for (j = cur_min; j <= cur_max; j++)
			temp_handles[j - cur_min] = triplet_thread_handles[j];
		
		if (WAIT_OBJECT_0 != WaitForMultipleObjects(cur_max-cur_min+1, temp_handles, TRUE, INFINITE))
		{
			clean_exit('A', num_of_threads, buffer_size, triplet_thread_handles, sort_thread_handle, output_path, num_of_threads, num_limit);
			printf("Thread wait failed");
			return FUNC_FAIL;
		}

		i += 64;
		if (cur_max >= num_of_threads -1)
			break;
	}

	//Tells sorter to enter last round of checks before terminating
	done_triplet_generation = 1;

	//Wait for sort_thread to finish sorting and creating linked list from buffer
	if (WAIT_OBJECT_0 != WaitForSingleObject(*sort_thread_handle, INFINITE))
	{
		clean_exit('A', num_of_threads, buffer_size, triplet_thread_handles, sort_thread_handle, output_path, num_of_threads, num_limit);
		printf("Thread wait failed");
		return FUNC_FAIL;
	}

	//Write to file
	if (FUNC_SUCCESS != write_to_file(output_path))
	{
		printf("Error writing to output file");
		return FUNC_FAIL;
	}
	
	//Release all used memory and free handles
	clean_exit('A', num_of_threads, buffer_size, triplet_thread_handles, sort_thread_handle, output_path, num_of_threads, num_limit);
	
	return FUNC_SUCCESS;
}

int clean_exit(char state,int num_of_threads, int buffer_size, HANDLE *triplet_handles, HANDLE *sort_handle, char *out_path, int threads_created, int num_limit)
{
	/*
	Function: clean_exit

	Inputs: char state - represents the state of the program when exit was called, thus indicating
						 what resources and must be freed and closed (as to not close unopened handles fro ex.)
			ALL OTHER INPUTS - relevant information for releasing and closing resources
	
	Outputs: return value = FUNC_SUCCESS (0) for successful execution

	Functionality: Depending on the state of the program (of the main thread) closes and frees all allocated
					memory and opened Handles.
	*/
	int i = 0;
	switch (state)
	{
		case 'A': free_ll(ll_head);
		
		case 'B': CloseHandle(*sort_handle);

		case 'H':free(sort_handle);
		
		case 'C': 
			for (i = 0; i < threads_created; i++)
				CloseHandle(triplet_handles[i]);
			
		case 'D': free(triplet_handles);
	
		case 'E': free_mutex_array(num_limit);

		case 'F': free_output_buffer(buffer_size);
					CloseHandle(buffer_empty);
					CloseHandle(buffer_full);
	
		case 'G': free(out_path);
			
	}
	return FUNC_SUCCESS;

}
