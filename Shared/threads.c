/*
Name:threads.c
Description:Thread and processes module implementation.
Authors:Amit Herman Raz Rajwan

*/

#include "threads.h"


HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine,
	LPDWORD p_thread_id, LPVOID lpParam)
{
	/* Should check for NULL pointers. Skipped for the sake of simplicity. */

	return CreateThread(
		NULL,            /*  default security attributes */
		0,               /*  use default stack size */
		p_start_routine, /*  thread function */
		lpParam,            /*  argument to thread function */
		0,               /*  use default creation flags */
		p_thread_id);  /*  returns the thread identifier */
}






int Init_Threads(int N, LPTHREAD_START_ROUTINE p_start_routine,
	HANDLE p_thread_handles[], DWORD p_thread_ids[]) {

	int i;

	for (i = 0; i < N; i++) {
		p_thread_handles[i] = CreateThreadSimple(p_start_routine, &(p_thread_ids[i]), NULL);

		if (p_thread_handles[i] == NULL) {
			printf("Error when creating thread: %d\n", GetLastError());
			return 0;
		}
	}

	return 1;
}
//inputs : N- number of threads, p_thread_handles - array of handles
//outputs :int indicates of the function succeeded or not
//functionality : this function closes the handles to threads
int Close_Threads(int N, HANDLE * p_thread_handles) {
	int i;

	for (i = 0; i < N; i++)
	{
		if (p_thread_handles[i]!=NULL)
			CloseHandle(p_thread_handles[i]);
	}
	return SUCCESS;
}

void KillThreads(HANDLE h_Threads[],int num_of_threads) {
	for (int i = 0; i < num_of_threads; i++)
	{
		TerminateThread(h_Threads[i], 0);
	}

}
//input arguments: handle to event
//output rguments : int indicates if the function succeeded 
//functionality: this function creates an event
int  create_event_simple(HANDLE * event )// create event
{
	HANDLE event_handle;

	static const LPSECURITY_ATTRIBUTES P_SECURITY_ATTRIBUTES = NULL;
	static const BOOL IS_MANUAL_RESET = FALSE; /* Manual-reset event */
	static const BOOL IS_INITIALLY_SET = FALSE;
	/* Get handle to event by name. If the event doesn't exist, create it */
	event_handle = CreateEvent(
		P_SECURITY_ATTRIBUTES, /* default security attributes */
		IS_MANUAL_RESET,       /* manual-reset event */
		IS_INITIALLY_SET,      /* initial state is non-signaled */
		NULL);         /* name */
	/* Check if succeeded and handle errors */

	if (event_handle == NULL)
	{
		printf("problem with create event,error code %d\n", GetLastError());
		return PROBLEM_CREATE_EVENT;
	}
	*event = event_handle;
	return SUCCESS; 

}


//input arguments: handle to wait for , time_us -miliseconds to wait
//output arguments: int indicates if the function succeeded or not
//functionality:  this function wraps the WaitForSingleObject windows API function
int WaitForSingleObjectWrap(HANDLE handle, uli time_ms)
{
	int wait_code = WaitForSingleObject(handle, time_ms);
	if (wait_code != WAIT_OBJECT_0)
	{
		printf("problem with WaitForSingleObject ,error code is %d \n\n", GetLastError());
		return TIME_OUT_THREAD;
	}	
	return SUCCESS;
}
//input arguments: handle to wait for , time_us -miliseconds to wait
//output arguments: int indicates if the function succeeded or not
//functionality:  this function wraps the WaitForMultipleObject windows API function
int WaitForMultipleObjectsWrap(uli num_of_threads, HANDLE* handle_arr, uli time_ms, BOOL bWaitAll)
{

	int wait_code = WaitForMultipleObjects(num_of_threads, handle_arr, TRUE, INFINITE);
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("problem with WaitForMultipleObject ,error code is %d \n\n", GetLastError());
		return TIME_OUT_THREAD;
	}
	return SUCCESS;
}
//ipnut arguments : max_count - maximum to signal for the semapohre object, semaphore- handle to semaphore , initialcount- start value for the semaphore count
//outpus arguments : int indicates if the function succeeded
int CreateSemphoreWrap(int max_count, HANDLE* OUT semphore, int initialcount)
{
	*semphore = CreateSemaphoreA(
		NULL,	/* Default security attributes */
		initialcount,		/* Initial Count - all slots are empty */
		max_count,		/* Maximum Count */
		NULL);

	if (*semphore == NULL)
	{
		printf("problem with create semphore,error code %d\n", GetLastError());
		return PROBLEM_CREATE_SEMPHORE;
	}
	return SUCCESS;
}

int CreateMutexWrap(BOOL bInitialOwner, HANDLE* OUT mutex)
{

	*mutex = CreateMutexA(
		NULL,	/* Default security attributes */
		bInitialOwner,		/* Set if the creator of the mutex is lock it after it create it.  */
		NULL);

	if (*mutex == NULL)
	{
		printf("problem with create mutex,error code %d\n", GetLastError());
		return PROBLEM_CREATE_MUTEX;
	}
	return SUCCESS;
}

//input arguments : semaphore- handle to semaphore, name - The name of the semaphore object
//output arguments : int that indicats if the function succeeded
//functionality: this function wraps the OpenSemaphoreA windows API function

int OpenSemphoreWrap(HANDLE* OUT semphore, const char* name)
{
	*semphore = OpenSemaphoreA(
		SYNCHRONIZE,
		FALSE,
		name);

	if (*semphore == NULL)
	{
		printf("problem with OPEN semphore,error code %d\n", GetLastError());
		return PROBLEM_OPEN_SEMPHORE;
	}
	return SUCCESS;
}
//input arguments : semaphore- handle to semaphore, Irelease-The amount by which the semaphore object's current count is to be increased
//output arguments : int that indicats if the function succeeded
//functionality: this function wraps the ReleaseSemaphore windows API function
int ReleaseSemphoreWrap(HANDLE semphore, int lReleaseCount)
{
	int wait_code = ReleaseSemaphore(semphore, lReleaseCount, NULL);
	if (wait_code == 0)
	{
		printf("problem with realease semphore ,error code%d\n", GetLastError());
		return ERROR_RELEASE_SEMPHORE;
	}
	return SUCCESS;
}
//input arguments : mutex - handle to mutex object
//output arguments : int that indicats if the function succeeded
//functionality: this function wraps the ReleaseMutex windows API function

int ReleaseMutexeWrap(HANDLE mutex)
{
	int wait_code = ReleaseMutex(mutex);
	if (wait_code == 0)
	{
		printf("problem with realease Mutex,error code  %d\n", GetLastError());
		return ERROR_RELEASE_MUTEX;
	}
	return SUCCESS;
}

//// can we delete ???
void TerminateThreadGracefully(HANDLE * my_thread )
{
	int ret_val = 0; 
	ret_val = WaitForSingleObject(*my_thread, TIME_OUT_THREADS);
	if (ret_val != WAIT_OBJECT_0)
	{
		TerminateThread(*my_thread, -1);
	}
	CloseHandle(*my_thread);
	*my_thread = NULL;
}