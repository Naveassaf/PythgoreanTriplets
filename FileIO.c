/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex3
Description:	This file contains two functions, the first prints the primitive triplets to the output file and the second frees the triplets linked list.
*/


//Includes 
#include "FileIO.h"

//Function implementations
int write_to_file(char* out_path)
{
	/*
	Function:		write_to_file
	Inputs:			out_path - Path to output file.
	Outputs:		return value = FUNC_SUCSSES (Or FUNC_FAIL if facing problem to open output file).
	Functionality:	The function operates on the sorted linked list contains the primitives triplets and print the the triplets (a,b,c) to the output file, seperated by line.
	*/

	int a = 0, b = 0, c = 0, n, m;
	FILE* f = NULL;
	sorter_node* temp = NULL;

	temp = ll_head;

	if (NULL == out_path)
		return FUNC_FAIL;

	f = fopen(out_path, "w");

	if (f == NULL)
		return FUNC_FAIL;

	while (temp != NULL)
	{
		m = temp->m;
		n = temp->n;
		a = m * m - n * n;
		b = 2 * m*n;
		c = m * m + n * n;

		//Only one "enter" on last line
		if (NULL == temp->next)
			fprintf(f, "%d,%d,%d\n", a, b, c);
		else
			fprintf(f, "%d,%d,%d\n\n", a, b, c);
		temp = temp->next;
	}
	fclose(f);
	return FUNC_SUCCESS;
}

int free_ll(sorter_node *head)
{
	/*
	Function:		free_ll
	Inputs:			sorter_node *head - pointer to triplets linked list head.
	Outputs:		return value = FUNC_SUCSSES.
	Functionality:	The function operates on the linked list contains the primitives triplets and frees it node by node.
	*/
	sorter_node *temp = NULL, *cur = NULL;

	if (NULL == head)
		return FUNC_SUCCESS;
	else
		cur = head;

	while (NULL != cur)
	{
		temp = cur;
		cur = cur->next;
		free(temp);
	}
	return FUNC_SUCCESS;
}