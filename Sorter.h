/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex3
Description:	Header of Sorter.c module. THe "lowest level" header which gets included recursively by rest of 
				the headers in this solution. Further details on the module found in Sorter.c file.
*/


//Includes
#include <stdio.h>
#include <windows.h>
#pragma warning (disable : 4996)

//Constants
#define MAX_BUFFER_SIZE 100
#define MAX_NUMBER 1000
#define MAX_THREADS 100
#define FUNC_SUCCESS 0
#define FUNC_FAIL -1

//Type declarations
typedef struct _sorter_node_ {
	int n;
	int m;
	struct _sorter_node_* next;

}sorter_node;

typedef struct _buffer_node_ {
	int n,m,a,b,c;
	char empty, readable;
	HANDLE mutex;
}buffer_node;

typedef struct _sort_params_ {
	int buffer_size;
	int num_limit;
}sort_params;

//Global variables
buffer_node output_buffer[MAX_BUFFER_SIZE];
HANDLE buffer_full;
HANDLE buffer_empty;
int done_triplet_generation;
sorter_node *ll_head;
HANDLE num_mutex;

//Function declarations
int initialize_buffer(int buffer_size);
int insert_to_buffer(int n, int m, int buffer_size);
DWORD WINAPI sort(LPVOID lpparam);
int free_output_buffer(int buffer_size);
sorter_node* insert_to_list(sorter_node* head, sorter_node* insert);