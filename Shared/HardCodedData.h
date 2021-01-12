//ERROR NUMS 
#ifndef __HARD_CODED_DATA__ 
# define __HARD_CODED_DATA__  

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCKAPI_ 


#define TIMEOUT_IN_MILLISECONDS 10000
#define BRUTAL_TERMINATION_CODE 0x55
// socket defines 
#define SERVER_ADDRESS_STR "127.0.0.1"
#define SERVER_PORT 2345
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
#define SIZE_OF_IP 3*4+4
typedef unsigned long int  uli;
 
#define FILEPATH "../GameSession.txt"
/// LENS #
#define NUM_OF_BYTES_TO_READ 100 

enum ERRORS_ENUM {
	exitUser,
	INVALID_NUMBER_OF_PARAMS,
	ERR_CODE_NOT_ENOUGH_ARGUMENTS,
	ERR_CODE_TOO_MANY_ARGUMENTS,
	INVALID_INPUT_PARM_EMPTY_POINTER,
	NOT_VALID_INPUT,
	NOT_A_NUMBER,
	FAILAD_TO_OPEN_FILE,
	FAILAD_TO_CLOSE_FILE,
	EMPTY_FILE,
	ERROR_READ_FILE,
	FAILED_WRITE_TO_FILE,
	FILE_IS_TOO_BIG,
	MEMORY_ALLOCATION_FAILURE,
	FAILED_CREATE_PROCESS,
	TIME_OUT_THREAD,
	PROBLEM_CREATE_SEMPHORE,
	PROBLEM_OPEN_SEMPHORE,
	ERROR_RELEASE_SEMPHORE,
	PROBLEM_CREATE_MUTEX,
	PROBLEM_OPEN_MUTEX,
	ERROR_RELEASE_MUTEX,
	NOT_VALID_OPREATION,
	NO_NEW_LINE,
	PROBLEM_CREATE_EVENT,
	SUCCESS
};


//typedef enum {
//	ARRAY,
//	HANDLE
//}element_type ;
//typedef struct {
//	void* array_handle;
//	int count;
//	element_type type;
//}MemoryTrackerElement;

//typedef struct {
//	MemoryTrackerElement* array_tracker;
//	int count;
//	uli max_size  ;
//}MemoryTracker;


#endif
