/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex3
Description:	This file contains 4 functions. Two of them (initialize_mutex_array and free_mutex_array) are initializing and frees the mutex array that is being used
to make sure each new thread is calculating the primitive triplets for different "N" (actually by protecting the critical section where we chang ethe used bits).
The function FindTriplets is using the auxiliary function HelpTriplets to find the primitive tripletes. This function is the function which the thread are created on.
*/

//Includes
#include "TripletGenerator.h"

//Function implementations
int initialize_mutex_array(int num_limit)
{
	/*
	Function:		initialize_mutex_array
	Inputs:			num_limit - the maximal M given.
	Outputs:		return value = FUNC_SUCSSES (Or FUNC_FAIL if facing problem to create one of the mutexes).
	Functionality:	The function creates an array contains "num_limit" mutex indexes. We are using this array to make sure that there's no situatuin where
	two threads are calculating the triplets for same N.
	*/
	int i;

	for (i = 1; i <= num_limit; i++)
	{
		used[i] = 0;
		mutex_array[i] = CreateMutex(
			NULL,   /* default security attributes */
			FALSE,	/* don't lock mutex immediately */
			NULL);  /* un-named */


		if (NULL == mutex_array[i])		// if createmutex() failed for one of the mutexes, the return value is FUNC_FAIL.
			return FUNC_FAIL;
	}

	return FUNC_SUCCESS;
}

int HelpTriplets(int num_limit, int buffer_size, int x)
{
	/*
	Function:		HelpTriplets
	Inputs:			num_limit - the maximal M given (from command line).
	buffer_size - the size of the output buffer (from command line).
	int x - auxiliary parameters that we're using for x (in order to be coherent with the rest of the naming conventions).
	Outputs:		return value = FUNC_SUCSSES (Or FUNC_FAIL if one of the functions: insert_to_buffer or ReleaseSemaphore is failes).
	Functionality:	The function find the relevant couples (N,M) for which there are exists primitive triplets and then calls insert_to_buffer function that finds empty place
	in the buffer to store the new struct (contains N,M,a,b,c and some more fields).
	*/
	int n, m;
	n = x;

	for (m = n + 1; m <= num_limit; m++)
	{
		if (((m%n == 0) && (n != 1)) || ((m % 2 == 0) && (n % 2 == 0)) || ((m % 2 == 1) && (n % 2 == 1)))	// Conditions for being Primitive Triplet: (1) N isn't a divider of M.
			continue;																					//										   (2) if M is odd, N is even. 
		else																							//			                               (3) if M is even, N is odd.
		{
			if (FUNC_SUCCESS != insert_to_buffer(n, m, buffer_size))
				return FUNC_FAIL;

			if (FALSE == ReleaseSemaphore(buffer_full, 1, NULL))
				return FUNC_FAIL;
		}
	}
	return FUNC_SUCCESS;
}

DWORD WINAPI FindTriplets(LPVOID lpparam)
{
	/*
	Function:		FindTriplets
	Inputs:			lpparam - struct that continas the buffer_size and the num_limit parametes.
	Outputs:		return value = FUNC_SUCSSES (Or FUNC_FAIL if one of the functions: WaitForSingleObject, ReleaseMutex or HelpTriplets failes).
	Functionality:	The function find the relevant couples (N,M) for which there are exists primitive triplets by using the auxiliary function: HelpTriplets.
	The function is the funcrtion which the threads are cerated on.
	There is an critical section where we change the elements in used[] and we protect it by using the mutex_array.
	*/
	DWORD wait_result;
	int num_limit, buffer_size;
	sort_params *inputs = NULL;
	int index = 1;
	int cur_used;

	inputs = (sort_params*)lpparam;
	num_limit = inputs->num_limit;
	buffer_size = inputs->buffer_size;

	for (index = 1; index < num_limit; index++)
	{
		if (used[index] == 1)
			continue;

		//uses mutexes from mutex array to protect used array values
		if (WAIT_OBJECT_0 != WaitForSingleObject(mutex_array[index], INFINITE))
			return FUNC_FAIL;

		// Critical Section
		cur_used = used[index];
		used[index] = 1;
		////

		if (FALSE == ReleaseMutex(mutex_array[index]))
			return FUNC_FAIL;

		//If the current index has not been used to calc triplets yet
		if (0 == cur_used)
		{
			if (FUNC_SUCCESS != HelpTriplets(num_limit, buffer_size, index))
				return FUNC_FAIL;
		}

	}

	return FUNC_SUCCESS;
}

int free_mutex_array(int num_limit)
{
	/*
	Function:		free_mutex_array
	Inputs:			num_limit - the maximal M given.
	Outputs:		return value = FUNC_SUCSSES.
	Functionality:	The function frees the mutexes array that has been used to protect the critical section in FindTriplet function and to make sure each new thread is calculating
	the triplets for different N. The function releasing the mutex and then close the handle (because it isn't dynamically allocated, we only need to release it
	and close the handle).
	*/
	int i;

	for (i = 1; i <= num_limit; i++)
	{
		//ReleaseMutex should return false. Either way, Release() ensures there is no unexpected behavior in closing 'locked' mutex
		ReleaseMutex(mutex_array[i]);
		CloseHandle(mutex_array[i]);
	}

	return FUNC_SUCCESS;;
}