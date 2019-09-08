/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex3
Description:	Header of TripletGenerator.c module. Including the Sorter.h, this header adds two
				static arrays used to time triplet generation. Further details on the module found in TripletGenerator.c file.
*/

#pragma once
#include "Sorter.h"

//Global Variables
static HANDLE mutex_array[MAX_NUMBER+1];
static int used[MAX_NUMBER+1];

//Function declarations
int initialize_mutex_array(int num_limit);
int HelpTriplets(int num_limit, int* param);
DWORD WINAPI FindTriplets(LPVOID lpparam);
int free_mutex_array(int num_limit);
