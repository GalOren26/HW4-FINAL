


#include "Functions.h"
#include <assert.h>

#include <stdlib.h>
// ******** valiation of data and parms ************ 

void CheakArgs(int argc, int excpted_num_of_args)
{
	if (argc < excpted_num_of_args)
	{
		printf("ERROR: Not enough input arguments\n");
		exit(ERR_CODE_NOT_ENOUGH_ARGUMENTS);
	}
	if (argc > excpted_num_of_args)
	{
		printf("ERROR: Too many input arguments\n");
		exit(ERR_CODE_TOO_MANY_ARGUMENTS);
	}
}

int valid_PTR(void* ptr)
{
	if (!ptr)
	{
		printf(" invalid input pointer parm\n");
		return INVALID_INPUT_PARM_EMPTY_POINTER;
	}
	return SUCCESS;
}

int FreeHandelsArray(HANDLE* handels, int len)
{
	int ret_val = 0;
	ret_val = valid_PTR(handels);
	if (ret_val != SUCCESS)
	{
		return ret_val;
	}
	for (int i = 0; i < len; i++)
	{
		if (handels[i] != 0)
			CloseHandleWrap(handels[i]);
	}
	if (handels != 0)
		free(handels);
	return SUCCESS;
}



int  CheckAlocation(void* p_arr)
{
	if (p_arr == NULL) {
		printf_s("MEMORY_ALLOCATION_FAILURE.\n");
		return MEMORY_ALLOCATION_FAILURE;
	}
	return SUCCESS;
}
int CheakHandle(HANDLE my_handle)
{
	if (my_handle == INVALID_HANDLE_VALUE)
	{
		printf_s("INVALID_HANDLE. error code %d\n", GetLastError());
		return  GetLastError();
	}
	return SUCCESS;
}

int CheakIsAnumber(char* str)
{
	int ret_val = 0;
	ret_val = valid_PTR(str);
	if (ret_val != SUCCESS)
	{
		return ret_val;
	}
	/*This function cheak if string is a number - used for validate the value of key in this code */
	for (int i = 0; str[i] != '\0'; i++)
	{
		if (!isdigit(str[i]))
		{
			return FALSE;
		}
	}
	return SUCCESS;
}
//****************File methods**************

int ReadLine(HANDLE input_file, char* line)
{

	uli curser_index = 0;
	int ret_val = 0;
	int start_pos;
	ret_val = SetFilePointerWrap(input_file, 0, FILE_CURRENT, &start_pos);
	if (ret_val != SUCCESS)
	{
		return ret_val;
	}
	DWORD num_of_bytes_read;
	ret_val = ReadFileWrap(MAX_LEN_MESSAGE, input_file, line, &num_of_bytes_read);
	if (ret_val != SUCCESS)
		return ret_val;
	for (; line[curser_index] != 0 && line[curser_index] != '\n' && curser_index < MAX_LEN_MESSAGE; curser_index++);
	if (line[curser_index] == '\n')
	{
		ret_val = SetFilePointerWrap(input_file, start_pos + curser_index + 1, FILE_BEGIN, NULL);
		if (ret_val != SUCCESS)
		{
			return ret_val;
		}

		line[curser_index - 1] = '\0';
		memset(&line[curser_index - 1], 0, MAX_LEN_MESSAGE - (curser_index - 1));
		return SUCCESS;
	}
	return NOT_VALID_INPUT;

}

int OpenFileWrap(LPCSTR str, DWORD mode, HANDLE* OUT hFile)
{
	//CreateFileA wrap 
	int ret_val = 0;
	*hFile = CreateFileA(str, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, mode, FILE_ATTRIBUTE_NORMAL, NULL);
	return CheakHandle(*hFile);
}

int CloseHandleWrap(HANDLE file)
{
	//CloseHandle wrap 
	BOOL  file_status = CloseHandle(file);
	if (!file_status)
	{
		printf("Failed to close file.error code %d\n", GetLastError());
		return FAILAD_TO_CLOSE_FILE;
		//not exit couse try best effort to close more files. 
	}
	return SUCCESS;
}
int FreeArray(void** arr, int len)
{
	int ret_val = 0;
	ret_val = valid_PTR(arr);
	if (ret_val != SUCCESS)
	{
		return ret_val;
	}
	for (int i = 0; i < len; i++)
	{
		if (arr[i] != NULL)
			free(arr[i]);
	}
	free(arr);
	return SUCCESS;
}
int  ReadFileWrap(DWORD len, HANDLE file, char* my_file_buff, DWORD* NumberOfBytesRead)
{
	//WRAP TO ReadFile 
	if (ReadFile(file, (LPVOID)my_file_buff, len, NumberOfBytesRead, NULL) == 0)
	{
		printf("error read file . error code %d\n", GetLastError());
		return ERROR_READ_FILE;
	}
	return SUCCESS;
}

int WriteFileWrap(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite)
{
	DWORD lpNumberOfBytesRead = 0;
	if (WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, &lpNumberOfBytesRead, NULL) == 0)
	{
		printf("error write file . error code %d\n", GetLastError());
		return FAILED_WRITE_TO_FILE;
	}
	return SUCCESS;
}
//write string to file add that string a /n in the end ;
int WriteLineString(HANDLE input_file, char* line)
{
	int len = strlen(line);
	char* temp_string = NULL;
	//set eof 
	int ret_val1= SetEofAccordingToText(input_file, line);
	if (ret_val1 != SUCCESS)
	{
		return ret_val1;
	}
	//alocate array in odredr to add \n in the end 
	temp_string = (char*)calloc(len, sizeof(char)); 
	ret_val1=CheckAlocation(temp_string);
	if (ret_val1 != SUCCESS)
	{
		return ret_val1; 
	}

	sprintf_s(temp_string,sizeof(temp_string), "%s\n", line);
	ret_val1=WriteFileWrap(input_file, line, len);
	if (ret_val1 != SUCCESS)
	{
		return ret_val1;
	}
	free(temp_string);
	return SUCCESS;
	
}

int SetFilePointerWrap(HANDLE input_file, uli DistanceToMove, DWORD FromWhereToMove, DWORD* OUT PositionAfterSet)
{
	DWORD retval;
	if (PositionAfterSet != NULL)
	{
		retval = SetFilePointer(input_file, DistanceToMove, NULL, FromWhereToMove);
		*PositionAfterSet = retval;
	}
	else
		retval = SetFilePointer(input_file, DistanceToMove, NULL, FromWhereToMove);
	if (retval == INVALID_SET_FILE_POINTER)
	{
		printf("INVALID_SET_FILE_POINTER\n");
		return INVALID_SET_FILE_POINTER;
	}
	return SUCCESS;
}

int SetEofAccordingToText( HANDLE input_file, char* string )
{
	int ret_val1 = 0;
	uli current_poistion;
	//set file pointer on the current eof 
	int len = strlen(string);
	ret_val1 = SetFilePointerWrap(input_file, 0, FILE_END, &current_poistion);
	if (ret_val1 != SUCCESS)
		return ret_val1;
	//set end of file to the end of file +number of charcters that current therad need to write ,count by len

	ret_val1 = SetEndOfFile(input_file);
	if (ret_val1 == 0)
	{
		printf("error with set eof ,error code %d\n", GetLastError());
		return ret_val1;
	}
	//return cursor to the previous "end of file"
	ret_val1 = SetFilePointerWrap(input_file, current_poistion, FILE_BEGIN, NULL);
	if (ret_val1 != SUCCESS)
		return ret_val1;

	return SUCCESS;
}
