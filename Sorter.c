/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex3
Description:	The sorter module contains all functions dealing with initialization and management of the output buffer.
				It also containg the functions necessary to create an linked list from the output buffer and maintaining it sorted correctly.
				The initialize_buffer, insert_to_buffer, and free_output_buffer function make, modify, and free the output buffer correspondigly.
				THe sort function - in short- reads from the output_buffer to the sorted linked list (creating the necessary nodes) and using 
				the insert_to_list help function.
*/


//Includes
#include "Sorter.h"

//Function implementations
int initialize_buffer(int buffer_size)
{
	/*
	Function: initialize_buffer

	Inputs: int buffer_size - provided through command line by user, indicates the size of the output buffer

	Outputs: return value = FUNC_SUCCESS (0) for successful run, FUNC_FAIL (-1) for unsuccessful run

	Functionality: iterating from 0 to buffer_size, this function creates an array of originally "opened"
					mutexes and an int array of zeros of the same size. Once done successfully initializing 
					these arrays it creates the buffer_full and buffer_empty semaphores used to time the
					writing and reading of the program from the output_buffer. These arrays and semaphores
					are global and thus not lost once the function returns.
	*/
	int i;
	
	for (i = 0; i < buffer_size; i++)
	{
		output_buffer[i].empty = 'T';
		output_buffer[i].mutex = CreateMutex(
			NULL,   /* default security attributes */
			FALSE,	/* don't lock mutex immediately */
			NULL);  /* un-named */


		if (NULL == output_buffer[i].mutex)
			return FUNC_FAIL;
		output_buffer[i].readable = 'F';
	}

	buffer_full = CreateSemaphore(
		NULL,			//Default sec.
		0,				//Init value
		buffer_size,	//Max value
		NULL);			//Unnamed

	if (NULL == buffer_full)
		return FUNC_FAIL;

	buffer_empty = CreateSemaphore(
		NULL,			//Default sec.
		buffer_size,	//Init value
		buffer_size,	//Max value
		NULL);

	if (NULL == buffer_empty)
		return FUNC_FAIL;

	return FUNC_SUCCESS;
}

int insert_to_buffer(int n, int m, int buffer_size)
{
	/*
	Function: insert_to_buffer

	Inputs: int n - In coherence with project guidlines, represents the smaller of the two parameters which define the triplet.
			int m - As stated above for n, but represents the larger of the two parameters (n<m).
			int buffer_size - size of output buffer.

	Outputs: return value = FUNC_SUCCESS (0) for successful run, FUNC_FAIL (-1) for unsuccessful run.
			 At the end of execution, the triplet represented by (n,m) is in the output buffer ready to be read by the sorter thread.

	Functionality: Given (n,m) this function inserts the corresponding node (containing a,b,c,n,m, readble, and empty parameters) into 
					the first "available" (meaning already read by sorter) node in the output buffer. It uses the
					mutex found in every node of the buffer to ensure only one thread writes to a specific node of the
					output array at a given time (mutex protects the "empty" field).
	*/
	int i, wait_result;
	char empty;

	for (i = 0; i < buffer_size; i++)
	{
		//Down on current buffer node
		if (WAIT_OBJECT_0 != WaitForSingleObject(output_buffer[i].mutex, INFINITE))
			return FUNC_FAIL;
	
		//Critical Section reads if the node is empty and may thus be written into
		empty = output_buffer[i].empty;
		if(empty == 'T')
			output_buffer[i].empty = 'F';
		////End

		//Mutex up
		if(FALSE == ReleaseMutex(output_buffer[i].mutex))
			return FUNC_FAIL;

		//If the node found was empty, its value was changed to full in critical region and now rest of values may be updated
		if (empty == 'T')
		{
			
			if (WAIT_OBJECT_0 != WaitForSingleObject(buffer_empty, INFINITE))
				return FUNC_FAIL;
			output_buffer[i].n = n;
			output_buffer[i].m = m;
			output_buffer[i].a = m * m - n * n;
			output_buffer[i].b = 2 * m*n;
			output_buffer[i].c = m * m + n * n;
			output_buffer[i].readable = 'T';
			break;
		}
		
		//ensures the loop continues until a place in the buffer was found
		if (i == buffer_size - 1)
			i = -1;
	}

	return FUNC_SUCCESS;
}

int free_output_buffer(int buffer_size)
{
	/*
	Function: free_output_buffer

	Inputs: int buffer_size - size of output buffer

	Outputs: return value = FUNC_SUCCESS (0) for successful run

	Functionality: The buffer is a global, none dynamically allocated array. This function releases the handle
					found in each node of the array.
	*/
	int i;

	for (i = 0; i < buffer_size; i++)
	{
		//Ensures mutex is released before closing to avoid unexpected behavior.
		ReleaseMutex(output_buffer[i].mutex);
		CloseHandle(output_buffer[i].mutex);
	}

	return FUNC_SUCCESS;
}

DWORD WINAPI sort(LPVOID lpparam)
{
	/*
	Function: sort

	Inputs: lpparam - passed through CreateThread on this function, this struct is a "sort_params" struct
						containing two fields - buffer_size (size of output_buffer) and num_limin (max_num
						passed through the command line).

	Outputs: return value = FUNC_SUCCESS (0) for successful run, FUNC_FAIL (-1) for unsuccessful run.
			using insert_to_list, this method creates the sorted linked_list to be written into the 
			output file provided in the command line call.

	Functionality: This function is the function which the sorter thread is create on. This function loops infinitely
					over the output buffer, searching for nodes which are marked "readable" (the field node.readable = 'T').
					Once it finds one of these node this "consumer" 
	*/
	sort_params* inputs = NULL;
	int buffer_size, num_limit, index=0 ,count=0;
	buffer_node cur_buf;
	sorter_node *head = NULL, *cur_linked = NULL, *previous = NULL;
	int wait_result, first = 1;
	char read = 'F';

	//Ensure proper input
	if (NULL == lpparam)
		return FUNC_FAIL;

	//Casting of input parameter
	inputs = (sort_params*)lpparam;
	buffer_size = inputs->buffer_size;
	num_limit = inputs->num_limit;

	//Endless loop (until producers are done) over the output_buffer searching for "readable" nodes
	while (1)
	{
		//Mutex down on current output buffer node's mutex
		if (WAIT_OBJECT_0 != WaitForSingleObject(output_buffer[index].mutex, INFINITE))
			return FUNC_FAIL;
		
		//Critical Section - checks if the node is readable
		cur_buf = output_buffer[index];
		read = cur_buf.readable;
		if (read == 'T')
			output_buffer[index].readable = 'F';
		////End

		//Mutex up
		if (FALSE == ReleaseMutex(output_buffer[index].mutex))
			return FUNC_FAIL;
		
		//If node contains readable data, insert it into linked list, sorted
		if (read == 'T')
		{
			//Semaphore buffer_full down (one less full node in output_buffer)
			if (WAIT_OBJECT_0 != WaitForSingleObject(buffer_full, INFINITE))
				return FUNC_FAIL;

			//Allocation of mem for new linked list node
			if (NULL == (cur_linked = (sorter_node*)malloc(sizeof(sorter_node))))
				return FUNC_FAIL;

			//Data reading from node
			cur_linked->n = cur_buf.n;
			cur_linked->m = cur_buf.m;
			
			if (first == 1)
			{
				//creation of LL for first node
				head = cur_linked;
				first = 0;
				cur_linked->next = NULL;
			}
			else
			{
				//Sorted addition of LL node if LL exists already
				head = insert_to_list(head, cur_linked);
			}

			//Only now will the triplet generator be able to write new data into this node
			output_buffer[index].empty = 'T';

			//Semaphore up on buffer_empty - there is one more empty node in the buffer
			if (FALSE == ReleaseSemaphore(buffer_empty, 1, NULL))
				return FUNC_FAIL;
		}

		//Ensures infinite loop on correct indices
		if (index == buffer_size - 1)
			index = 0;
		else
			index++;

		//Allows sorter to perform one last iteration over output buffer once all triplet generating threads are done
		if (count == buffer_size)
			break;
		if (done_triplet_generation == 1)
			count += 1;
	}
	
	//Sets global ll_head equal to the LL created in this function (LL is dynamically allocated and only pointer is copied).
	ll_head = head;
	return FUNC_SUCCESS;
}

sorter_node* insert_to_list(sorter_node* head, sorter_node* insert)
{
	/*
	Function: inset_to_list

	Inputs: sorter_node *head - current head of linked list (created in sorter)
			soreter_node *insert - node to be inserted into the linked list

	Outputs: sorter_node *head - pointer to (possibly new) head of linked list.

	Functionality: Iterating over the already existant linked list, this function compares the insert node's n and m
					values with each node until finding the proper place to insert the new node such that the LL is still 
					sorted. It then returnes the head of the linked list containing the new node.
	*/
	sorter_node *cur = head;
	sorter_node *prev = head;


	while (cur != NULL)
	{
		//found correct place for node in linked list
		if ((cur->n > insert->n) || ((cur->n == insert->n) && (cur->m > insert->m)))
		{
			//Inserting node at head of list
			if (head == cur)
			{
				insert->next = cur;
				head = insert;
			}
			//Inserting node in middle of list
			else
			{
				prev->next = insert;
				insert->next = cur;
			}	
			break;
		}
		else
		{
			prev = cur;
			cur = cur->next;
		}
	}
	//Inserting node at end of list
	if (cur == NULL)
	{
		prev->next = insert;
		insert->next = NULL;
	}

	return head;
}



