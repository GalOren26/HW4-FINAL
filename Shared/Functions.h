  
#include <stdio.h>
#include <stdlib.h>
#ifndef __Hard_coded_data 
#define __Hard_coded_data 
#include "HardCodedData.h"
#include <Windows.h>

#endif

void CheakArgs(int argc,int excepted_num_of_args);
/*This function cheak if string is a number - used for validate the value of key in this code */
BOOL CheakIsAnumber(char* str);
/*input: pointer
checks if the pointer is valid. if not it exists with the relevant error.
*/
int valid_PTR(void*);

//------------------files--------------------

/*the function is a wrap of the createfileA function it gets as an input the file name and the mode,
it returns the handle of the opened file*/
int OpenFileWrap(LPCSTR str, DWORD mode, HANDLE* OUT hFile);

/*A wrap to the readfilefunction, gets as an input Handle of a file, it's length and the buffer as well
if it does not manage to read it exits with the last error*/
int  ReadFileWrap(DWORD len, HANDLE file, char* my_file_buff, DWORD* NumberOfBytesRead);

// set EOF at the end of the input file 
int SetEndOfFileWarp(LPCSTR output_path, uli end_pos, DWORD mode);

/*A wrap for the setfilepointer function. the function gets as an input the handle of the file,
and it moves the file pointer by "pos" bytes acording to the mode given as an input */
// the function can return the current file poistion in out paramter PositionAfterSet .
//if not needed PositionAfterSet set it to None 
int SetFilePointerWrap(HANDLE input_file, uli DistanceToMove, DWORD FromWhereToMove, DWORD* OUT PositionAfterSet);

/*A wrap if the writefile function, it gets as an input handle of a file,buffer, and the amount of bytes to write
if does not manage to write, it exits with the last error*/
int WriteFileWrap(HANDLE hFile, LPCVOID  lpBuffer, DWORD  nNumberOfBytesToWrite);


int ReadLine(HANDLE input_file, char ** OUT line);
/*return the  number of line in file  and the places in the file of each  end of line */
//int read_number_of_line_and_end_of_lines(HANDLE file, PDWORD OUT num_of_lines_out,  OUT uli** p_end_of_lines);



//--------------------------aray and and handles ------------------------


//cheack if alloction of memory succeded if not return correspnding error code 
int  CheckAlocation(void* p_arr);
//cheack if  HANDLE IS VALID IF  not return correspnding error code 
int CheakHandle(HANDLE my_handle);

/*A wrap of the closehandle , it get as an input the handle of the file and close it,
if it does not manage to close , it exits with last error*/
int CloseHandleWrap(HANDLE file);

/* input: array of allocated memories, the function runs
over the array and it dealocate it
*/
int FreeArray(void** arr, int len);

/*input: array of handles and it's length,
the function closes the handles in the array, if not it exit with LastError.
*/
int FreeHandelsArray(HANDLE* handels, int len);





//memory mengment 
//int setup_memory_menagment(memorytracker* out memtracker);
//int push_element_memory_mangment(memorytracker*  memtracker, memorytracker element);
//int freeall(memorytracker * memory_traker);







